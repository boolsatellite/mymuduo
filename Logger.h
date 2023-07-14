#pragma once 

#include "nocopyable.h"
#include <string>
#include <stdio.h>

#define LOG_INFO(LogmsgFormat , ...)\
    do\
    {\
        Logger& logger = Logger::getInstance();\
        logger.setLogLevel(INFO);\
        char buf[1024];\
        sprintf(buf,LogmsgFormat,##__VA_ARGS__);\
        logger.log(buf);\
    } while (0);
    
#define LOG_ERROR(LogmsgFormat , ...)\
    do\
    {\
        Logger& logger = Logger::getInstance();\
        logger.setLogLevel(ERROR);\
        char buf[1024];\
        sprintf(buf,LogmsgFormat,##__VA_ARGS__);\
        logger.log(buf);\
    } while (0);


#define LOG_FATAL(LogmsgFormat , ...)\
    do\
    {\
        Logger& logger = Logger::getInstance();\
        logger.setLogLevel(FATAL);\
        char buf[1024];\
        sprintf(buf,LogmsgFormat,##__VA_ARGS__);\
        logger.log(buf);\
    } while (0);

#ifdef MUDEBUG
#define LOG_DEBUG(LogmsgFormat , ...)\
    do\
    {\
        Logger& logger = Logger::getInstance();\
        logger.setLogLevel(DEBUG);\
        char buf[1024];\
        sprintf(buf,LogmsgFormat,##__VA_ARGS__);\
        logger.log(buf);\
    } while (0);
#else
#define LOG_DEBUG(LogmsgFormat , ...)
#endif

enum LogLevel
{
    INFO,   //普通信息
    ERROR,  //错误信息
    FATAL,  //coredump
    DEBUG   //调试信息
};

//日志类,采用单例模式

class Logger : nocopyable
{
public:
    static Logger& getInstance();   //获取单例
    void setLogLevel(int level);    //设置日志级别
    void log(std::string msg);      

private:

    int logLevel_;   //日志级别
    Logger();
};


