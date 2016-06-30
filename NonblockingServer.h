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

#include <cstdlib>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <stack>
#include <mutex>
#include <assert.h>
#include <boost/shared_ptr.hpp>
#include <event.h>
#include <event2/event_compat.h>
#include <event2/event_struct.h>
#include <Poco/Runnable.h>

enum SocketState {
    SOCKET_RECV_FRAMING,
    SOCKET_RECV,
    SOCKET_SEND
};

enum ConnectionState {
    CONN_INIT,
    CONN_RECV_FRAMING,
    CONN_RECV,
    CONN_WAIT,
    CONN_SEND,
    CONN_CLOSE
};

union Framing {
    uint8_t buf[sizeof (uint32_t)];
    uint32_t size;
};

class IOHandler;
class Connection;
class Processor;
class Protocol;

class Server {
public:
    Server();
    Server(int port);
    void createAndListenSocket();
    void serve();
    void stop();
    void listenHandler(evutil_socket_t fd, short what);
    IOHandler* getIOHandler();
    void closeConnection(Connection* conn);
private:
    static const int BACKLOG = 100;
    static const int STACK_SIZE = 100;
    int port_;
    int serverSocket_;
    IOHandler *ioHandler_;
    std::stack<Connection*> stackConnections_;
    std::vector<Connection*> activeConnections_;
    std::mutex mutex;
    Connection* createConnection(int fd);
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

class Processor {
public:
    Processor(Connection* connection);
    void proccess();
private:
    Connection* connection_;
    boost::shared_ptr<Protocol> protocol_;
};

class Connection {
public:
    Connection(Server* server, int socket);
    void init(int socket);
    void closeConnection();
    static void workCallback(evutil_socket_t fd, short what, void *arg);
    void workHandler(evutil_socket_t fd, short what);
    void transition();
    char* getReadBuffer();
    uint32_t getMessageSize();
    void setwriteBuffer(char* content);
    void setwriteBufferSize(int size);
private:
    int connectionSocket_;
    IOHandler* ioHandler_;
    Server* server_;
    struct event event_;
    short eventFlags_;
    SocketState socketState_;
    ConnectionState connectionState_;
    uint32_t messageSize_;
    uint8_t* readBuffer_;
    uint32_t readBufferPos_;
    uint32_t readBufferSize_;
    uint8_t* writeBuffer_;
    uint32_t writeBufferPos_;
    uint32_t writeBufferSize_;
    Processor* processor_;
    void setRead();
    void setWrite();
    void setIdle();
    void setFlags(short flags);
};

class Protocol {
public:
    std::string processInput(char* inMessage);
    char* processOutput(std::string outMessage);
};

#endif /* NONBLOCKINGSERVER_H */

