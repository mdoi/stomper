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

StompClient::StompClient()
{
    printf("%s: new client %p\n", __func__, this);

    Socket = NULL;
}

StompClient::~StompClient()
{
    std::list<Binding*>::iterator iter = Bindings.begin();

    printf("%s: delete client %p\n", __func__, this);

    for (; iter != Bindings.end(); iter++) {
        (*iter)->unbind(this);
        if ((*iter)->receivers() == 0)
            BindingManager::remove(*iter);
    }
}

// virtual
int StompClient::send_message(char* msg, int len)
{
    if (Socket == NULL)
        return -1;

    Socket->send_buf(msg, len);

    return 0;
}

int StompClient::subscribe(char *name)
{
    Binding *binding;

    if ((binding = BindingManager::search(name)) == NULL)
        binding = BindingManager::create(name);

    binding->bind(this);
    Bindings.push_back(binding);

    return 0;
}
