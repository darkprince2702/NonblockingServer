/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "NonblockingServer.h"

Client::Client(int port) : protocol_(new Protocol()) {
    port_ = port;
    clientConnect();
}

Client::~Client() {
    if (clientSocket_) {
        clientDisconnect();
    }
}

void Client::clientConnect() {
    int clientSocket, status;
    struct addrinfo hints, *res, *res0;
    struct sockaddr_storage serverAddr;

    memset(&hints, 0, sizeof (hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    char port[sizeof ("65536") + 1];
    sprintf(port, "%d", port_);
    if ((status = getaddrinfo(NULL, port, &hints, &res)) != 0) {
        std::cout << "Get host address error" << gai_strerror(status) << std::endl;
        return;
    }
    for (res0 = res; res0 != NULL; res0 = res0->ai_next) {
        if ((clientSocket =
                socket(res0->ai_family, res0->ai_socktype, res0->ai_protocol))
                == -1) {
            std::cout << "Get socket FD error" << std::endl;
            continue;
        }

        if (connect(clientSocket, res0->ai_addr, res0->ai_addrlen) == -1) {
            close(clientSocket);
            perror("Connect error");
            continue;
        }
        break;
    }

    if (clientSocket) {
        clientSocket_ = clientSocket;
    }
}

void Client::clientDisconnect() {
    close(clientSocket_);
}

void Client::clientSend(Operator* o) {
    std::shared_ptr<Message> message(protocol_->processOutput(o));
    int writePos = 0, writeSize = message->size, left, sent;
    while (writePos < writeSize) {
        left = writeSize - writePos;
        sent = write(clientSocket_, message->content + writePos, left);
        writePos += sent;
    }
}

Operator* Client::clientReceive() {
    int readPos = 0, readSize, fetch, got;
    // Read framing size
    Framing framing;
    framing.size = 0;
    readSize = sizeof (framing.size);
    while (readPos < readSize) {
        fetch = read(clientSocket_, &framing.buf[readPos],
                (size_t) (readSize - readPos));
        readPos += fetch;
    }
    // Read message;
    uint8_t* buffer = new uint8_t[framing.size]; // TODO: shared_ptr
    readSize = framing.size;
    fetch = 0;
    while (readPos < readSize) {
        fetch = readSize - readPos;
        got = read(clientSocket_, buffer + readPos, fetch);
        readPos += got;
    }
    Message receive;
    receive.content = buffer;
    receive.size = readSize;
    return protocol_->processInput(&receive);
}

bool Client::set(std::string key, std::string value) {
    boost::shared_ptr<Operator> o(new Operator());
    o->type = "set";
    o->key = key;
    o->value = value;

    clientSend(o.get());
    o.reset(clientReceive());

    assert(o->type == "set");
    return (o->result == "1") ? true : false;
}

std::string Client::get(std::string key) {
    boost::shared_ptr<Operator> o(new Operator());
    o->type = "get";
    o->key = key;

    clientSend(o.get());
    o.reset(clientReceive());

    assert(o->type == "get");
    return o->result;
}

bool Client::remove(std::string key) {
    boost::shared_ptr<Operator> o(new Operator());
    o->type = "remove";
    o->key = key;

    clientSend(o.get());
    o.reset(clientReceive());

    assert(o->type == "remove");
    return (o->result == "1") ? true : false;
}
