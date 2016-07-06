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
    eventFlags_ = 0;
    socketState_ = SOCKET_RECV;
    connectionState_ = CONN_INIT;
    readBuffer_ = NULL;
    readBufferSize_ = 0;
    writeBufferSize_ = 0;
    processor_ = new Processor(this);
}

void Connection::init(int socket) {
    connectionSocket_ = socket;
    eventFlags_ = 0;
    socketState_ = SOCKET_RECV;
    connectionState_ = CONN_INIT;
}

void Connection::transition() {
    switch (connectionState_) {
        case CONN_RECV_FRAMING:
            if (messageSize_ > readBufferSize_) {
                delete readBuffer_;
                uint8_t* newBuffer = new uint8_t[messageSize_];
                readBuffer_ = newBuffer;
                readBufferSize_ = messageSize_;
            }

            readBufferPos_ = 0;

            // Move into receive state
            socketState_ = SOCKET_RECV;
            connectionState_ = CONN_RECV;
            return;
        case CONN_RECV:
            // Received message, give task to threadpool for processin 
            notifyThreadManager(new Task(this));
//            processor_->proccess();
//            setConnectionState(CONN_WAIT);
//            notifyIOHanlder();
            return;
        case CONN_WAIT:
            if (writeBufferSize_ > 4) {
                // Move into write state
                writeBufferPos_ = 0;
                socketState_ = SOCKET_SEND;
                // Socket into write mode
                connectionState_ = CONN_SEND;
                setWrite();
                return;
            }
            return;
        case CONN_SEND:
            // Turn back to init state
        case CONN_INIT:
            // Clear write buffer
            writeBuffer_ = NULL;
            writeBufferPos_ = 0;
            writeBufferSize_ = 0;
            // Move into receive framing state
            socketState_ = SOCKET_RECV_FRAMING;
            connectionState_ = CONN_RECV_FRAMING;

            readBufferPos_ = 0;
            // Re-register events
            setRead();

            return;
        case CONN_CLOSE:
            server_->closeConnection(this);
            return;
        default:
            std::cout << "transition() error\n";
            return;
    }
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

void Connection::setFlags(short flags) {
    if (eventFlags_ == flags) {
        return;
    }

    if (eventFlags_ != 0) {
        if (event_del(&event_) == -1) {
            std::cout << "event_del() error\n";
            return;
        }
    }
    eventFlags_ = flags;
    event_set(&event_, connectionSocket_, eventFlags_, Connection::workCallback, this);
    event_base_set(ioHandler_->getEventBase(), &event_);
    if (event_add(&event_, 0) == -1) {
        std::cout << "event_add() error\n";
        return;
    }
}

void Connection::workCallback(evutil_socket_t fd, short what, void *arg) {
    ((Connection*) arg)->workHandler(fd, what);
}

void Connection::workHandler(int fd, short what) {
    int got = 0, left = 0, sent = 0;
    uint32_t fetch = 0;
    switch (socketState_) {
        case SOCKET_RECV_FRAMING:
            // Restore received byte
            Framing framing;
            framing.size = messageSize_;
            fetch = read(fd, &framing.buf[readBufferPos_],
                    (size_t) uint32_t(sizeof (framing.size) - readBufferPos_));
            if (fetch == 0) {
                std::cout << "Connection close\n";
                server_->closeConnection(this);
                return;
            }
            readBufferPos_ += fetch;
            if (readBufferPos_ < sizeof (framing.size)) {
                messageSize_ = framing.size;
                return;
            }
            messageSize_ = framing.size;
            transition();
            return;

        case SOCKET_RECV:
            fetch = messageSize_ - readBufferPos_;
            got = read(fd, readBuffer_ + readBufferPos_, fetch);
            if (got > 0) {
                readBufferPos_ += got;
                if (readBufferPos_ == messageSize_) { // Read done
                    transition();
                }
                return;
            }
            return;

        case SOCKET_SEND:
            assert(writeBufferPos_ <= writeBufferSize_);
            if (writeBufferPos_ == writeBufferSize_) {
                std::cout << "Send with no data\n";
                transition();
                return;
            }
            left = writeBufferSize_ - writeBufferPos_;
            sent = write(fd, writeBuffer_ + writeBufferPos_, left);
            writeBufferPos_ += sent;
            if (writeBufferPos_ == writeBufferSize_) {
                transition();
            }
            return;

        default:
            std::cout << "workHandler() error\n";
            return;
    }
}

uint8_t* Connection::getReadBuffer() {
    return readBuffer_;
}

void Connection::setwriteBuffer(uint8_t* content) {
    writeBuffer_ = content;
}

void Connection::setwriteBufferSize(uint32_t size) {
    writeBufferSize_ = size;
}

uint32_t Connection::getMessageSize() {
    return messageSize_;
}

void Connection::closeConnection() {
    event_del(&event_);
    ioHandler_ = NULL;
    close(connectionSocket_);
}

void Connection::notifyIOHanlder() {
    ioHandler_->notify(this);
}

Processor* Connection::getProcessor() {
    return processor_;
}

ThreadManager* Connection::getThreadManager() {
    return server_->getThreadManager();
}

void Connection::setConnectionState(ConnectionState cs) {
    connectionState_ = cs;
}

bool Connection::notifyThreadManager(Task* task) {
    int fd = getThreadManager()->getTaskNotificationSendFD();
    if (fd < 0) {
        return false;
    }

    fd_set wfds, efds;
    int ret = -1;
    int kSize = sizeof (task);
    const char* pos = (const char*) reinterpret_cast<void*> (&task);

    while (kSize > 0) {
        FD_ZERO(&wfds);
        FD_ZERO(&efds);
        FD_SET(fd, &wfds);
        FD_SET(fd, &efds);
        ret = select(fd + 1, NULL, &wfds, &efds, NULL);
        if (ret < 0) {
            return false;
        } else if (ret == 0) {
            continue;
        }
        if (FD_ISSET(fd, &efds)) {
            close(fd);
            return false;
        }
        if (FD_ISSET(fd, &wfds)) {
            ret = send(fd, pos, kSize, 0);
            if (ret < 0) {
                if (errno == EAGAIN) {
                    continue;
                }
                close(fd);
                return false;
            }
            kSize -= ret;
            pos += ret;
        }
    }
    return true;
}
