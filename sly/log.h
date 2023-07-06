#ifndef __SYLAR_LOG_H__   //防止头文件重复饮用
#define __SYLAR_LOG_H__

#include <string>
#include <stdint.h>
#include <memory>
#include <list>
#include <sstream>
#include <fstream>
#include <vector>
#include <stdarg.h>
#include <map>
#include "util.h"
#include "singleton.h"
#include "thread.h"




/**
 * @brief 使用流式方式将日志级别level的日志写入到logger
 */
#define SYLAR_LOG_LEVEL(logger, level) \
    if(logger->getLevel() <= level)    \
        sylar::LogEventWarp(sylar::LogEvent::ptr (new sylar::LogEvent(logger, level, \
                        __FILE__, __LINE__, 0, sylar::GetThreadId(), \
                        sylar::GetFiberId(), time(0), sylar::Thread::GetName()))).getSS()

/**
 * @brief 使用流式方式将日志级别debug的日志写入到logger
 */
#define SYLAR_LOG_DEBUG(logger)  SYLAR_LOG_LEVEL(logger, sylar::LogLevel::DEBUG)
#define SYLAR_LOG_INFO(logger)  SYLAR_LOG_LEVEL(logger, sylar::LogLevel::INFO)
#define SYLAR_LOG_WARN(logger)  SYLAR_LOG_LEVEL(logger, sylar::LogLevel::WARN)
#define SYLAR_LOG_ERROR(logger)  SYLAR_LOG_LEVEL(logger, sylar::LogLevel::ERROR)
#define SYLAR_LOG_FATAL(logger)  SYLAR_LOG_LEVEL(logger, sylar::LogLevel::FATAL)

/**
 * @brief 使用格式化方式将日志级别level的日志写入到logger
 */
#define SYLAR_LOG_FMT_LEVEL(logger, level, fmt, ...) \
    if(logger->getLevel() <= level)   \
        sylar::LogEventWrap(sylar::LogEvent::ptr(new sylar::LogEvent(logger, level, \
                        __FILE__, __LINE__, 0, sylar::GetThreadId(),\
                sylar::GetFiberId(), time(0), sylar::Thread::GetName()))).getEvent()->format(fmt, __VA_ARGS__)

/**
 * @brief 使用格式化方式将日志级别debug的日志写入到logger
 */
#define SYLAR_LOG_FMT_DEBUG(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::DEBUG, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_INFO(logger, fmt, ...)  SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::INFO, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_WARN(logger, fmt, ...)  SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::WARN, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_ERROR(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::ERROR, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_FATAL(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::FATAL, fmt, __VA_ARGS__)

/**
 * @brief 获取主日志器
 */
#define SYLAR_LOG_ROOT() sylar::LoggerMgr::GetInstance()->getRoot()
/**
 * @brief 获取name的日志器
 */
#define SYLAR_LOG_NAME(name) sylar::LoggerMgr::GetInstance()->getLogger(name)





//namespace 防止不同类文件名冲突
namespace sylar{
    /**
 * 类声明，宏定义用到了*/
    class LogEventWarp;
    class Logger;
    class LoggerManager;


    //日志级别由高到底
    class LogLevel {
    public:
        /**
         * 日志级别枚举
         */
        enum Level {
            /// 未知级别
            UNKNOW = 0,
            /// DEBUG 级别
            DEBUG = 1,
            /// INFO 级别
            INFO = 2,
            /// WARN 级别
            WARN = 3,
            /// ERROR 级别
            ERROR = 4,
            /// FATAL 级别
            FATAL = 5
        };
        /**
         * @brief 将日志级别转成文本输出
         * @param[in] level 日志级别
         */
        static const char* ToString(LogLevel::Level level);
        /**
        * @brief 将文本转换成日志级别
        * @param[in] str 日志级别文本
        */
        static LogLevel::Level FromString(const std::string& str);
    };

    //日志事件
    class LogEvent {
    public:
        typedef std::shared_ptr<LogEvent> ptr;
        LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char* file, int32_t line, uint32_t elapse
                , uint32_t thread_id, uint32_t fiber_id, uint64_t time, const std::string& thread_name);
        /**
        * @brief 返回文件名
        */
        const char* getFile() const { return m_file;}

        /**
         * @brief 返回行号
         */
        int32_t getLine() const { return m_line;}

        /**
         * @brief 返回耗时
         */
        uint32_t getElapse() const { return m_elapse;}

        /**
         * @brief 返回线程ID
         */
        uint32_t getThreadId() const { return m_threadId;}
        /**
        * @brief 返回线程名称
        */
        const std::string& getThreadName() const { return m_threadName;}
        /**
         * @brief 返回日志器
         */
        std::shared_ptr<Logger> getLogger() const { return m_logger;}

        /**
         * @brief 返回协程ID
         */
        uint32_t getFiberId() const { return m_fiberId;}

        /**
         * @brief 返回时间
         */
        uint64_t getTime() const { return m_time;}
        /**
        * @brief 返回日志内容
        */
        std::string getContent() const { return m_ss.str();}
        /**
         * @brief 返回日志级别
         */
        LogLevel::Level getLevel() const { return m_level;}

        /**
         * @brief 返回日志内容字符串流
         */
        std::stringstream& getSS() {return m_ss;}
        /**
         * @brief 格式化写入日志内容
         */
        void format(const char* fmt, ...);

        /**
         * @brief 格式化写入日志内容
         */
        void format(const char* fmt, va_list al);
    private:
        /// 文件名
        const char* m_file = nullptr;
        /// 行号
        int32_t m_line = 0;
        /// 程序启动开始到现在的毫秒数
        uint32_t m_elapse = 0;
        /// 线程ID
        uint32_t m_threadId = 0;
        /// 协程ID
        uint32_t m_fiberId = 0;
        /// 时间戳
        uint64_t m_time = 0;
        std::string m_content;
        /// 线程名称
        std::string m_threadName;
        /// 日志内容流
        std::stringstream m_ss;
        /// 日志器
        std::shared_ptr<Logger> m_logger;
        /// 日志等级
        LogLevel::Level m_level;

    };

    class LogEventWarp{
    public:
        LogEventWarp(LogEvent::ptr e);

        ~LogEventWarp();
        //获取日志事件
        LogEvent::ptr getEvent() const { return m_event;}
        //获取日志内容流
        std::stringstream& getSS();

    private:
        //日志事件
        LogEvent::ptr m_event;

    };



    //日志格式器
    class LogFormatter {
    public:
        typedef std::shared_ptr<LogFormatter> ptr;
        //返回格式化日志文本
        std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);
        std::string format(std::ostream& ofs, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);

        LogFormatter(const std::string& pattern);
        /**
         * @brief 构造函数
         * @param[in] pattern 格式模板
         * @details
         *  %m 消息
         *  %p 日志级别
         *  %r 累计毫秒数
         *  %c 日志名称
         *  %t 线程id
         *  %n 换行
         *  %d 时间
         *  %f 文件名
         *  %l 行号
         *  %T 制表符
         *  %F 协程id
         *  %N 线程名称
         *
         *  默认格式 "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
         */
    public:
        /**
         * @brief 日志内容项格式化
         */
        class FormatItem {
        public:
            typedef std::shared_ptr<FormatItem> ptr;
            /**
             * @brief 析构函数
             */
            virtual ~FormatItem() {}
            /**
             * @brief 格式化日志到流
             * @param[in, out] os 日志输出流
             * @param[in] logger 日志器
             * @param[in] level 日志等级
             * @param[in] event 日志事件
             */
            virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
        };

        /**
         * @brief 初始化,解析日志模板
         */
        void init();

        bool isError() const {return m_error;}

        const std::string getPattern() const {return m_pattern;}

    private:
        // 日志格式模板
        std::string m_pattern;
        // 日志格式解析后格式
        std::vector<FormatItem::ptr> m_items;
        /// 是否有错误
        bool m_error = false;
    };


    //日志输出地
    class LogAppender {
        friend class Logger;
    public:
        typedef std::shared_ptr<LogAppender> ptr;
        //自旋锁
        typedef Spinlock MutexType;
        //析构函数
        virtual ~LogAppender() {}
        /**
        * @brief 写入日志
        * @param[in] logger 日志器
        * @param[in] level 日志级别
        * @param[in] event 日志事件
        */
        virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;

        virtual std::string toYamlString() = 0;

        //设置日志格式函数
        void setFormatter(LogFormatter::ptr val);
        //获取日志格式
        LogFormatter::ptr getFormatter();
        /**
        * @brief 获取日志级别
        */
        LogLevel::Level getLevel() const { return m_level;}

        /**
         * @brief 设置日志级别
         */
        void setLevel(LogLevel::Level val) { m_level = val;}



    protected:
        /// 日志级别
        LogLevel::Level m_level = LogLevel::DEBUG;
        bool m_hasFormatter = false;
        MutexType m_mutex;
        //定义输出的格式,需要设置一个LogFormmatter类
        LogFormatter::ptr m_formatter;

    };

    ///日志器
    class Logger:public std::enable_shared_from_this<Logger>  {
        friend class LoggerManager;
    public:
        typedef std::shared_ptr<Logger> ptr;
        typedef Spinlock MutexType;
        //日志名称初始为“root”
        Logger(const std::string& name = "root");
        // 写日志函数 日直级别，日志事件
        void log(LogLevel::Level level, LogEvent::ptr event);

        void debug(LogEvent::ptr event);

        /**
         * @brief 写info级别日志
         * @param[in] event 日志事件
         */
        void info(LogEvent::ptr event);

        /**
         * @brief 写warn级别日志
         * @param[in] event 日志事件
         */
        void warn(LogEvent::ptr event);

        /**
         * @brief 写error级别日志
         * @param[in] event 日志事件
         */
        void error(LogEvent::ptr event);

        /**
         * @brief 写fatal级别日志
         * @param[in] event 日志事件
         */
        void fatal(LogEvent::ptr event);
        /**
         * @brief 添加日志目标
         * @param[in] appender 日志目标
         */
        void addAppender(LogAppender::ptr appender);

        /**
         * @brief 删除日志目标
         * @param[in] appender 日志目标
         */
        void delAppender(LogAppender::ptr appender);

        /**
         * @brief 清空日志目标
         */
        void clearAppenders();
        /**
         * @brief 返回日志级别
         */
        LogLevel::Level getLevel() const { return m_level;}

        /**
         * @brief 设置日志级别
         */
        void setLevel(LogLevel::Level val) { m_level = val;}
        /**
         * @brief 返回日志名称
         */
        const std::string& getName() const { return m_name;}
        /**
        * @brief 设置日志格式器
        */
        void setFormatter(LogFormatter::ptr val);
        /**
         * @brief 设置日志格式模板
         */
        void setFormatter(const std::string& val);
        /**
        * @brief 获取日志格式器
        */
        LogFormatter::ptr getFormatter();

        std::string toYamlString();

    private:
        /// 日志名称
        std::string m_name;
        /// 日志级别
        LogLevel::Level m_level;
        /// 锁
        MutexType m_mutex;
        /// 日志目标集合
        std::list<LogAppender::ptr> m_appenders;
        /// 日志格式器
        LogFormatter::ptr m_formatter;
        /// 主日志器
        Logger::ptr m_root;

    };

    //输出到控制台的Appender
    class StdoutLogAppender : public LogAppender {
        friend class Logger;
    public:
        typedef std::shared_ptr<StdoutLogAppender> ptr;
        virtual void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
        std::string toYamlString() override;
    };

    //输出到文件的Appender
    class FileLogAppender : public LogAppender {
        friend class Logger;
    public:
        typedef std::shared_ptr<FileLogAppender> ptr;
        FileLogAppender(const std::string& filename);
        virtual void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
        std::string toYamlString() override;
        /**
         * @brief 重新打开日志文件
         * @return 成功返回true
         */
        bool reopen();
    private:
        /// 文件路径
        std::string m_filename;
        /// 文件流
        std::ofstream m_filestream;
        /// 上次重新打开时间
        uint64_t m_lastTime = 0;
    };

    ///日志管理其
    class LoggerManager{
    public:
        typedef Spinlock MutexType;
        LoggerManager();
        Logger::ptr getLogger(const std::string& name);

        void init();
        Logger::ptr getRoot() const {return m_root;}

        std::string toYamlString();
    private:
        MutexType m_mutex;
        std::map<std::string, Logger::ptr> m_loggers;
        Logger::ptr m_root;
    };

    // 日志器管理类单例模式
    typedef sylar::Singleton<LoggerManager> LoggerMgr;



}

#endif