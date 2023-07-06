#include <iostream>
#include "../sly/log.h"
#include "../sly/util.h"


void test01(){
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


    auto l = sylar::LoggerMgr::GetInstance()->getLogger("xsx");
    SYLAR_LOG_INFO(l) << "sss";
}

void test_logger(){
    //创建日志
    sylar::Logger::ptr logger(new sylar::Logger("sly"));
    //添加日志，删除日志，清空日志
    logger->addAppender(sylar::LogAppender::ptr(new sylar::StdoutLogAppender));
    logger->clearAppenders();
    // 设置日志等级
    logger->setLevel(sylar::LogLevel::ERROR);
    std::cout << "日志名称:  " << logger-> getName() << '\n'
              << "日志等级:  " << logger->getLevel() << '\n';
    //自定义日志格式： 1.获取日志格式 2.设置日志格式  3.在txt显示
    sylar::LogFormatter::ptr fmt(new sylar::LogFormatter("%d%n"));
    logger->setFormatter(fmt);
    sylar::FileLogAppender::ptr file_appender(new sylar::FileLogAppender("./log1.txt"));
    logger->addAppender(file_appender);
    // 数据流式输出
    sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();
    SYLAR_LOG_INFO(g_logger) << "test macro INFO";
    SYLAR_LOG_ERROR(g_logger) << "test macro ERROR";

}


int main(int argc, char** argv){

    test_logger();

    return 0;
}





