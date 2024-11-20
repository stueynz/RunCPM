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

#ifndef GDBARCH_H
#define GDBARCH_H

#include <stdint.h>

#define SZ 2
#define FEATURE_STR "l<target version=\"1.0\">"\
    "<feature name=\"org.gnu.gdb.z80.cpu\">"\
    "<reg name=\"af\" bitsize=\"16\" type=\"int\"/>"\
    "<reg name=\"bc\" bitsize=\"16\" type=\"int\"/>"\
    "<reg name=\"de\" bitsize=\"16\" type=\"int\"/>"\
    "<reg name=\"hl\" bitsize=\"16\" type=\"int\"/>"\
    "<reg name=\"sp\" bitsize=\"16\" type=\"data_ptr\"/>"\
    "<reg name=\"pc\" bitsize=\"16\" type=\"code_ptr\"/>"\
    "<reg name=\"ix\" bitsize=\"16\" type=\"int\"/>"\
    "<reg name=\"iy\" bitsize=\"16\" type=\"int\"/>"\
    "<reg name=\"af'\" bitsize=\"16\" type=\"int\"/>"\
    "<reg name=\"bc'\" bitsize=\"16\" type=\"int\"/>"\
    "<reg name=\"de'\" bitsize=\"16\" type=\"int\"/>"\
    "<reg name=\"hl'\" bitsize=\"16\" type=\"int\"/>"\
    "<reg name=\"clockl\" bitsize=\"16\" type=\"int\"/>"\
    "<reg name=\"clockh\" bitsize=\"16\" type=\"int\"/>"\
    "</feature>"\
    "<architecture>z80</architecture>"\
    "</target>"

#define EXTRA_NUM 25
#define EXTRA_REG 16
#define EXTRA_SIZE 4

#endif /* GDBARCH_H */
