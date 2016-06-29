/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   server.cpp
 * Author: ductn
 *
 * Created on June 27, 2016, 11:16 AM
 */

#include <cstdlib>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <fcntl.h>
#include <event2/util.h>
#include <event2/event.h>
#include <event2/event_compat.h>

#define PORT "5050"

/*
 * server.cpp - a demo socket server
 */

class client {
public:
    struct event *clientEvent;
};

void clientCallback(evutil_socket_t fd, short what, void *arg) {
    client *currentClient = (client*) arg;
    char buffer[1000];
    int readLength, writeLength;
    readLength = read(fd, buffer, sizeof buffer);
    if (readLength == 0) {
        std::cout << "Client disconnected" << std::endl;
        close(fd);
        event_free(currentClient->clientEvent);
        delete currentClient;
        return;
    } else if (readLength < 0) {
        std::cout << "Socket error" << std::endl;
        close(fd);
        event_free(currentClient->clientEvent);
        delete currentClient;
        return;
    }

    writeLength = write(fd, buffer, sizeof buffer);
    if (writeLength < readLength) {
        std::cout << "Buffer small than content" << std::endl;
    }

}

void acceptCallback(evutil_socket_t fd, short what, void *arg) {
    event_base* eventBase = (event_base*) arg;
    int clientFD;
    struct sockaddr_storage clientAddr;
    socklen_t addrLen = sizeof clientAddr;
    // Waiting to accept new client
    clientFD = accept(fd, (struct sockaddr *) &clientAddr, &addrLen);
    if (clientFD == -1) {
        std::cout << "Accept error" << std::endl;
        return;
    }
    std::cout << "New client connected" << std::endl;
    client* newClient = new client();
    newClient->clientEvent = event_new(eventBase, clientFD, EV_READ | EV_PERSIST, clientCallback, newClient);
    event_add(newClient->clientEvent, 0);
}

int setIdle(int fd) {

}

int setRead(int fd) {

}

int setWrite(int fd) {

}

int setFDNonblock(int fd) {
    int flags;
    flags = fcntl(fd, F_GETFL);
    if (flags < 0) {
        return flags;
    }
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) < 0) {
        return -1;
    }

    return 0;
}

int main(int argc, char** argv) {
    int socketFD, acceptFD, status;
    struct addrinfo hints, *res, *p;
    struct sockaddr_storage clientAddress;
    struct event_base* eventBase = event_base_new();
    struct event* acceptEvent;
    // Get host address
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((status = getaddrinfo(NULL, PORT, &hints, &res)) != 0) {
        std::cout << "Get host address error" << gai_strerror(status) << std::endl;
        return 1;
    }
    // Loop through gotten address and bind to socket
    for (p = res; p != NULL; p = p->ai_next) {
        if ((socketFD = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            std::cout << "Get socket FD error" << std::endl;
            continue;
        }

        if (bind(socketFD, p->ai_addr, p->ai_addrlen) == -1) {
            close(socketFD);
            perror("Bind error");
            continue;
        }

        break;
    }

    freeaddrinfo(res); // Struct is not neccessary any more

    if (p == NULL) {
        std::cout << "Not found host to bind" << std::endl;
        exit(1);
    }

    // Listen on PORT
    if (listen(socketFD, 10) == -1) {
        std::cout << "Listen error" << std::endl;
        exit(1);
    }
    // Set port to nonblock
    if (setFDNonblock(socketFD) < 0) {
        std::cout << "Can't set socket to nonblock" << std::endl;
    }
    std::cout << "Server listening on port " << PORT << std::endl;
    // Init accept event
    acceptEvent = event_new(eventBase, socketFD, EV_READ | EV_PERSIST, acceptCallback, eventBase);
    event_add(acceptEvent, 0);
    // Enter event base loop
    event_base_loop(eventBase, 0);
    return 0;
}

