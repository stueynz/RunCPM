/* command.c: Parse a debugger command
   Copyright (c) 2002-2015 Philip Kendall

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

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "../globals.h"   // RunCPM global definitions...

#include "debugger.h"
#include "internals.h"
#include "ui.h"

/* The last debugger command we were given to execute */
static char *command_buffer = NULL;

/* And a pointer as to how much we've parsed */
static char *command_ptr;

int yyparse( void );
int yywrap( void );

/* Evaluate the debugger command given in 'command' */
void
debugger_command_evaluate( const char *command )
{
  if( !command ) return;

  ui_error(UI_ERROR_INFO, "dbg-evaluate:\r\n");
  ui_error(UI_ERROR_INFO, "%s\r\n", command);

  if( command_buffer ) free( command_buffer );

  command_buffer = strdup( command );

  /* Start parsing at the start of the given command */
  command_ptr = command_buffer;
    
  /* Parse (& evaluate) the command */
  yyparse();
}

/* Utility functions called from the flex scanner */

int
yywrap( void )
{
  return 1;
}

/* Called to get up to 'max_size' bytes of the command to be parsed */
int
debugger_command_input( char *buf, int *result, int max_size )
{
  size_t length = strlen( command_ptr );

  if( !length ) {
    return 0;
  } else if( length < (size_t)max_size ) {
    memcpy( buf, command_ptr, length );
    *result = length; command_ptr += length;
    return 1;
  } else {
    memcpy( buf, command_ptr, max_size );
    *result = max_size; command_ptr += max_size;
    return 1;
  }
}


/* Utility functions called by the bison parser */

extern char* dbgInput;   // current dbg input string

/* The error callback if yyparse finds an error */
void
yyerror( const char *s )
{
  ui_error( UI_ERROR_ERROR, "Invalid debugger command: %s\ndiagnostic:%s", dbgInput, s);
}
