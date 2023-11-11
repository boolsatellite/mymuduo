//
// Created by satellite on 2023-11-09.
//

#ifndef MYMUDUO_LOGGER_H
#define MYMUDUO_LOGGER_H


#include "stdio.h"
#include "nocopyable.h"
#include "string"

#define debugline __FILE__ , __FUNCTION__ ,__LINE__

#define LOG_INFO(LogmsgFormat , ...) \
    do {\
        Logger& logger = Logger::instence(); \
        logger.setLoggerLevel(INFO); \
        char buf[1024];              \
        snprintf(buf , 1024 , LogmsgFormat , ##__VA_ARGS__); \
        logger.log(buf); \
    }while(0);

#define LOG_ERROR(LogmsgFormat , ...) \
    do {\
        Logger& logger = Logger::instence(); \
        logger.setLoggerLevel(ERROR); \
        char buf[1024];              \
        snprintf(buf , 1024 , LogmsgFormat , ##__VA_ARGS__); \
        logger.log(buf); \
    }while(0);

#define LOG_FATAL(LogmsgFormat , ...) \
    do {\
        Logger& logger = Logger::instence(); \
        logger.setLoggerLevel(FATAL); \
        char buf[1024];              \
        snprintf(buf , 1024 , LogmsgFormat , ##__VA_ARGS__); \
        logger.log(buf); \
    }while(0);

#ifdef MUDUO_DEBUG
#define LOG_DEBUG(LogmsgFormat , ...) \
    do {\
        Logger& logger = Logger::instence(); \
        logger.setLoggerLevel(DEBUG); \
        char buf[1024];              \
        snprintf(buf , 1024 , logmsgFormat , ##__VA_ARGS__); \
        logger.log(buf); \
    }while(0);
#else
#define LOG_DEBUG(LogmsgFormat , ...)
#endif


enum Loglevel {
    INFO ,
    ERROR ,
    FATAL  ,
    DEBUG
};


class Logger : nocopyable{
public:
    static Logger& instence();
    void setLoggerLevel(int level);
    void log(std::string msg);
private:
    int loglevel_;
};


#endif //MYMUDUO_LOGGER_H
