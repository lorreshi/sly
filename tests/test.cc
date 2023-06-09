#include <iostream>
#include "../sly/log.h"
#include "../sly/util.h"

int main(int argc, char** argv){
    sylar::Logger::ptr logger(new sylar::Logger);
    logger->addAppender(sylar::LogAppender::ptr(new sylar::StdoutLogAppender));

    //在txt中显示
    sylar::FileLogAppender::ptr file_appender(new sylar::FileLogAppender("./log.txt"));

    //自定义的
    sylar::LogFormatter::ptr fmt(new sylar::LogFormatter("%d"));
    file_appender->setFormatter(fmt);
    file_appender->setLevel(sylar::LogLevel::ERROR);
    logger->addAppender(file_appender);

//    sylar::LogEvent::ptr event(new sylar::LogEvent(__FILE__,
//                                                   __LINE__,
//                                                   0, sylar::GetThreadId(), sylar::GetFiberId(), time(0)));
//    event->getSS() << "hello sylar log";
//    logger->log(sylar::LogLevel::DEBUG, event);
//    std::cout << '\n';
    std::cout << "hello sly log11" << std::endl;

    SYLAR_LOG_INFO(logger) << "test macro INFO";
    SYLAR_LOG_ERROR(logger) << "test macro ERROR";

    //有问题运行不了这个
    //SYLAR_LOG_FMT_ERROR(logger, "test macro fmt error %s", "aa");

    auto l = sylar::LoggerMgr::GetInstance()->getLogger("xx");
    SYLAR_LOG_INFO(l) << "xxx";

    return 0;
}


