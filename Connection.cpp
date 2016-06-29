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

#include "Connection.h"

using namespace server;

Connection::Connection(Server* server, int socket) {
    server_ = server;
    ioHandler_ = server->getIOHandler();
    connectionSocket_ = socket;
    socketState_ = SOCKET_RECV;
    connectionState_ = CONN_INIT;
}

