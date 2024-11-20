/* ui.h: General UI event handling routines
   Copyright (c) 2000-2015 Philip Kendall
   Copyright (c) 2016 BogDan Vatra

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

#ifndef FUSE_UI_H
#define FUSE_UI_H

/* Remove the gcc-specific incantations if we're not using gcc */
#ifdef __GNUC__
  #define GCC_UNUSED __attribute__ ((unused))
  #define GCC_PRINTF( fmtstring, args ) __attribute__ ((format( printf, fmtstring, args )))
  #define GCC_NORETURN __attribute__ ((noreturn))
#else				/* #ifdef __GNUC__ */
  #define GCC_UNUSED
  #define GCC_PRINTF( fmtstring, args )
  #define GCC_NORETURN
#endif				/* #ifdef __GNUC__ */


/* The various severities of error level, increasing downwards */
typedef enum ui_error_level {

  UI_ERROR_INFO,	  	/* Informational message */
  UI_ERROR_WARNING,		/* Something is wrong, but it's not that important */
  UI_ERROR_ERROR,		  /* An actual error */

} ui_error_level;

int ui_init(int *argc, char ***argv);
int ui_event(void);
int ui_end(void);

/* Error handling routines */
int ui_error( ui_error_level severity, const char *format, ... )
     GCC_PRINTF( 2, 3 );
int ui_verror( ui_error_level severity, const char *format, va_list ap )
     GCC_PRINTF( 2, 0 );
     
void ui_debugger_disassemble(uint16_t address);
void ui_debugger_status();

#endif			/* #ifndef FUSE_UI_H */
