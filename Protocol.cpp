/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "NonblockingServer.h"

Operator* Protocol::processInput(const Message* inMessage) {
    // Cast from uint8_t array to string
    std::string output(reinterpret_cast<char*>(inMessage->content));
    nlohmann::json o = nlohmann::json::parse(output);
    Operator* result = new Operator();
    if (o.count("type")) {
        result->type = o["type"];
    }
    if (o.count("key")) {
        result->key = o["key"];
    }
    if (o.count("value")) {
        result->value = o["value"];
    }
    return result;
}

Message* Protocol::processOutput(Operator* result) {
    nlohmann::json o;
    if (!result->type.empty()) {
        o["type"] = result->type;
    } 
    if (!result->key.empty()) {
        o["key"] = result->key;
    } 
    if (!result->value.empty()) {
        o["value"] = result->value;
    } 
    if (!result->result.empty()) {
        o["result"] = result->result;
    }
    std::string outMessage = o.dump();
    // Get size of send message (+1 for \0)
    uint32_t outputSize = (uint32_t) outMessage.size() + 1;
    // Convert from string to uint8_t, TODO: simplify
    char charArray[outMessage.size()];
    strcpy(charArray, outMessage.c_str());
    uint8_t* uintArray = reinterpret_cast<uint8_t*>(charArray);
    uint8_t* outputMessage = new uint8_t[outputSize + 4];
    // Write framing size and message
    memcpy(outputMessage, &outputSize, 4);
    memcpy(outputMessage + 4, uintArray, outputSize);
    Message* return_ = new Message();
    return_->content = outputMessage;
    return_->size = outputSize + 4;
    return return_;
}
