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

Socket::Socket(int fd)
{
    Sock = fd;
    printf("%s: new socket %d\n", __func__, Sock);
}

Socket::~Socket()
{
    if (Sock != -1) {
        printf("%s: close socket %d\n", __func__, Sock);
        ::close(Sock);
    }
}

// protected, static
int Socket::sock_listen(int sock)
{
    return ::listen(sock, 8);
}

// protected, static
int Socket::sock_accept(int sock)
{
    return ::accept(sock, NULL, NULL);
}

// protected, static
int Socket::sock_nonblock(int sock)
{
    int flags;

    if ((flags = fcntl(sock, F_GETFL)) < 0)
        return -1;
    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0)
        return -1;

    return 0;
}

// protected, static
int Socket::sock_bind(int sock, uint16_t port)
{
    struct sockaddr_in sin;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sa_setport((SA *) &sin, port);

    if (bind(sock, (SA *) &sin, sizeof(sin)) < 0)
        return -1;

    return 0;
}

// protected, static
void Socket::sa_setport(struct sockaddr *sa, uint16_t port)
{
    struct sockaddr_in *sin;
    struct sockaddr_in6 *sin6;

    switch (sa->sa_family) {
    case AF_INET:
        sin = (struct sockaddr_in *) sa;
        sin->sin_port = htons(port);
        break;

    case AF_INET6:
        sin6 = (struct sockaddr_in6 *) sa;
        sin6->sin6_port = htons(port);
        break;
    }
}


////////////////////////////////////////////////////////////////////////////////

int ListenSocket::listen(int port)
{
    int on = 1;

    if (setsockopt(Sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
        return -1;
    if (sock_bind(Sock, port) < 0)
        return -1;
    if (sock_listen(Sock) < 0)
        return -1;
    if (sock_nonblock(Sock) < 0)
        return -1;

    return 0;
}

// virtual
int ListenSocket::receive_event()
{
    int s;

    if ((s = sock_accept(Sock)) < 0) {
        printf("sock_accept() failed\n");
        return 0;
    }

    Manager->accept_event(s);

    return 0;
}


////////////////////////////////////////////////////////////////////////////////

StreamSocket::StreamSocket(int fd) : Socket(fd)
{
    sock_nonblock(fd);
}


////////////////////////////////////////////////////////////////////////////////

int StreamManager::initialize(int port)
{
    int sock;

    if (epoll_create() < 0)
        return -1;
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        goto error;

    Listen = new ListenSocket(sock, this);
    if (Listen->listen(port) < 0) {
        delete Listen;
        goto error;
    }

    if (epoll_add(sock, Listen) < 0) {
        delete Listen;
        goto error;
    }        

    return 0;

error:
    close(PollFd);
    return -1;
}

int StreamManager::wait_event(int timeout)
{
    int i, count;
    struct epoll_event events[8];

    if ((count = epoll_wait(PollFd, events, NELEMS(events), timeout)) < 0) {
        printf("epoll_wait() failed\n");
        return -1;
    }

    for (i = 0; i < count; i++) {
        Socket *socket = (Socket *) events[i].data.ptr;
        if (socket != NULL) {
            if (socket->receive_event() < 0)
                delete socket;
        }
    }

    return 0;
}

// protected
void StreamManager::register_stream_socket(int new_fd, StreamSocket* ss)
{
    Streams.push_back(ss);
    epoll_add(new_fd, ss);
}

// private
int StreamManager::epoll_create()
{
    if ((PollFd = ::epoll_create(8)) < 0)
        return -1;

    return 0;
}

// private
int StreamManager::epoll_add(int fd, Socket* sock)
{
    struct epoll_event event;

    memset(&event, 0, sizeof(event));
    event.events  = EPOLLIN | EPOLLET;
    event.data.ptr = sock;

    if (::epoll_ctl(PollFd, EPOLL_CTL_ADD, fd, &event) < 0)
        return -1;

    return 0;
}
