/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   IOThread.h
 * Author: ductn
 *
 * Created on June 29, 2016, 11:08 AM
 */

#ifndef IOHANDLER_H
#define IOHANDLER_H

#include <event.h>
#include <event2/event_compat.h>
#include <event2/event_struct.h>
#include <Poco/Runnable.h>
#include <iostream>
#include "Server.h"

namespace server {

    class IOHandler : Poco::Runnable {
    public:
        IOHandler();
        IOHandler(Server* server, int serverSocket);
        void registerEvents();
        void listenCallback(evutil_socket_t fd, short what, void *arg);
        void run();
        event_base* getEventBase();
    private:
        Server server_;
        event_base* eventBase_;
        event* listenEvent_;
        event* notificationEvent_;
        int listenSocket_;
    };
}
#endif /* IOTHREAD_H */

