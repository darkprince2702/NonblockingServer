/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "NonblockingServer.h"

Task::Task(Connection* conn) {
    connection_ = conn;
    isObsolete_ = false;
}

void Task::run() {
    Processor* processor = connection_->getProcessor();
    processor->proccess();
    connection_->setConnectionState(CONN_WAIT);
    connection_->notifyIOHanlder(); // Notify IOHandler that work is done
    notify(); // Notify Thread Manager that a thread is now idle
}

bool Task::notify() {
    Task* task = this;
    int fd = connection_->getThreadManager()->getWorkerNotificationSendFD();
    if (fd < 0) {
        return false;
    }

    fd_set wfds, efds;
    int ret = -1;
    int kSize = sizeof (task);
    const char* pos = (const char*) reinterpret_cast<void*> (&task);

    while (kSize > 0) {
        FD_ZERO(&wfds);
        FD_ZERO(&efds);
        FD_SET(fd, &wfds);
        FD_SET(fd, &efds);
        ret = select(fd + 1, NULL, &wfds, &efds, NULL);
        if (ret < 0) {
            return false;
        } else if (ret == 0) {
            continue;
        }
        if (FD_ISSET(fd, &efds)) {
            close(fd);
            return false;
        }
        if (FD_ISSET(fd, &wfds)) {
            ret = send(fd, pos, kSize, 0);
            if (ret < 0) {
                if (errno == EAGAIN) {
                    continue;
                }
                close(fd);
                return false;
            }
            kSize -= ret;
            pos += ret;
        }
    }

    return true;
}

int Task::getThreadID() {
    return threadID_;
}

void Task::setThreadID(int ID) {
    threadID_ = ID;
}

void Task::setObsolete() {
    std::lock_guard<std::mutex> guard(mutex_);
    isObsolete_ = true;
}

bool Task::getIsObsolete() {
    std::lock_guard<std::mutex> guard(mutex_);
    return isObsolete_;
}
