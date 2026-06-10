/* ui.c: User interface routines, but those which are independent of any UI
   Copyright (c) 2002-2015 Philip Kendall
   Copyright (c) 2016 Sergio Baldoví

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

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "debugger.h"
#include "internals.h"
#include "ui.h"


extern int32 PCX; /* external view of PC                          */
extern int32 AF;  /* AF register                                  */
extern int32 BC;  /* BC register                                  */
extern int32 DE;  /* DE register                                  */
extern int32 HL;  /* HL register                                  */
extern int32 IX;  /* IX register                                  */
extern int32 IY;  /* IY register                                  */
extern int32 PC;  /* program counter                              */
extern int32 SP;  /* SP register                                  */
extern int32 AF1; /* alternate AF register                        */
extern int32 BC1; /* alternate BC register                        */
extern int32 DE1; /* alternate DE register                        */
extern int32 HL1; /* alternate HL register                        */
extern int32 IFF; /* Interrupt Flip Flop                          */
extern int32 IR;  /* Interrupt (upper) / Refresh (lower) register */

#define MESSAGE_MAX_LENGTH 256

static char last_message[ MESSAGE_MAX_LENGTH ] = "";
static size_t frames_since_last_message = 0;

static int
print_error_to_stderr( ui_error_level severity, const char *message );

int
ui_error( ui_error_level severity, const char *format, ... )
{
  int error;
  va_list ap;

  va_start( ap, format );
  error = ui_verror( severity, format, ap );
  va_end( ap );

  return error;
}

int
ui_verror( ui_error_level severity, const char *format, va_list ap )
{
  char message[ MESSAGE_MAX_LENGTH ];

  vsnprintf( message, MESSAGE_MAX_LENGTH, format, ap );

  /* Skip the message if the same message was displayed recently */
  if( frames_since_last_message < 50 && !strcmp( message, last_message ) ) {
    frames_since_last_message = 0;
    return 0;
  }

  /* And store the 'last message' */
  strncpy( last_message, message, MESSAGE_MAX_LENGTH );
  last_message[ MESSAGE_MAX_LENGTH - 1 ] = '\0';

  print_error_to_stderr( severity, message );

  return 0;
}

static int
print_error_to_stderr( ui_error_level severity, const char *message )
{
  /* Print the error to stderr if it's more significant than just
     informational */
  if( severity >= UI_ERROR_INFO ) {

    switch( severity ) {

    case UI_ERROR_INFO:    fprintf( stderr, "runCPM info: " ); break;
    case UI_ERROR_WARNING: fprintf( stderr, "runCPM warn: " ); break;
    case UI_ERROR_ERROR:   fprintf( stderr, "runCPM  err: " ); break;
    }

    fprintf( stderr, "%s\n", message );
  }

  return 0;
}

/* Which base should we display things in */
extern int debugger_output_base;

void ui_breakpoint(gpointer entry, gpointer udata)
{
  char buffer[80];
  debugger_breakpoint *bp = (debugger_breakpoint *) entry;
  fprintf(stderr, "   0x%04ld: %8s 0x%04x  %9s  ignore:%ld\n", 
            bp->id, 
            debugger_breakpoint_type_text[bp->type], 
            bp->value.address.offset,
            debugger_breakpoint_life_text[bp->life],
            bp->ignore);

  if(bp->condition) {
    debugger_expression_deparse( buffer, 80, bp->condition);
    fprintf(stderr, "      if:%s\n", buffer);
  }
  
  if(bp->commands)
    fprintf(stderr, "      cmds:%s\n", bp->commands);
}


static void
ui_variable(gpointer key, gpointer value, gpointer udata)
{
  fprintf(stderr, "      %s: %04x\n", (char *) key,  GPOINTER_TO_INT(value));
}

void
ui_debugger_status()
{
  fprintf(stderr, "dbg Status:\n   outputBase:%d\n   breakpoints:%d\n", 
                  debugger_output_base, g_slist_length(debugger_breakpoints));        
  g_slist_foreach(debugger_breakpoints, ui_breakpoint, NULL);

  fprintf(stderr, "   variables:%d\n", g_hash_table_size(debugger_variables));
  g_hash_table_foreach(debugger_variables, ui_variable, NULL);

  fprintf(stderr, "   registers: BC :%04x  DE :%04x   HL :%04x   AF : %04x\n", WORD16(BC), WORD16(DE), WORD16(HL), WORD16(AF));
  fprintf(stderr, "              BC':%04x  DE':%04x   HL':%04x   AF': %04x\n", WORD16(BC1), WORD16(DE1), WORD16(HL1), WORD16(AF1));
  fprintf(stderr, "              IX :%04x  IY :%04x   SP :%04x   PC : %04x\n", WORD16(IX), WORD16(IY), WORD16(SP), WORD16(PC));

  fprintf(stderr, "\ndebugging_mode:%d\n", debugger_mode);
}

void
ui_debugger_list_breakpoints()
{
  fprintf(stderr, "\n\nbreakpoints:%d\n", g_slist_length(debugger_breakpoints));
  g_slist_foreach(debugger_breakpoints, ui_breakpoint, NULL);
}

extern uint8 bDisasm(uint16 pos, char *bptr);
extern char *bDisHex(uint16 pos, char *bptr);

extern char *_bputhex8(uint8 c, char *bptr);
extern char * _bputhex16(uint16 w, char *bptr);


/**
 *  Disassemble 16 instructions from given address onwards .. to stderr
 */
void
ui_debugger_disassemble(uint16_t address)
{
  char buf[32];

  uint16_t l = address;
  for (int i=0; i<=16; i++)
  {
    char *bptr = &buf[0];

    fprintf(stderr, "%04x : ", l);
    bptr = bDisHex(l, bptr);
    l += bDisasm(l, bptr);
    fprintf(stderr, "%s\r\n", buf);
  }
  fputs("\r\n", stderr);
}

/**
 *  HEX Dump 256 bytes from given address to stderr
 */
void
ui_debugger_dump(uint16_t address)
{
  char buf[2048];
  char *bptr;

  uint16 h = address;
	uint16 c = address;
	uint8 l, i;
	uint8 ch = address & 0xff;

  fprintf(stderr, "       00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\r\n       -----------------------------------------------\r\n");
  bptr = &buf[0];
	for (l = 0; l < 16; ++l) {
		bptr = _bputhex16(h, bptr);
    *bptr++= ' ';
    *bptr++= ':';
    *bptr++= ' ';
    
		for (i = 0; i < 16; ++i) {
			bptr = _bputhex8(_RamRead(h++), bptr);
      *bptr++ = ' ';
		}
		for (i = 0; i < 16; ++i) {
			ch = _RamRead(c++);
			*bptr++ = (ch > 31 && ch < 127 ? ch : '.');
		}
    *bptr++ = '\r';
    *bptr++ = '\n';
	}
  *bptr++ = '\0';
  fprintf(stderr, "%s\r\n", buf);
}

const char* usage = 
"DEBUGGER/MONITOR\n\n"
   
   "Debugger commands are similar in nature (but definitely not identical to or as powerful as) to those in gdb(1).\n"
   "In general, the debugger is case-insensitive, and numbers will be interpreted as decimal, unless prefixed by\n"
   "either `0x' or `$' when they will be interpreted as hex.\n\n"

   "   Addresses can be specified as an absolute addresses, specified by an integer in the range 0x0000 to 0xFFFF\n"
   "   Anywhere the debugger is expecting a numeric value, except where it expects a breakpoint id,\n"
   "   you can instead use a numeric expression, which uses a restricted version of C's syntax;\n"
   "   exactly the same syntax is used for conditional  breakpoints,  with `0' being false and any other value being true.\n\n"

   "   In numeric expressions, you can use:\n" 
   "     - integer constants (all calculations are done in integers)\n"
   "     - z80:registers - A B C D E F H L AF BC DE HL IX IY AF' BC' DE' HL' SP PC IX IY I R IFF\n"
   "     - debugger variables\n"
   "     - parentheses\n"
   "     - the standard four numeric operations (`+', `-', `*' and `/')\n"
   "     - the (non-)equality operators `==' and `!='\n"
   "     - the comparison operators `>', `<', `>=' and `<='\n"
   "     - bitwise and (`&'), or (`|') and  exclusive  or  (`^')\n"
   "     - logical  and (`&&') and or (`||')\n"
   "     - Square brackets (`[' and `]') can be used to dereference a memory location;\n"
   "        ie: `[0x4000]' will give the value at address 0x4000\n\n"

   "Available commands are:\n"
   "  BASE PRINT SET BREAK TBREAK CLEAR IGNORE CONTINUE NEXT STEP FINISH GO DUMP DISASSEMBLE\n\n";

const char* usageGeneral = 

   "DEBUGGER/MONITOR - Output Commands\n\n"

   "   ba{se} number\n"
   "          Change the debugger window to displaying output in base number.  Available values are 10 (decimal) or 16 (hex).\n\n"

   "   o{ut} port value\n"
   "          Write value to IO port port.\n\n"

   "   pr{int} expression\n"
   "          Print the value of expression to standard output.\n\n"

   "   sta{tus}\n"
   "          Display the current status of the emulator including:\n"
   "            - list variables, breakpoints & registers\n"
   "            - disassemble from PC location\n\n";

const char* usageBreak =
   "DEBUGGER/MONITOR - Breakpoint Management\n\n"

   "   br{eakpoint} [address] [if condition]\n"
   "          Set a breakpoint to stop emulation and return to the debugger whenever an opcode is executed at address and condition\n"
   "          evaluates true. If address is omitted, it defaults to the current value of PC.\n\n"

   "   br{eakpoint} p{ort} (re{ad}|w{rite}) port [if condition]\n"
   "          Set a breakpoint to trigger whenever IO port port is read from or written to and condition evaluates true.\n\n"

   "   br{eakpoint} (re{ad}|w{rite}) [address] [if condition]\n"
   "          Set a breakpoint to trigger whenever memory location address is read from (other than via an opcode fetch) or written to\n"
   "          and condition evaluates true.  Address again defaults to the current value of PC if omitted.\n\n"

   "   t{breakpoint} [options]\n"
   "          This is the same as the `breakpoint' command in its various forms, except that the breakpoint is temporary;\n"
   "          it will trigger once and once only, and then be removed.\n\n"

   "   cl{ear} [address]\n"
   "          Remove all breakpoints at address or the current value of PC if address is omitted. Port read/write breakpoints are unaffected."

   "   cond{ition} id [condition]\n"
   "          Set breakpoint id to trigger only when condition is true, or unconditionally if condition is omitted.\n\n"

   "   del{ete} [id]\n"
   "          Remove breakpoint id, or all breakpoints if id is omitted.\n\n"

   "   i{gnore} id count\n"
   "          Do not trigger the next count times that breakpoint id would have triggered.\n\n";

   /*
       com{mmands} id <newline>
       <debugger command> <newline>
       <debugger command> <newline>
       ...
       end
              Set things such that the specified debugger commands will be automatically executed when breakpoint id is triggered. There is currently no user interface for entering multi-line debugger commands, so the only  way  to
              specify this command is on the command-line via the --debugger-command option.
  */

  const char* usageMemory = 

  "DEBUGGER/MONITOR - Accessing Memory\n\n"

  "   du{mp} address\n"
  "           Dump 0x100 bytes of memory at address in Intel HEX format\n\n"

  "   di{sassemble} address\n"
  "           Set the centre panel disassembly to begin at address.\n\n"

  "   se{t} address value\n"
  "           Poke value into memory at address.\n\n"

  "   se{t} $variable value\n"
  "          Set the value of the debugger variable variable to value.\n\n"

  "   se{t} z80:register value\n"
  "          Set the value of the z80 register to value\n\n";




  const char* usageEmulation = 

  "DEBUGGER/MONITOR - Controlling Emulator\n\n"

  "   co{ntinue}\n"
  "          Drop out of the monitor and continue CPU emulation from the current PC address.\n\n"

  "   s{tep}\n"
  "         Drop out of the monitor and execute CPU emulation for a single instruction.\n\n"

  "   n{ext}\n"
  "         Drop out of the monitor and execute CPU emulation until the opcode following the current one;\n"
  "         this will jump over any `call` instruction. As with the `finish' command, this works by setting\n"
  "         a temporary breakpoint at the next opcode, so is not infallible.\n\n"

  "   go address\n"
  "         Set PC register to given address, and drop out of the monitor and commence CPU emulation from that address.\n\n"

  "   ex{it} [expression]\n"
  "          Exit the emulator immediately, using the exit code resulting from the evaluation of expression\n"
  "          or 0 if expression is omitted.\n\n"

  "   fi{nish}\n"
  "          Exit  from  the  current  CALL  or equivalent. This isn't infallible: it works by setting a temporary breakpoint\n"
  "          at the current contents of the stack pointer, so will not function correctly if the code returns to some other point\n"
  "          or plays with its stack in other ways. Also, setting this breakpoint doesn't disable other breakpoints, which may\n"
  "          trigger before this one. In that case, the temporary breakpoint remains, and the `continue' command can be used to return to it.\n\n";


void
ui_usage(char *area) {

  
  if(area == NULL) {
    fputs(usage, stderr);
  }
  else if(! strcmp("breakpoint",area))
    fputs(usageBreak, stderr);

  else if(! strcmp("emulation", area))
    fputs(usageEmulation, stderr);

  else if(! strcmp("memory", area))
    fputs(usageMemory, stderr);

  else if(! strcmp("general", area))
    fputs(usageGeneral, stderr);

  else {
    fprintf(stderr, "Help area: %s\n", area);
  }
}
