/* debugger.c: Fuse's monitor/debugger
   Copyright (c) 2002-2016 Philip Kendall

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

   Author contact information:

   E-mail: philip-fuse@shadowmagic.org.uk

   Borrowed from  https://github.com/speccytools/fuse/ by Stuart McGrigor
*/

#include "../globals.h"
#include "debugger.h"
#include "gdbserver.h"
#include "internals.h"
#include "ui.h"

/* Need to do (pretend) IN and OUT instructions */
extern void cpu_out(const uint32 Port, const uint32 Value);
extern uint32 cpu_in(const uint32 Port);

/* RunCPM Status */
extern uint32_t Status;

/* Z80 Registers */
extern int32_t PC;  /* PC register */
extern int32_t SP;  /* SP register */

/* The current activity state of the debugger */
enum debugger_mode_t debugger_mode;
char gdbserver_debugging_enabled = 0;

/* Which base should we display things in */
int debugger_output_base;

/* Fuse's exit code */
static int exit_code = 0;

int
debugger_init( void )
{
  debugger_breakpoints = NULL;
  debugger_output_base = 16;

  // debugger_event_init();
  // debugger_reset_tstates();
  debugger_system_variable_init();
  debugger_variable_init();
  debugger_reset();

  return 0;
}

void
debugger_reset( void )
{
  debugger_breakpoint_remove_all();
  debugger_mode = DEBUGGER_MODE_INACTIVE;
}

void
debugger_end( void )
{
  debugger_breakpoint_remove_all();
  debugger_variable_end();
  debugger_system_variable_end();
  // debugger_event_end();
}

/*
void
debugger_register_startup( void )
{
  startup_manager_module dependencies[] = {
    STARTUP_MANAGER_MODULE_EVENT,
    STARTUP_MANAGER_MODULE_MEMPOOL,
    STARTUP_MANAGER_MODULE_SETUID,
  };
  startup_manager_register( STARTUP_MANAGER_MODULE_DEBUGGER, dependencies,
                            ARRAY_SIZE( dependencies ), debugger_init, NULL,
                            debugger_end );
}
*/

int
is_debugger_enabled()
{
  return gdbserver_debugging_enabled;
}

/* Activate the debugger */
int
debugger_trap( void )
{
    if (gdbserver_debugging_enabled) {
        return gdbserver_activate();
    }
    return 0;
}

/* Step one instruction */
int
debugger_step( void )
{
  debugger_mode = DEBUGGER_MODE_HALTED;
  if (gdbserver_debugging_enabled) {
      return 0;
  }
  return 0;
}

/* Step to the next instruction, ignoring CALLs etc */
int
debugger_next( void )
{
  size_t length;

  /* Find out how long the current instruction is */
  debugger_disassemble( NULL, 0, &length, PC );

  /* And add a breakpoint after that */
  debugger_breakpoint_add_address(
    DEBUGGER_BREAKPOINT_TYPE_EXECUTE, 
      // memory_source_any, 0, 
      PC + length, 0,
      DEBUGGER_BREAKPOINT_LIFE_ONESHOT, NULL
  );

  debugger_run();

  return 0;
}

/* Set debugger_mode so that emulation will occur */
int
debugger_run( void )
{
  debugger_mode = debugger_breakpoints ?
                  DEBUGGER_MODE_ACTIVE :
                  DEBUGGER_MODE_INACTIVE;
  if (gdbserver_debugging_enabled)
  {
      return 0;
  }
  return 0;
}

/* Exit from the last CALL etc by setting a oneshot breakpoint at
   (SP) and then starting emulation */
int
debugger_breakpoint_exit( void )
{
  uint16_t target;

  target = _RamRead16(SP); // readbyte_internal( SP ) + 0x100 * readbyte_internal( SP+1 );

  if( debugger_breakpoint_add_address(
        DEBUGGER_BREAKPOINT_TYPE_EXECUTE, 
        // memory_source_any, 0, 
        target, 0, DEBUGGER_BREAKPOINT_LIFE_ONESHOT, NULL
      )
    )
    return 1;

  if( debugger_run() ) return 1;

  return 0;
}

/* Poke a value into RAM */
int
debugger_poke( uint16_t address, uint8_t value )
{
  _RamWrite(address, value);
  return 0;
}

/* Write a value to a port */
int
debugger_port_write( uint16_t port, uint8_t value )
{
  cpu_out(port, value);
  return 0;
}

/* Exit the emulator */
void
debugger_exit_emulator( debugger_expression *exit_code_expression )
{
  exit_code = exit_code_expression ?
    debugger_expression_evaluate( exit_code_expression ) : 0;

  /* Ensure we break out of the main Z80 loop immediately */
  Status=1;

  debugger_run();
}

int
debugger_get_exit_code( void )
{
  return exit_code;
}
