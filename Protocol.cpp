/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "NonblockingServer.h"

std::string Protocol::processInput(char* inMessage) {
    std::string output(inMessage);
    return output;
}

char* Protocol::processOutput(std::string outMessage) {
    char* result = new char[outMessage.size() + 1];
    strcpy(result, outMessage.c_str());
    return result;
}
