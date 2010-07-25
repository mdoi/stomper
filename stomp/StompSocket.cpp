/*
 * Copyright (c) 2010 Satoshi Ebisawa. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. The names of its contributors may not be used to endorse or
 *    promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include "../stomper.h"

////////////////////////////////////////////////////////////////////////////////

StompStreamSocket::StompStreamSocket(int fd) : StreamSocket(fd)
{
    Client = NULL;
}

// virtual
StompStreamSocket::~StompStreamSocket()
{
    if (Client != NULL)
        delete Client;
}

// virtual 
int StompStreamSocket::receive_event()
{
    int flen;
    char buf[4096], *line, *next, *value;

    if ((flen = getframe(buf, sizeof(buf))) < 0)
        return -1;
    if (flen == 0)
        return 0;   // it's not an error

    printf("%s: frame length = %d\n", __func__, flen);

    // command
    line = getline(&next, buf);
    printf("%s: command = %s\n", __func__, line);

    StompProtocol *proto = StompProtocol::create(buf, this);
    if (proto == NULL) {
        printf("%s: protocol create failed: command='%s'\n", __func__, buf);
        return -1;
    }

    // headers
    while (next != NULL) {
        line = getline(&next, next);
        if (line[0] == 0)
            break;
            
        if ((value = strchr(line, ':')) != NULL) {
            *(value++) = 0;
            while (isspace(*value))
                value++;

            proto->set_header(line, value);
        }
    }

    // message body
    int mlen = flen - (next - buf) - 1;
    if (next == NULL || mlen == 0) {
        printf("%s: no message body\n", __func__);
        proto->execute(NULL, 0);
    } else {
        printf("%s: message='%s', len=%d\n", __func__, next, mlen);
        proto->execute(next, mlen);
    }

    delete proto;

    return 0;
}

void StompStreamSocket::send_term()
{
    char c = STOMP_FRAME_TERMINATOR;
    send_buf(&c, 1);
}

void StompStreamSocket::send_line(char* line)
{
    if (line != NULL) {
        printf("<-- SEND: %s\n", line);
        send_buf(line, strlen(line));
    }

    send_buf("\n", 1);
}

void StompStreamSocket::send_buf(char* buf, int len)
{
    // XXX handle send error
    if (len > 0)
        send(Sock, buf, len, 0);
}

// private
char *StompStreamSocket::getline(char **next, char *buf)
{
    char *p;

    if ((p = strchr(buf, '\n')) == NULL)
        *next = NULL;
    else {
        *p = 0;
        *next = p + 1;
    }

    if ((p = strchr(buf, '\r')) != NULL)
        *p = 0;

    return buf;
}

// private
int StompStreamSocket::getframe(char* buf, int bufmax)
{
    int len, flen;

    if ((len = recv(Sock, buf, bufmax, MSG_PEEK)) < 0)
        return -1;

    if (len == 0) 
        return -1;
    if ((flen = frame_length(buf, len)) < 0)
        return 0;   // it's not an error. drop it silently.

    if (recv(Sock, buf, flen, 0) != flen)
        return -1;

    return flen;
}

// private
int StompStreamSocket::frame_length(char *buf, int len)
{
    int clen;

    for (clen = 0; clen < len; clen++) {
        if (buf[clen] == STOMP_FRAME_TERMINATOR)
            return clen + 1;
    }

    return -1;
}
