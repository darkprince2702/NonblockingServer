/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "NonblockingServer.h"

ThreadManager::ThreadManager(int workerNum) {
    workEvent_ = NULL;
    taskEvent_ = NULL;
    eventBase_ = NULL;
    threads_ = new Poco::Thread[workerNum];
    for (int i = 0; i < workerNum; i++) {
        idleThreadIDs_.push_back(i);
    }
}

void ThreadManager::registerEvents() {
    eventBase_ = event_base_new();

    createNotificationPipes();

    workEvent_ = event_new(eventBase_, getWorkerNotificationRecvFD(),
            EV_READ | EV_PERSIST, ThreadManager::workerCallback, this);
    if (event_add(workEvent_, 0) == -1) {
        std::cout << "ThreadManager::registerEvents() error\n";
    }
    taskEvent_ = event_new(eventBase_, getTaskNotificationRecvFD(),
            EV_READ | EV_PERSIST, ThreadManager::taskCallback, this);
    if (event_add(taskEvent_, 0) == -1) {
        std::cout << "ThreadManager::registerEvents() error\n";
    }
}

void ThreadManager::run() {
    if (eventBase_ == NULL) {
        registerEvents();
    }

    event_base_loop(eventBase_, 0);
}

void ThreadManager::createNotificationPipes() {
    // workerPipe
    if (evutil_socketpair(AF_LOCAL, SOCK_STREAM, 0, workerNotificationPipeFDs_) == -1) {
        std::cout << "createNotificationPipes()\n";
    }
    if (evutil_make_socket_nonblocking(workerNotificationPipeFDs_[0]) < 0
            || evutil_make_socket_nonblocking(workerNotificationPipeFDs_[1]) < 0) {
        close(workerNotificationPipeFDs_[0]);
        close(workerNotificationPipeFDs_[1]);
        std::cout << "createNotificationPipes()\n";
    }
    // taksPipe
    if (evutil_socketpair(AF_LOCAL, SOCK_STREAM, 0, taskNotificationPipeFDs_) == -1) {
        std::cout << "createNotificationPipes()\n";
    }
    if (evutil_make_socket_nonblocking(taskNotificationPipeFDs_[0]) < 0
            || evutil_make_socket_nonblocking(taskNotificationPipeFDs_[1]) < 0) {
        close(taskNotificationPipeFDs_[0]);
        close(taskNotificationPipeFDs_[1]);
        std::cout << "createNotificationPipes()\n";
    }
    for (int i = 0; i < 2; ++i) {
        if (evutil_make_socket_closeonexec(workerNotificationPipeFDs_[i]) < 0) {
            close(workerNotificationPipeFDs_[0]);
            close(workerNotificationPipeFDs_[1]);
            std::cout << "createNotificationPipes()\n";
        }
        if (evutil_make_socket_closeonexec(taskNotificationPipeFDs_[i]) < 0) {
            close(taskNotificationPipeFDs_[0]);
            close(taskNotificationPipeFDs_[1]);
            std::cout << "createNotificationPipes()\n";
        }
    }
}

int ThreadManager::getTaskNotificationRecvFD() {
    return taskNotificationPipeFDs_[0];
}

int ThreadManager::getTaskNotificationSendFD() {
    return taskNotificationPipeFDs_[1];
}

int ThreadManager::getWorkerNotificationRecvFD() {
    return workerNotificationPipeFDs_[0];
}

int ThreadManager::getWorkerNotificationSendFD() {
    return workerNotificationPipeFDs_[1];
}

void ThreadManager::taskCallback(int fd, short what, void* v) {
    ThreadManager* threadManger = (ThreadManager*) v;
    while (true) {
        Task* task = 0;
        const int size = sizeof (task);
        long bytes = recv(fd, reinterpret_cast<void*> (&task), size, 0);
        if (bytes == size) { // Received 
            if (task == NULL) {
                return;
            } else {
                std::cout << "Task queue size: " << threadManger->taskQueue_.size() << std::endl;
                threadManger->taskQueue_.push(task);
                threadManger->eventHanlder();
            }
        } else if (bytes == 0) { // Caller finish send
            break;
        } else { // Error
            return;
        }
        break;
    }
}

void ThreadManager::workerCallback(int fd, short what, void* v) {
    ThreadManager* threadManger = (ThreadManager*) v;
    while (true) {
        Task* task = 0;
        const int size = sizeof (task);
        long bytes = recv(fd, reinterpret_cast<void*> (&task), size, 0);
        if (bytes == size) { // Received 
            if (task == NULL) {
                return;
            } else {
                Poco::Thread* thread = threadManger->getThread(task->getThreadID());
                thread->join();
                threadManger->addIdleThread(task->getThreadID());
                threadManger->eventHanlder();
            }
        } else if (bytes == 0) { // Caller finish send
            break;
        } else { // Error
            return;
        }
        break;
    }
}

void ThreadManager::eventHanlder() {
    if (taskQueue_.empty() || idleThreadIDs_.empty()) {
        return;
    } else {
        Task* task;
        // Pop until get a non-obsolete task
        do {
            task = taskQueue_.front();
            taskQueue_.pop();
            if (!task->getIsObsolete()) {
                int threadID = idleThreadIDs_.back();
                idleThreadIDs_.pop_back();
                task->setThreadID(threadID);
                threads_[threadID].start(*task);
                break;
            }
        } while (!taskQueue_.empty());
        return;
    }
}

void ThreadManager::addIdleThread(int threadID) {
    idleThreadIDs_.push_back(threadID);
}

Poco::Thread* ThreadManager::getThread(int threadID) {
    return &threads_[threadID];
}
