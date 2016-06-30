/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   NonblockingServer.h
 * Author: duc
 *
 * Created on June 29, 2016, 8:04 PM
 */

#ifndef NONBLOCKINGSERVER_H
#define NONBLOCKINGSERVER_H

#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <stack>
#include <event.h>
#include <event2/event_compat.h>
#include <event2/event_struct.h>
#include <Poco/Runnable.h>

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

class IOHandler;
class Connection;

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

class IOHandler : Poco::Runnable {
public:
    IOHandler();
    IOHandler(Server* server, int serverSocket);
    void registerEvents();
    static void listenCallback(evutil_socket_t fd, short what, void *arg);
    void run();
    event_base* getEventBase();
private:
    Server* server_;
    event_base* eventBase_;
    event* listenEvent_;
    event* notificationEvent_;
    int listenSocket_;
};

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
};

#endif /* NONBLOCKINGSERVER_H */

