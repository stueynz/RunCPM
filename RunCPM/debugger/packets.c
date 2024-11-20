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

// Many codes in this file was borrowed from GdbConnection.cc in rr
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <unistd.h>
#include <stdbool.h>
#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/poll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif
#include "packets.h"

struct packet_buf
{
    uint8_t buf[PACKET_BUF_SIZE];
    int end;
} in, out;

uint8_t *inbuf_get()
{
    return in.buf;
}

int inbuf_end()
{
    return in.end;
}

static void pktbuf_clear(struct packet_buf *pkt)
{
    pkt->end = 0;
    pkt->buf[pkt->end] = 0;
}

void inbuf_reset()
{
    pktbuf_clear(&in);
}

void pktbuf_insert(struct packet_buf *pkt, const uint8_t *buf, ssize_t len)
{
    if (pkt->end + len >= sizeof(pkt->buf))
    {
        puts("Packet buffer overflow");
        exit(-2);
    }
    memcpy(pkt->buf + pkt->end, buf, len);
    pkt->end += len;
    pkt->buf[pkt->end] = 0;
}

void pktbuf_erase_head(struct packet_buf *pkt, ssize_t end)
{
    memmove(pkt->buf, pkt->buf + end, pkt->end - end);
    pkt->end -= end;
    pkt->buf[pkt->end] = 0;
}

void inbuf_erase_head(ssize_t end)
{
    pktbuf_erase_head(&in, end);
}

int read_data_once(int sockfd)
{
    ssize_t nread;
    uint8_t buf[4096];

    nread = recv(sockfd, (void*)buf, sizeof(buf), 0);
    if (nread <= 0)
    {
        return -1;
    }
    pktbuf_insert(&in, buf, nread);
    return 0;
}

void write_flush(int sockfd)
{
    size_t write_index = 0;
    while (write_index < out.end)
    {
        ssize_t nwritten = send(sockfd, (const void *)(out.buf + write_index), out.end - write_index, 0);
        if (nwritten < 0)
        {
            printf("Write error\n");
        }
        write_index += nwritten;
    }
    pktbuf_clear(&out);
}

void write_data_raw(const uint8_t *data, ssize_t len)
{
    pktbuf_insert(&out, data, len);
}

void write_hex(unsigned long hex)
{
    char buf[32];
    size_t len;

    len = snprintf(buf, sizeof(buf) - 1, "%02lx", hex);
    write_data_raw((uint8_t *)buf, len);
}

void write_packet_bytes(const uint8_t *data, size_t num_bytes)
{
    uint8_t checksum;
    size_t i;

    write_data_raw((uint8_t *)"$", 1);
    for (i = 0, checksum = 0; i < num_bytes; ++i)
        checksum += data[i];
    write_data_raw((uint8_t *)data, num_bytes);
    write_data_raw((uint8_t *)"#", 1);
    write_hex(checksum);
}

void write_packet(const char *data)
{
    write_packet_bytes((const uint8_t *)data, strlen(data));
    printf("w: %s\n", data);
}

void write_binary_packet(const char *pfx, const uint8_t *data, ssize_t num_bytes)
{
    uint8_t *buf;
    ssize_t pfx_num_chars = strlen(pfx);
    ssize_t buf_num_bytes = 0;
    int i;

    buf = malloc(2 * num_bytes + pfx_num_chars);
    memcpy(buf, pfx, pfx_num_chars);
    buf_num_bytes += pfx_num_chars;

    for (i = 0; i < num_bytes; ++i)
    {
        uint8_t b = data[i];
        switch (b)
        {
        case '#':
        case '$':
        case '}':
        case '*':
            buf[buf_num_bytes++] = '}';
            buf[buf_num_bytes++] = b ^ 0x20;
            break;
        default:
            buf[buf_num_bytes++] = b;
            break;
        }
    }
    write_packet_bytes(buf, buf_num_bytes);
    free(buf);
}

bool skip_to_packet_start()
{
    ssize_t end = -1;
    for (size_t i = 0; i < in.end; ++i)
        if (in.buf[i] == '$' || in.buf[i] == INTERRUPT_CHAR)
        {
            end = i;
            break;
        }

    if (end < 0)
    {
        pktbuf_clear(&in);
        return false;
    }

    pktbuf_erase_head(&in, end);
    assert(1 <= in.end);
    assert('$' == in.buf[0] || INTERRUPT_CHAR == in.buf[0]);
    return true;
}

int read_packet(int sockfd)
{
    while (!skip_to_packet_start())
    {
        int ret = read_data_once(sockfd);
        if (ret)
        {
            return ret;
        }
    }
    return 0;
}

void acknowledge_packet(int sockfd)
{
    write_data_raw((uint8_t *)"+", 1);
    write_flush(sockfd);
}
