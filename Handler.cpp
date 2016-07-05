/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "NonblockingServer.h"


Handler* Handler::getInstance() {
    static Handler instance;
    return &instance;
}

bool Handler::set(std::string key, std::string value) {
    std::lock_guard<std::mutex> guard(mutex_);
    data_[key] = value;
    return true;
}

std::string Handler::get(std::string key) {
    std::lock_guard<std::mutex> guard(mutex_);
    return data_[key];
}

bool Handler::remove(std::string key) {
    std::lock_guard<std::mutex> guard(mutex_);
    if (data_.erase(key)) {
        return true;
    } else {
        return false;
    }
}



