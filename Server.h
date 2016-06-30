/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Server.h
 * Author: ductn
 *
 * Created on June 27, 2016, 3:20 PM
 */
/*
#ifndef SERVER_H
#define SERVER_H

#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <stack>
#include <event.h>

namespace server {    
    class Server {
    public:
        Server();
        Server(int port);
        void createAndListenSocket();
        void serve();
        void stop();
        void listenHandler(evutil_socket_t fd, short what);
        IOHandler* getIOHandler();
    private:
        static const int BACKLOG = 100;
        static const int STACK_SIZE = 100;
        int port_;
        int serverSocket_;
        IOHandler *ioHandler_;
        std::stack<Connection*> stackConnections_;
        std::vector<Connection*> activeConnections_;
        Connection* createConnection(int fd);
        void closeConnection(Connection* conn);
    };
}
#endif /* SERVER_H */

