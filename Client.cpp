/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "NonblockingServer.h"


Client::Client(int port) : protocol_(new Protocol()) {
    port_ = port;
    connect();
}

Client::~Client() {
    if (clientSocket_) {
        disconnect();
    }
}

void Client::connect() {
    int clientSocket, status;
    struct addrinfo hints, *res, *res0;
    struct sockaddr_storage serverAddr;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((status = getaddrinfo(NULL, port_, &hints, &res)) != 0) {
		std::cout << "Get host address error" << gai_strerror(status) << std::endl;
		return 1;
	}
    for (res0 = res; res0 != NULL; res0 = res0->ai_next) {
        if ((clientSocket = 
                socket(res0->ai_family, res0->ai_socktype, res0->ai_protocol)) 
                == -1) {
			std::cout << "Get socket FD error" << std::endl;
			continue;
		}

		if(connect(clientSocket, res0->ai_addr, res0->ai_addrlen) == -1) {
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


void Client::disconnect() {
    close(clientSocket_);
}



bool Client::set(std::string key, std::string value) {
    // Init operator struct, then pass to 
    Operator o;
    o.type = "set";
    o.key = key;
    o.value = value;

    std::shared_ptr<Message> message(protocol_->processInput(&o));
    int writePos = 0, writeSize = message->size, left, sent;
    while (writePos < writeSize) {
        left = writeSize - writePos;
        sent = write(clientSocket_, message->content + writePos, left);
        writePos += sent;
    }
    
    
}

