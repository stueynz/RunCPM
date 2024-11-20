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

static const char*
format_16_bit( void )
{
  return debugger_output_base == 10 ? "%5d" : "0x%04X";
}

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

  fprintf(stderr, "   registers: BC :%04x  DE :%04x   HL :%04x   AF : %04x\n", BC, DE, HL, AF);
  fprintf(stderr, "              BC':%04x  DE':%04x   HL':%04x   AF': %04x\n", BC1, DE1, HL1, AF1);
  fprintf(stderr, "              IX :%04x  IY :%04x   SP :%04x   PC : %04x\n", IX, IY, SP, PC);
}

void
ui_debugger_disassemble(uint16_t address)
{
  size_t i;

  for( i = 0; i < 20; i++ ) {
    size_t l, length;
    char buffer1[40], buffer2[40];

    snprintf( buffer1, sizeof( buffer1 ), format_16_bit(), address );
    debugger_disassemble( buffer2, sizeof( buffer2 ), &length, address );

    /* pad to 16 characters (long instruction) to avoid varying width */
    /*
    l = strlen( buffer2 );
    while( l < 16 ) buffer2[l++] = ' ';
    buffer2[l] = 0;
    */

    address += length;

    // output the two buffers to stderr
    fprintf( stderr, "%s   %s", buffer1, buffer2);
  }
}
