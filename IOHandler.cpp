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

IOHandler::IOHandler(Server* server, int serverSocket, int ID) {
    server_ = server;
    listenSocket_ = serverSocket;
    ID_ = ID;
    eventBase_ = NULL;
//    notificationEvent_ = NULL;
}

void IOHandler::registerEvents() {
    // Initialize event base
    if (eventBase_ == NULL) {
        eventBase_ = event_base_new();
    }

    if (listenSocket_ >= 0) {
        event_set(&listenEvent_, listenSocket_, EV_READ | EV_PERSIST, IOHandler::listenCallback, server_);
        event_base_set(eventBase_, &listenEvent_);
        //        listenEvent_ = event_new(eventBase_, listenSocket_, EV_READ | EV_PERSIST,
        //                IOHandler::listenCallback, server_);
        if (event_add(&listenEvent_, 0) == -1) {
            std::cout << "registerEvent error\n";
        }
    }

    createNotificationPipe();
    event_set(&notificationEvent_, getNotificationRecvFD(), EV_READ | EV_PERSIST, 
            IOHandler::notificationHandler, this);
    event_base_set(eventBase_, &notificationEvent_);
//    notificationEvent_ = event_new(eventBase_, getNotificationRecvFD(),
//            EV_READ | EV_PERSIST, IOHandler::notificationHandler, this);
    if (event_add(&notificationEvent_, 0) == -1) {
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

    std::cout << "IOHandler " << ID_ << " entering loop...\n";

//    while (true) {
        int ret = event_base_loop(eventBase_, 0);
//        registerEvents();
//        event_add(&notificationEvent_, 0);
        std::cout << "event_base_loop return: " << ret << std::endl;
        std::cout << "IO THREAD " << ID_ << " EXIT!!!!!!!!\n";
//    }
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
    if (ID_ != 0) {
        std::cout << "notify start\n";
    }
    std::lock_guard<std::mutex> guard(mutex);
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
    if (ID_ != 0) {
        std::cout << "notify end\n";
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
    assert(ioHandler != NULL);
    if (ioHandler->ID_ != 0) {
        std::cout << "notify handler start\n";
    }
    while (true) {
        Connection* connection = 0;
        const int kSize = sizeof (connection);
        long nBytes = recv(fd, reinterpret_cast<void*> (&connection), kSize, 0);
        if (nBytes == kSize) {
            if (connection == NULL) {
                std::cout << "notify a NULL connection\n";
                return;
            }
            // Check if this connection is closed
            if (connection->getConnectionState() == CONN_WAIT) {
                connection->transition();
            } else {
                std::cout << "notify a obsolete connection\n";
            }
        } else if (nBytes > 0) {
            ioHandler->stop();
            return;
        } else if (nBytes == 0) {
            std::cout << "notification socket closed\n";
            break;
        } else {
            ioHandler->stop();
            return;
        }
        break;
    }
    if (ioHandler->ID_ != 0) {
        std::cout << "notify handler end\n";
    }
}

void IOHandler::stop() {
    std::cout << "notify error, IO handler break\n";
    //    event_base_loopbreak(eventBase_);
    return;
}
