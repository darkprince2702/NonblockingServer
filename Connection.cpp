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
                /*
                if (readBufferSize_ == 0) {
                    readBufferSize_ = 1;
                }
                uint32_t newSize = readBufferSize_;
                while (messageSize_ > newSize) {
                    newSize *= 2;
                }
                
                uint8_t* newBuffer = (uint8_t*) std::realloc(readBuffer_, messageSize_);
                if (newBuffer == NULL) {
                    // nothing else to be done...
                    throw std::bad_alloc();
                }
                readBuffer_ = newBuffer;
                 */
                delete readBuffer_;
                readBuffer_ = new u_int8_t[messageSize_];
                readBufferSize_ = messageSize_;
            }

            readBufferPos_ = 0;

            // Move into receive state
            socketState_ = SOCKET_RECV;
            connectionState_ = CONN_RECV;
            return;

        case CONN_RECV:
            // Received message, give task to threadpool for processing
            processor_->proccess();
            connectionState_ = CONN_WAIT;
            transition();
            return;

        case CONN_WAIT:
            if (writeBufferSize_ > 4) {
                // Move into write state
                writeBufferPos_ = 0;
                socketState_ = SOCKET_SEND;

                // Put the frame size into the write buffer
                int32_t frameSize = (int32_t) htonl(writeBufferSize_ - 4);
                memcpy(writeBuffer_, &frameSize, 4);

                // Socket into write mode
                connectionState_ = CONN_SEND;
                setWrite();
                return;
            }
            return;
        case CONN_SEND:
            // TODO: limit writeBufferSize_

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
            sent = write(fd, readBuffer_ + writeBufferPos_, left);
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

char* Connection::getReadBuffer() {
    char* result = reinterpret_cast<char*> (readBuffer_);
    return result;
}

void Connection::setwriteBuffer(char* content) {
    writeBuffer_ = (uint8_t*) content;
}

void Connection::setwriteBufferSize(int size) {
    writeBufferSize_ = (uint32_t) size;
}

uint32_t Connection::getMessageSize() {
    return messageSize_;
}

void Connection::closeConnection() {
    event_del(&event_);
    close(connectionSocket_);
}
