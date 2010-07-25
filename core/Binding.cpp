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

Binding::Binding(char* name)
{
    strncpy(Name, name, sizeof(Name) - 1);
    Name[sizeof(Name) - 1] = 0;
}

int Binding::bind(Receiver* r)
{
    Receivers.push_back(r);
    return 0;
}

void Binding::unbind(Receiver *r)
{
    Receivers.remove(r);
}

int Binding::send_line(char* line)
{
    if (line != NULL) {
        if (send_message(line, strlen(line)) < 0)
            return -1;
    }

    if (send_message("\n", 1) < 0)
        return -1;

    return 0;
}

int Binding::send_message(char* msg, int len)
{
    std::list<Receiver*>::iterator iter = Receivers.begin();

    for (; iter != Receivers.end(); iter++)
        (*iter)->send_message(msg, len);

    return 0;
}


////////////////////////////////////////////////////////////////////////////////

std::list<Binding*> BindingManager::Bindings;

Binding* BindingManager::create(char* name)
{
    Binding* binding = new Binding(name);
    Bindings.push_back(binding);

    printf("%s: create binding: name=%s, binding=%p\n", __func__, name, binding);

    return binding;
}

Binding* BindingManager::search(char *name)
{
    std::list<Binding*>::iterator iter = Bindings.begin();

    for (; iter != Bindings.end(); iter++) {
        if (strcmp((*iter)->name(), name) == 0) {
            printf("%s: binding found: name=%s, binding=%p\n", __func__, name, *iter);
            return *iter;
        }
    }

    return NULL;
}

void BindingManager::remove(Binding* binding)
{
    printf("%s: remove binding: name=%s, binding=%p\n", __func__, binding->name(), binding);

    Bindings.remove(binding);
    delete binding;
}
