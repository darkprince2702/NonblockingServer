/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Connection.h
 * Author: ductn
 *
 * Created on June 29, 2016, 2:04 PM
 */

#ifndef CONNECTION_H
#define CONNECTION_H

#include "IOHandler.h"

enum SocketState {
    SOCKET_RECV,
    SOCKET_SEND
};

enum ConnectionState {
    CONN_INIT,
    CONN_RECV,
    CONN_WAIT,
    CONN_SEND,
    CONN_CLOSE
};

namespace server {

    class Connection {
    public:
        Connection(Server* server, int socket);
        void init(int socket);
        void workCallback();
        void transition();
    private:
        int connectionSocket_;
        IOHandler* ioHandler_;
        Server* server_;
        struct event event_;
        short eventFlags_;
        SocketState socketState_;
        ConnectionState connectionState_;
        void setRead();
        void setWrite();
        void setIdle();
        void setFlags(short flags);
        void registerEvents();
    };
}
#endif /* CONNECTION_H */

