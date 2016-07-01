/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "NonblockingServer.h"

std::string Protocol::processInput(const Message* inMessage) {
    // Cast from uint8_t array to string
    std::string output(reinterpret_cast<char*>(inMessage->content));
    return output;
}

Message* Protocol::processOutput(std::string outMessage) {
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
    Message* result = new Message();
    result->content = outputMessage;
    result->size = outputSize + 4;
    return result;
}
