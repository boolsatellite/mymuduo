//
// Created by satellite on 2023-12-02.
//
#include "Callbacks.h"
#include "Buffer.h"
void defaultConnectionCallback(const TcpConnectionPtr& conn) {}
void defaultMessageCallback(const TcpConnectionPtr& conn,
                            Buffer* buffer,
                            Timestamp receiveTime) {

    buffer->retrieveAll();
}
