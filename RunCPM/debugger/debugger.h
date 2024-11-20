/* debugger.h: Fuse's monitor/debugger
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

#ifndef FUSE_DEBUGGER_H
#define FUSE_DEBUGGER_H

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

#include "breakpoint.h"

/* The current state of the debugger */
enum debugger_mode_t
{
  DEBUGGER_MODE_INACTIVE,	/* No breakpoint set */
  DEBUGGER_MODE_ACTIVE,		/* Breakpoint set, but emulator running */
  DEBUGGER_MODE_HALTED,		/* Execution not happening */
};

extern enum debugger_mode_t debugger_mode;
extern char gdbserver_debugging_enabled;

/* Which base should we display things in */
extern int debugger_output_base;

void debugger_register_startup( void );

int debugger_init( void );

void debugger_end( void );

void debugger_reset( void );

int is_debugger_enabled();

int debugger_trap( void );	/* Activate the debugger */

int debugger_step( void );	/* Single step */
int debugger_next( void );	/* Go to next instruction, ignoring CALL etc */
int debugger_run( void ); /* Set debugger_mode so that emulation will occur */

/* Disassemble the instruction at 'address', returning its length in
   '*length' */
void debugger_disassemble( char *buffer, size_t buflen, size_t *length, uint16_t address );

/* Get an instruction relative to a specific address */
uint16_t debugger_search_instruction( uint16_t address,
                                              int delta );

/* Evaluate a debugger command */
void debugger_command_evaluate( const char *command );

/* Get a deparsed expression */
int debugger_expression_deparse( char *buffer, size_t length,
				 const debugger_expression *exp );

/* Register an event type with the debugger */
int debugger_event_register( const char *type, const char *detail );

/* Fire off a debugger event */
void debugger_event( int event_code );

void debugger_reset_tstates();
void debugger_track_frame( uint32_t frame_length );
uint32_t debugger_track_tstates();

/* Exit the emulator */
void debugger_exit_emulator( debugger_expression *exit_code_expression );

/* Get the exit code to be used when exiting the emulator */
int debugger_get_exit_code( void );

/* Debugger system variables */
typedef uint32_t (*debugger_get_system_variable_fn_t)( void );
typedef void (*debugger_set_system_variable_fn_t)( uint32_t value );

void z80_debugger_variables_init( void );

void debugger_system_variable_register(
  const char *type, const char *detail,
  debugger_get_system_variable_fn_t get,
  debugger_set_system_variable_fn_t set );

/* Unit tests */
int debugger_disassemble_unittest( void );

#endif				/* #ifndef FUSE_DEBUGGER_H */
