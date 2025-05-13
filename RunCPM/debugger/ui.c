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