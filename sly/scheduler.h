//
// Created by sly on 6/21/23. 协程调度模块
//

#ifndef SLY_SCHEDULER_H
#define SLY_SCHEDULER_H

#include <memory>
#include <vector>
#include <list>
#include "fiber.h"
#include "thread.h"

namespace sylar{

    /**
    * @brief 协程调度器
    * @details 封装的是N-M的协程调度器
    *          内部有一个线程池,支持协程在线程池里面切换
    */
    class Scheduler{
    public:
        typedef std::shared_ptr<Scheduler> ptr;
        typedef Mutex MutexType;

        /**
         *
         * @param thread 协程的默认线程数字设置成为 1
         * @param use_caller 是否使用的是当前调度线程
         * @param name 协程调度器的年名称
         */
        Scheduler(size_t thread = 1, bool use_caller = true, const std::string& name = "");

        /**
         * @brief 析构函数 虚西沟函数 子类重写
         */
        virtual ~Scheduler();

        /**
         * 获取协程名称
         * @return
         */
        const std::string& getName() const {
            return m_name;
        }

        /**
         * 返回当前协程调度器
         * @return
         */
        static Scheduler* GetThis();

        /**
        * @brief 返回当前协程调度器的调度协程
        */
        static Fiber* GetMainFiber();

        /**
        * @brief 启动协程调度器
        */
        void start();

        /**
         * @brief 停止协程调度器
         */
        void stop();

        /**
         * @brief 调度协程
         * @param[in] fc 协程或函数
         * @param[in] thread 协程执行的线程id,-1标识任意线程
         * schedule函数的作用是将协程或回调函数添加到容器中，并在需要时执行调度唤醒操作
         */
        template<class FiberOrCb>
        void schedule(FiberOrCb fc, int thread = -1) {
            bool need_tickle = false;
            //{}没有特定的条件判断含义，只是用于构造一个局部作用域，限定互斥锁的作用范围，确保在解锁之前互斥锁一直持有。
            {
                MutexType::Lock lock(m_mutex);
                need_tickle = scheduleNoLock(fc, thread);
            }

            if(need_tickle) {
                //如果为true 唤醒
                tickle();
            }
        }

        /**
         * @brief 批量调度协程
         * @param[in] begin 协程数组的开始
         * @param[in] end 协程数组的结束
         * 这个函数的作用是将一系列任务添加到协程调度器的任务队列中，
         * 并在需要时执行调度唤醒操作。使用互斥锁确保对任务队列的访问是线程安全的。
         */
        template<class InputIterator>
        void schedule(InputIterator begin, InputIterator end){
            bool need_tickle = false;
            {
                MutexType::Lock lock(m_mutex);
                while (begin != end){
                    //begin是一个迭代器，表示任务范围的起始位置。*begin表示通过迭代器取得当前位置的元素值。
                    //而&*begin则表示对该元素值进行取地址操作，即获取指向该元素的指针。
                    need_tickle = scheduleNoLock(&*begin,-1)||need_tickle;
                    ++begin;
                }
            }
            if(need_tickle){
                tickle();
            }
        }

    protected:
        /**
         * @brief 通知协程调度器有任务了
         */
        virtual void tickle();
        /**
         * @brief 协程调度函数
         */
        void run();
        /**
         * @brief 返回是否可以停止
         */
        virtual bool stopping();
        /**
         * @brief 协程无任务可调度时执行idle协程
         */
        virtual void idle();
        /**
         * @brief 设置当前的协程调度器
         */
        void setThis();
        /**闲置线程数
         *
         * @return true有空闲线程
         */
        bool hasIdleThreads(){ return m_idleThreadCount > 0;}

    private:
        /**
         * @brief 协程调度启动(无锁)
         * 这段代码的主要作用是将协程或回调函数添加到一个容器中，以便在后续的调度过程中执行。
         * 如果容器之前为空，则意味着当前没有待调度的协程，需要进行调度唤醒操作。
         * 返回的need_tickle值可以用于判断是否需要进行调度唤醒操作
         */
        template<class FiberOrCb>
        bool scheduleNoLock(FiberOrCb fc, int thread) {
            bool need_tickle = m_fibers.empty();
            FiberAndThread ft(fc, thread);
            if(ft.fiber || ft.cb) {
                m_fibers.push_back(ft);
            }
            return need_tickle;
        }

    private:

        /**
         * @brief 协程/函数/线程组
         * 对于协程调度器来说，协程可以是调度任务，但实际上，函数也可以是，
         * 只需要把函数包装成协程即可。因此这里设计了一个调度任务
         */
        struct FiberAndThread {
            /// 协程
            Fiber::ptr fiber;
            /// 协程执行函数
            std::function<void()> cb;
            /// 线程id
            int thread;

            /**
             * @brief 构造函数
             * @param[in] f 协程
             * @param[in] thr 线程id
             */
            FiberAndThread(Fiber::ptr f, int thr)
                    : fiber(f), thread(thr) {
            }

            /**
             * @brief 构造函数
             * @param[in] f 协程指针
             * @param[in] thr 线程id
             * @post *f = nullptr
             */
            FiberAndThread(Fiber::ptr *f, int thr)
                    : thread(thr) {
                fiber.swap(*f);
            }

            /**
             * @brief 构造函数
             * @param[in] f 协程执行函数
             * @param[in] thr 线程id
             */
            FiberAndThread(std::function<void()> f, int thr)
                    : cb(f), thread(thr) {
            }

            /**
             * @brief 构造函数
             * @param[in] f 协程执行函数指针
             * @param[in] thr 线程id
             * @post *f = nullptr
             */
            FiberAndThread(std::function<void()> *f, int thr)
                    : thread(thr) {
                cb.swap(*f);
            }

            /**
             * @brief 无参构造函数 -1不指定任何线程，也可以是说任意线程
             */
            FiberAndThread()
                    : thread(-1) {
            }

            /**
            * @brief 重置数据
            */
            void reset() {
                fiber = nullptr;
                cb = nullptr;
                thread = -1;
            }
        };

    private:
        /// Mutex
        MutexType m_mutex;
        /// 线程池
        std::vector<Thread::ptr> m_threads;
        /// 待执行的协程队列
        std::list<FiberAndThread> m_fibers;
        /// 协程名称
        std::string m_name;
        /// use_caller为true时有效, 调度协程
        Fiber::ptr m_rootFiber;

    protected:
        /// 协程下的线程id数组
        std::vector<int> m_threadIds;
        /// 线程数量
        size_t m_threadCount = 0;
        /// 工作线程数量
        std::atomic<size_t> m_activeThreadCount = {0};
        /// 空闲线程数量
        std::atomic<size_t> m_idleThreadCount = {0};
        /// 是否正在停止
        bool m_stopping = true;
        /// 是否自动停止
        bool m_autoStop = false;
        /// 主线程id(use_caller)
        int m_rootThread = 0;
    };

}

#endif //SLY_SCHEDULER_H
