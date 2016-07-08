/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Server.cpp
 * Author: ductn
 * 
 * Created on June 27, 2016, 3:20 PM
 */

#include "NonblockingServer.h"

Server::Server(int port, int ioHandlerNum, int workerNum) {
    port_ = port;
    serverSocket_ = 0;
    threadManager_ = NULL;
    eventBase_ = NULL;
    ioHandlerNum_ = ioHandlerNum;
    workerNum_ = workerNum;
    currentIOHandler_ = 0;
}

void Server::createAndListenSocket() {
    int fd, error;
    char port[sizeof ("65536") + 1];
    struct addrinfo hints, *res, *res0;
    // Initialize hints to getaddrinfo
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    sprintf(port, "%d", port_);
    if ((error = getaddrinfo(NULL, port, &hints, &res0))) {
        std::cout << "getaddrinfo() error\n";
        return;
    }
    // Prefer IPv6
    for (res = res0; res != NULL; res = res->ai_next) {
        if (res->ai_family == AF_INET6 || res->ai_next == NULL) {
            break;
        }
    }
    // Create socket fd
    if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
        freeaddrinfo(res0);
        std::cout << "socket() error\n";
        return;
    }
    // Bind fd to addr
    if (bind(fd, res->ai_addr, static_cast<int> (res->ai_addrlen)) == -1) {
        freeaddrinfo(res0);
        std::cout << "bind() error\n";
        return;
    }
    // Done with resource, free
    freeaddrinfo(res0);
    // Set socket fd to nonblock
    int flags;
    if ((flags = fcntl(fd, F_GETFL)) < 0 || ((fcntl(fd, F_SETFD, flags | O_NONBLOCK) < 0))) {
        std::cout << "fcntl() NONBLOCK error\n";
        return;
    }
    // Make fd listening
    if (listen(fd, BACKLOG) == -1) {
        std::cout << "listen() error\n";
        return;
    }
    std::cout << "Server listening on port " << port_ << std::endl;
    // Done job, return
    serverSocket_ = fd;
}

void Server::serve() {
    Poco::ThreadPool threadPool;
    threadPool_ = new Poco::Thread[ioHandlerNum_];
    if (!serverSocket_) {
        createAndListenSocket();
    }
    threadManager_ = new ThreadManager(workerNum_);
    Poco::Thread threadManagerThread;
    threadManagerThread.start(*threadManager_);
    // Initialize io handler and run it
    for (int i = 0; i < ioHandlerNum_; i++) {
        int listenFD = (i == 0 ? serverSocket_ : -1);
        IOHandler* ioHandler = new IOHandler(this, listenFD, i);
        ioHandler_.push_back(ioHandler);
        if (i != 0) {
            threadPool_[i].start(*ioHandler);
        }
    }
    
    ioHandler_[0]->run();
    // Wait for ioHandler finish
    // ioHandler_->join();
}

void Server::stop() {
    // ioHanlder_->stop();
}

void Server::listenHandler(int fd, short what) {
    struct sockaddr_storage addrStorage;
    socklen_t addrLen = sizeof addrStorage;
    int clientSocket = 0;
    if ((clientSocket = accept(fd, (sockaddr*) & addrStorage, &addrLen)) == -1) {
        std::cout << "accept() error\n";
    }
    // Set socket fd to nonblock
    int flags;
    if ((flags = fcntl(clientSocket, F_GETFL)) < 0 || 
            ((fcntl(clientSocket, F_SETFD, flags | O_NONBLOCK) < 0))) {
        std::cout << "fcntl() NONBLOCK error\n";
        return;
    }

    Connection* clientConnection = createConnection(clientSocket);

    if (clientConnection == NULL) {
        std::cout << "createConnection() error\n";
        return;
    }

    clientConnection->transition();
}

IOHandler* Server::getIOHandler() {
    int selectedIOHandler = (++currentIOHandler_) % ioHandlerNum_;
    return ioHandler_[selectedIOHandler];
}

ThreadManager* Server::getThreadManager() {
    return threadManager_;
}

void Server::setWorkerNum(int workerNum) {
    workerNum_ = workerNum;
}

event_base* Server::getEventBase() {
    return eventBase_;
}

void Server::setEventBase(event_base* eventBase) {
    eventBase_ = eventBase;
}

Connection* Server::createConnection(int fd) {
    static int count = 0;
    std::lock_guard<std::mutex> guard(mutex);
    count++;
    std::cout << "Connections created: " << count << std::endl;
    Connection* conn = NULL;
    if (stackConnections_.empty()) {
        conn = new Connection(this, fd);
    } else {
        conn = stackConnections_.top();
        stackConnections_.pop();
        conn->init(fd);
    }
    
    if (conn == NULL) {
        std::cout << "createConnection() error\n";
    } else {
        activeConnections_.push_back(conn);
    }

    return conn;
}

void Server::closeConnection(Connection* conn) {
    static int count = 0;
    std::lock_guard<std::mutex> guard(mutex);
    count++;
    for (int i = 1; i < ioHandlerNum_; i ++) {
        std::cout << "IO Handler number " << i << " still running: " << threadPool_[i].isRunning() << std::endl;
    }
    if (conn) {
        activeConnections_.erase(std::remove(activeConnections_.begin(),
                activeConnections_.end(), conn), activeConnections_.end());
        stackConnections_.push(conn);
        conn->closeConnection();
    }
}
