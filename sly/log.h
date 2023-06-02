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

//namespace 防止不同类文件名冲突
namespace sylar{
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
    };

    //日志事件
    class LogEvent {
    public:
        typedef std::shared_ptr<LogEvent> ptr;
        LogEvent(const char* file, int32_t line, uint32_t elapse
                , uint32_t thread_id, uint32_t fiber_id, uint64_t time);

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
        const std::string& getContent() const {return m_content;}
        std::stringstream& getSS() {return m_ss;}
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

    //日志格式器
    class LogFormatter {
    public:
        typedef std::shared_ptr<LogFormatter> ptr;
        //返回格式化日志文本
        std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);

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
    public:
        typedef std::shared_ptr<LogAppender> ptr;
        //析构函数
        virtual ~LogAppender() {}
        /**
        * @brief 写入日志
        * @param[in] logger 日志器
        * @param[in] level 日志级别
        * @param[in] event 日志事件
        */
        virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
        //设置日志格式函数
        void setFormatter(LogFormatter::ptr val){m_formatter = val;}
        //获取日志格式
        LogFormatter::ptr getFormatter() const {return m_formatter;}

    protected:
        /// 日志级别
        LogLevel::Level m_level;
        //定义输出的格式,需要设置一个LogFormmatter类
        LogFormatter::ptr m_formatter;

    };
    //日志器
    class Logger:public std::enable_shared_from_this<Logger>  {
    public:
        typedef std::shared_ptr<Logger> ptr;
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

    private:
        /// 日志名称
        std::string m_name;
        /// 日志级别
        LogLevel::Level m_level;
        /// 日志目标集合
        std::list<LogAppender::ptr> m_appenders;
        /// 日志格式器
        LogFormatter::ptr m_formatter;
        /// 主日志器
        Logger::ptr m_root;

    };

    //输出到控制台的Appender
    class StdoutLogAppender : public LogAppender {
    public:
        typedef std::shared_ptr<StdoutLogAppender> ptr;
        virtual void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
    };

    //输出到文件的Appender
    class FileLogAppender : public LogAppender {
    public:
        typedef std::shared_ptr<FileLogAppender> ptr;
        FileLogAppender(const std::string& filename);
        virtual void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
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



}

#endif