/* expression.c: A numeric expression
   Copyright (c) 2003-2016 Philip Kendall

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
#ifndef GDBSERVER_UTILS_H
#define GDBSERVER_UTILS_H

#include <stdint.h>

static const char hexchars[] = "0123456789abcdef";

int hex(char ch);
char *mem2hex(const uint8_t *mem, char *buf, int count);
uint8_t *hex2mem(const char *buf, uint8_t *mem, int count);
int unescape(char *msg, int len);

#endif /* GDBSERVER_UTILS_H */
