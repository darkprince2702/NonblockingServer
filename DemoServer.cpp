/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   server.cpp
 * Author: ductn
 *
 * Created on June 27, 2016, 11:16 AM
 */

#include <cstdlib>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <fcntl.h>
#include <event2/util.h>
#include <event2/event.h>
#include <event2/event_compat.h>
#include "NonblockingServer.h"

#define PORT "5050"

/*
 * server.cpp - a demo socket server
 */

int main(int argc, char** argv) {
    Server* server = new Server(5050, 4, 4);
    server->serve();
}