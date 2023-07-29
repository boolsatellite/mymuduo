#pragma once

#include <functional>
#include <memory>


class Buffer;
class Tcpconnection;
class TimeStamp;


using TcpConnectionPtr = std::shared_ptr<Tcpconnection>;

using ConnectionCallback = std::function<void (const TcpConnectionPtr&)>;

using CloseCallback = std::function<void(const TcpConnectionPtr&)>;

using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;

using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr& , size_t)>;

using MessageCallback = std::function<void(const TcpConnectionPtr& , Buffer* , TimeStamp)>;
