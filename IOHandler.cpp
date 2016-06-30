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
}

void IOHandler::registerEvents() {
    // Initialize event base
    eventBase_ = event_base_new();
    
    if (listenSocket_ >= 0) {
        listenEvent_ = event_new(eventBase_, listenSocket_, EV_READ|EV_PERSIST, 
                IOHandler::listenCallback, server_);
        event_add(listenEvent_, 0);
    } else {
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
    
    event_base_loop(eventBase_, 0);
}

event_base* IOHandler::getEventBase() {
    return eventBase_;
}
