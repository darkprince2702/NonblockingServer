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
    if (object->type == "set") {
        object->result = std::to_string((int) handler_->set(object->key, object->value));
    } else if (object->type == "get") {
        object->result = handler_->get(object->key);
    } else if (object->type == "remove") {
        object->result = std::to_string((int) handler_->remove(object->key));
    } else {
        std::cout << "process() error\n";
    }

    Message* outMessage = protocol_->processOutput(object);
    connection_->setwriteBuffer(outMessage->content);
    connection_->setwriteBufferSize(outMessage->size);
}
