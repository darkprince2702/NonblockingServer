/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "NonblockingServer.h"


Processor::Processor(Connection* connection) {
    connection_ = connection;
    protocol_ = new Protocol();
}

void Processor::proccess() {
    Message inMessage;
    inMessage.content = connection_->getReadBuffer();
    inMessage.size = connection_->getMessageSize();
    
    std::string input = protocol_->processInput(&inMessage);
    // TODO: process the input
    std::string output = input;    // dummy processor
    
    Message* outMessage = protocol_->processOutput(output);
    connection_->setwriteBuffer(outMessage->content);
    connection_->setwriteBufferSize(outMessage->size);
}
