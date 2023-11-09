//
// Created by satellite on 2023-11-09.
//

//定义日志级别 INFO ERROR FATAL DEBUG

#include <iostream>
#include "Logger.h"
#include "Timestamp.h"

Logger &Logger::instence() {
    static Logger logger;
    return logger;
}

void Logger::setLoggerLevel(int level) {
    loglevel_ = level;
}

void Logger::log(std::string msg) {
    switch (loglevel_) {
        case INFO:
            std::cout << "[INFO]" ;
            break;
        case ERROR:
            std::cout << "[ERROR]" ;
            break;
        case FATAL:
            std::cout << "[FATAL]" ;
            break;
        case DEBUG:
            std::cout << "[DEBUG]" ;
            break;
        default:
            break;
    }
    std::cout << Timestamp::now().toString() << msg << std::endl;
}

