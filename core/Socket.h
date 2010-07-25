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
#ifndef __SOCKET_H__
#define __SOCKET_H__

#define SA          struct sockaddr
#define SALEN(sa)   (((SA *) (sa))->sa_family == AF_INET) ? sizeof(struct sockaddr_in) : \
                    ((((SA *) (sa))->sa_family == AF_INET6) ? sizeof(struct sockaddr_in6) : 0)

class StreamManager;

class Socket {
protected:
    int Sock;

    static int sock_listen(int sock);
    static int sock_accept(int sock);
    static int sock_nonblock(int sock);
    static int sock_bind(int sock, uint16_t port);
    static void sa_setport(struct sockaddr* sa, uint16_t port);

public:
    Socket(int fd);
    virtual ~Socket();
    virtual int receive_event() = 0;
};

class ListenSocket : public Socket {
    StreamManager* Manager;

public:
    ListenSocket(int fd, StreamManager *mgr) : Socket(fd) { Manager = mgr; }
    virtual int receive_event();

    int listen(int port);
};

class StreamSocket : public Socket {
public:
    StreamSocket(int fd);
};

class StreamManager {
    int PollFd;
    ListenSocket* Listen;
    std::list<StreamSocket*> Streams;

    int epoll_create();
    int epoll_add(int fd, Socket* sock);

protected:
    void register_stream_socket(int new_fd, StreamSocket* ss);

public:
    virtual ~StreamManager() {}
    virtual void accept_event(int new_fd) = 0;

    int initialize(int port);
    int wait_event(int timeout);
};

#endif
