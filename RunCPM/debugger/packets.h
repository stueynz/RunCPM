/* debugger_internals.h: The internals of Fuse's monitor/debugger
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
#ifndef PACKETS_H
#define PACKETS_H

#include <stdint.h>

#define PACKET_BUF_SIZE 0x4000

static const char INTERRUPT_CHAR = '\x03';

uint8_t *inbuf_get();
int inbuf_end();
void inbuf_reset();
void inbuf_erase_head(ssize_t end);
void write_flush(int sockfd);
void write_packet(const char *data);
void write_binary_packet(const char *pfx, const uint8_t *data, ssize_t num_bytes);
int read_packet(int sockfd);
void acknowledge_packet(int sockfd);

#endif /* PACKETS_H */
