/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "NonblockingServer.h"


Processor::Processor(Connection* connection) {
    connection_ = connection;
    protocol_ = new Protocol();
    handler_ = Handler::getInstance();
}

void Processor::proccess() {
    Message inMessage;
    inMessage.content = connection_->getReadBuffer();
    inMessage.size = connection_->getMessageSize();
    
    Operator* object = protocol_->processInput(&inMessage);
    switch (object->type) {
        case "set":
            object->result = std::string(handler_->set(object->key, object->value));
            return;
        case "get":
            object->result = std::string(handler_->get(object->key));
            return;
        case "remove":
            object->result = std::string(handler_->remove(object->key));
            return;
    }
    
    Message* outMessage = protocol_->processOutput(object);
    connection_->setwriteBuffer(outMessage->content);
    connection_->setwriteBufferSize(outMessage->size);
}
