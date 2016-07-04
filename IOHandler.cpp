/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   IOThread.cpp
 * Author: ductn
 * 
 * Created on June 29, 2016, 11:08 AM
 */

#include "NonblockingServer.h"

IOHandler::IOHandler(Server* server, int serverSocket) {
    server_ = server;
    listenSocket_ = serverSocket;
    eventBase_ = NULL;
    notificationEvent_ = NULL;
}

void IOHandler::registerEvents() {
    // Initialize event base
    eventBase_ = event_base_new();

    if (listenSocket_ >= 0) {
        listenEvent_ = event_new(eventBase_, listenSocket_, EV_READ | EV_PERSIST,
                IOHandler::listenCallback, server_);
        event_add(listenEvent_, 0);
    } else {
        std::cout << "registerEvent error\n";
    }

    createNotificationPipe();


    notificationEvent_ = event_new(eventBase_, getNotificationRecvFD(),
            EV_READ | EV_PERSIST, IOHandler::notificationHandler, this);
    if (event_add(notificationEvent_, 0) == -1) {
        std::cout << "registerEvent error\n";
    }
}

void IOHandler::listenCallback(evutil_socket_t fd, short what, void *arg) {
    ((Server*) arg)->listenHandler(fd, what);
}

void IOHandler::run() {
    if (eventBase_ == NULL) {
        registerEvents();
    }
    std::cout << "IOHandler entering loop...\n";
    event_base_loop(eventBase_, 0);
}

event_base* IOHandler::getEventBase() {
    return eventBase_;
}

void IOHandler::createNotificationPipe() {
    if (evutil_socketpair(AF_LOCAL, SOCK_STREAM, 0, notificationPipeFDs_) == -1) {
        std::cout << "createNotificationBase()\n";
    }
    if (evutil_make_socket_nonblocking(notificationPipeFDs_[0]) < 0
            || evutil_make_socket_nonblocking(notificationPipeFDs_[1]) < 0) {
        close(notificationPipeFDs_[0]);
        close(notificationPipeFDs_[1]);
        std::cout << "createNotificationBase()\n";
    }
    for (int i = 0; i < 2; ++i) {

        if (evutil_make_socket_closeonexec(notificationPipeFDs_[i]) < 0) {
            close(notificationPipeFDs_[0]);
            close(notificationPipeFDs_[1]);
            std::cout << "createNotificationBase()\n";
        }
    }
}

bool IOHandler::notify(Connection* conn) {
    int fd = getNotificationSendFD();
    if (fd < 0) {
        return false;
    }

    fd_set wfds, efds;
    int ret = -1;
    int kSize = sizeof (conn);
    const char* pos = (const char*) reinterpret_cast<void*> (&conn);

    while (kSize > 0) {
        FD_ZERO(&wfds);
        FD_ZERO(&efds);
        FD_SET(fd, &wfds);
        FD_SET(fd, &efds);
        ret = select(fd + 1, NULL, &wfds, &efds, NULL);
        if (ret < 0) {
            return false;
        } else if (ret == 0) {
            continue;
        }

        if (FD_ISSET(fd, &efds)) {
            close(fd);
            return false;
        }

        if (FD_ISSET(fd, &wfds)) {
            ret = send(fd, pos, kSize, 0);
            if (ret < 0) {
                if (errno == EAGAIN) {
                    continue;
                }

                close(fd);
                return false;
            }

            kSize -= ret;
            pos += ret;
        }
    }

    return true;
}

int IOHandler::getNotificationRecvFD() {
    return notificationPipeFDs_[0];
}

int IOHandler::getNotificationSendFD() {
    return notificationPipeFDs_[1];
}

void IOHandler::notificationHandler(int fd, short what, void* v) {
    IOHandler* ioHandler = (IOHandler*) v;
    while (true) {
        Connection* connection = 0;
        const int kSize = sizeof (connection);
        long nBytes = recv(fd, reinterpret_cast<void*> (&connection), kSize, 0);
        if (nBytes == kSize) {
            if (connection == NULL) {
                return;
            }
            connection->transition();
        } else if (nBytes > 0) {
            ioHandler->stop();
            return;
        } else if (nBytes == 0) {
            // Exit the loop
            break;
        } else {
            ioHandler->stop();
            return;
        }
        break;
    }
}

void IOHandler::stop() {
    event_base_loopbreak(eventBase_);
    return;
}
