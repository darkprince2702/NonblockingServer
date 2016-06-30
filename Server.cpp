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

Server::Server(int port) {
    port_ = port;
    serverSocket_ = NULL;
    ioHandler_ = NULL;
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
    if (!serverSocket_) {
        createAndListenSocket();
    }
    // Initialize io handler and run it
    ioHandler_ = new IOHandler(this, serverSocket_);
    ioHandler_->run();
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
    return ioHandler_;
}

Connection* Server::createConnection(int fd) {
    std::lock_guard<std::mutex> guard(mutex);
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
    std::lock_guard<std::mutex> guard(mutex);
    if (conn) {
        activeConnections_.erase(std::remove(activeConnections_.begin(),
                activeConnections_.end(), conn), activeConnections_.end());
        stackConnections_.push(conn);
        conn->closeConnection();
    }
}
