/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Connection.cpp
 * Author: ductn
 * 
 * Created on June 29, 2016, 2:04 PM
 */

#include "NonblockingServer.h"

Connection::Connection(Server* server, int socket) {
    server_ = server;
    ioHandler_ = server_->getIOHandler();
    connectionSocket_ = socket;
    socketState_ = SOCKET_RECV;
    connectionState_ = CONN_INIT;
}

void Connection::init(int socket) {
    connectionSocket_ = socket;
    socketState_ = SOCKET_RECV;
    connectionState_ = CONN_INIT;
}

void Connection::transition() {
    return;
}

void Connection::setRead() {
    setFlags(EV_READ | EV_PERSIST);
}

void Connection::setWrite() {
    setFlags(EV_WRITE | EV_PERSIST);
}

void Connection::setIdle() {
    setFlags(0);
}




