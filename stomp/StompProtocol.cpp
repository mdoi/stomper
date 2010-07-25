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

StompProtocol *StompProtocol::create(char* cmd, StompStreamSocket *socket)
{
    if (strcmp(cmd, "CONNECT") == 0)
        return new StompConnect(socket);
    if (strcmp(cmd, "SUBSCRIBE") == 0)
        return new StompSubscribe(socket);
    if (strcmp(cmd, "SEND") == 0)
        return new StompSend(socket);

    return NULL;
}


////////////////////////////////////////////////////////////////////////////////

// virtual
void StompConnect::execute(char* msgbody, int msglen)
{
    StompClient *client = new StompClient;

    client->set_socket(Socket);
    Socket->set_client(client);

    Socket->send_line("CONNECTED");
    Socket->send_line("session:12345");
    Socket->send_line(NULL);
    Socket->send_term();
}

////////////////////////////////////////////////////////////////////////////////

// virtual
void StompSubscribe::set_header(char *key, char *value)
{
    if (strcmp(key, "destination") == 0) {
        StompClient *client = Socket->client();
        if (client != NULL)
            client->subscribe(value);
    }
}


////////////////////////////////////////////////////////////////////////////////

// virtual
void StompSend::set_header(char *key, char *value)
{
    if (strcmp(key, "destination") == 0) {
        Dest = BindingManager::search(value);
        printf("%s: destination=%s, binding=%p\n", __func__, value, Dest);
    }
}

// virtual
void StompSend::execute(char* msgbody, int msglen)
{
    if (Dest == NULL) {
        printf("%s: no destination\n", __func__);
    } else {
        char buf[256], term = 0;

        snprintf(buf, sizeof(buf), "destination:%s", Dest->name());

        Dest->send_line("MESSAGE");
        Dest->send_line(buf);
        Dest->send_line("message-id:12345");
        Dest->send_line(NULL);
        Dest->send_message(msgbody, msglen);
        Dest->send_message(&term, 1);
    }
}
