/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "NonblockingServer.h"


Processor::Processor(Connection* connection) : protocol_(new Protocol) {
    connection_ = connection;
}

void Processor::proccess() {
    std::string readContent = protocol_->processInput(connection_->getReadBuffer());
    if (readContent.empty()) {
        std::cout << "process() error\n";
        return;
    } else {
        // Cut all garbage from content;
        readContent = readContent.substr(0, connection_->getMessageSize());
    }
    char* writeContent = protocol_->processOutput(readContent);
    if (writeContent == NULL) {
        std::cout << "process() error\n";
    }
    connection_->setwriteBuffer(writeContent);
    connection_->setwriteBufferSize(sizeof(writeContent));
}
