//
// Created by sly on 6/21/23.
//
#include "scheduler.h"
#include "log.h"
#include "macro.h"
#include "hook.h"

namespace sylar{

    static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

    //当前协程调度指针
    static thread_local Scheduler* t_scheduler = nullptr;
    //主协程指针
    static thread_local Fiber* t_scheduler_fiber = nullptr;

    //实现在创建协程调度器时根据需求将创建协程调度器的线程放入协程调度器的线程池中，
    // 或者不放入并使用独立的主协程（即协程调度器线程）。
    Scheduler::Scheduler(size_t threads, bool use_caller, const std::string& name)
    :m_name(name){
        SYLAR_ASSERT(threads > 0);
//      这个地方的关键点在于，是否把创建协程调度器的线程放到协程调度器管理的线程池中。
//      如果不放入，那这个线程专职协程调度；如果放的话，那就要把协程调度器封装到一个协程中，
//      称之为主协程或协程调度器协程。
        if(use_caller){
            // getthis函数，如果没有协程，会初始化一个主协程
            sylar::Fiber::GetThis();
            //线程数量-1，因为主协程将会占用一个线程
            --threads;
            //如果此处发生断言说明一个线程中出现了两个协程调度 是错误的
            SYLAR_ASSERT(GetThis() == nullptr);
            //将当前协程的指针给全局变量，以便在其他地方可以访问次协程
            t_scheduler = this;
            //创建一个根协程（Fiber对象），该协程的执行函数是Scheduler::run，通过std::bind将this绑定为参数
            //这样，当根协程执行时，run()函数将以Scheduler对象的上下文运行。
            m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this),0, true));
            sylar::Thread::SetName(m_name);
            //get方法，获得当前的根指针，给t_fiber。表示当前的为根协程
            t_scheduler_fiber = m_rootFiber.get();
            //将当前线程的线程ID存储到协程调度器的线程ID列表中。
            m_rootThread = sylar::GetThreadId();
            m_threadIds.push_back(m_rootThread);

        }else{
            m_rootThread = -1;
        }
        m_threadCount = threads;
    }

    Scheduler::~Scheduler() {
        //协程调度停止，满足xigou条件
        SYLAR_ASSERT(m_stopping);
        //如果当前正在执行的协程（通过GetThis()获取）是当前协程调度器的对象 置空
        //这样可以避免在调用已销毁的协程调度器对象时出现悬空指针或错误的访问。
        if(GetThis() == this){
            t_scheduler = nullptr;
        }
    }

    Scheduler* Scheduler::GetThis() {
        return  t_scheduler;
    }

    Fiber* Scheduler::GetMainFiber() {
        return t_scheduler_fiber;
    }

    void Scheduler::start() {
        MutexType::Lock lock(m_mutex);
        if(!m_stopping){
            return;
        }
        //启动
        m_stopping = false;
        //线程池不为空
        SYLAR_ASSERT(m_threads.empty());
        //更改vector 大小
        m_threads.resize(m_threadCount);
        for(size_t i=0; i<m_threadCount; ++i){
            //reset 函数用于释放当前指针所拥有的资源，并将指针重置为空指针或指向新的对象。
            m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this),
                                          m_name + " " + std::to_string(i)));

            m_threadIds.push_back(m_threads[i]->getId());
        }
        //解锁
        lock.unlock();

//        if(m_rootFiber){
//            //m_rootFiber->swapIn();
//            m_rootFiber->call();
//            SYLAR_LOG_INFO(g_logger) << "call out " << m_rootFiber->getState();
//        }


    }

    void Scheduler::stop() {
        m_autoStop = true;
        if(m_rootFiber
           && m_threadCount == 0
           && (m_rootFiber->getState() == Fiber::TERM
               || m_rootFiber->getState() == Fiber::INIT)) {
            SYLAR_LOG_INFO(g_logger) << this << " stopped ";
            m_stopping = true;

            if(stopping()){
                return;
            }
        }

        if(m_rootThread != -1){
            SYLAR_ASSERT(GetThis() == this);
        }else{
            SYLAR_ASSERT(GetThis() != this);
        }
        m_stopping = true;
        for(size_t i=0; i< m_threadCount; ++i){
            tickle();
        }
        if(m_rootFiber){
            tickle();
        }
        //这里逻辑好绕 大意就是stop里面即做了mainFunc的启动，等mainFunc结束后stop。如果mainFunc放到start里结束，则mainFunc结束后start才会结束。
        if(m_rootFiber) {
//            while(!stopping()){
//                if(m_rootFiber->getState() == Fiber::TERM || m_rootFiber->getState() == Fiber::EXCEPT){
//                    m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));
//                    SYLAR_LOG_INFO(g_logger) << "root fiber is term, reset";
//                    //
//                    t_fiber = m_rootFiber.get();
//                }
//                m_rootFiber->call();
//            }
            if(!stopping()){
                m_rootFiber->call();
            }
        }

        std::vector<Thread::ptr> thrs;{
            MutexType::Lock lock(m_mutex);
            thrs.swap(m_threads);
        }

        for(auto& i : thrs){
            i->join();
        }



    }

    void Scheduler::setThis() {
        t_scheduler = this;
    }

    void Scheduler::run(){
        SYLAR_LOG_INFO(g_logger) << " run ";
        set_hook_enable(true);
        setThis();
        //当前线程id不等于主线程id,则将全局变量 t_fiber 设置为当前协程（Fiber::GetThis().get()）。
        if(sylar::GetThreadId() != m_rootThread){
            t_scheduler_fiber  = Fiber::GetThis().get();
        }

        Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));
        //用于回调函数的协程
        Fiber::ptr cb_fiber;

        FiberAndThread ft;
        while(true){
            ft.reset();
            bool tickle_me = false;
            {

                MutexType::Lock lock(m_mutex);
                auto it = m_fibers.begin();
                while(it != m_fibers.end()){
                    //便利协程队列，如果协程不等于-1 或者当前的协程id的话 不处理
                    if(it->thread != -1 && it->thread != sylar::GetThreadId()){
                        ++it;
                        //发出一个tickle信号
                        tickle_me = true;
                        continue;
                    }

                    SYLAR_ASSERT(it->fiber || it->cb);
                    // 正在执行状态，不需要处理
                    if(it->fiber && it->fiber->getState() == Fiber::EXEC){
                        ++it;
                        continue;
                    }

                    //处理请求 将协程对象赋值给 ft，并从协程队列中删除该协程
                    ft = *it;
                    m_fibers.erase(it);
                    break;
                }
                tickle_me |= it != m_fibers.end();
            }
            if(tickle_me){
                tickle();
            }
            //如果 ft 中的协程对象存在且未终止
            if(ft.fiber && (ft.fiber->getState() != Fiber::TERM
                            && ft.fiber->getState() != Fiber::EXCEPT)){
                ++m_activeThreadCount;
                //将协程对象切入运行。
                ft.fiber->swapIn();
                --m_activeThreadCount;
                //如果协程对象的状态为 Fiber::READY，则将其重新加入到调度队列中。
                //如果协程对象的状态既不是 Fiber::TERM 也不是 Fiber::EXCEPT，
                // 则将其状态设置为 Fiber::HOLD。
                if(ft.fiber->getState() == Fiber::READY){
                    schedule(ft.fiber);
                }else if(ft.fiber->getState() != Fiber::TERM
                         && ft.fiber->getState() != Fiber::EXCEPT){
                    ft.fiber->m_state = Fiber::HOLD;
                }
                ft.reset();
            }else if(ft.cb){
                if(cb_fiber){
                    //如果 cb_fiber 已存在，则重置其状态和执行函数为 ft.cb。
                    cb_fiber->reset(ft.cb);
                }else{
                    //如果 cb_fiber 不存在，则创建一个新的协程对象并将执行函数设置为 ft.cb。
                    cb_fiber.reset(new Fiber(ft.cb));
                }
                ++m_activeThreadCount;
                //重置 ft 对象。
                ft.reset();
                --m_activeThreadCount;
                //切换回调函数
                cb_fiber->swapIn();
                //如果回调协程的状态为 Fiber::READY，表示回调协程还需要继续执行，
                // 将其重新加入到调度队列中，并将 cb_fiber 重置为空。
                if(cb_fiber->getState() == Fiber::READY){
                    schedule(cb_fiber);
                    cb_fiber.reset();
                    //如果回调协程的状态为 Fiber::EXCEPT 或 Fiber::TERM，
                    // 表示回调协程已经执行完毕或出现异常，将其重置为 nullptr。
                }else if(cb_fiber->getState() == Fiber::EXCEPT
                         || cb_fiber->getState() == Fiber::TERM){
                    cb_fiber->reset(nullptr);
                    //如果回调协程的状态既不是 Fiber::READY 也不是 Fiber::EXCEPT
                    // 或 Fiber::TERM，则将其状态设置为 Fiber::HOLD，并将 cb_fiber
                    // 重置为 nullptr
                }else {
                    cb_fiber->m_state = Fiber::HOLD;
                    cb_fiber.reset();
                }
            //如果 ft 中的协程对象和回调函数都不存在，表示没有待调度的协程和回调函数：
            }else {

                //如果空闲协程 idle_fiber 的状态为 Fiber::TERM，
                // 表示空闲协程已经执行完毕，打印日志并跳出循环。
                if(idle_fiber->getState() == Fiber::TERM) {
                    SYLAR_LOG_INFO(g_logger) << "idle fiber term";
                    break;
                }
                //增加空闲线程计数。
                ++m_idleThreadCount;
                //将当前协程切换到空闲协程 idle_fiber 中执行。
                idle_fiber->swapIn();
                //减少空闲线程计数。
                --m_idleThreadCount;
                //如果空闲协程的状态既不是 Fiber::TERM 也不是 Fiber::EXCEPT，则将其状态设置为 Fiber::HOLD。
                if((idle_fiber->getState() != Fiber::TERM) && (idle_fiber->getState() != Fiber::EXCEPT)) {
                    idle_fiber->m_state = Fiber::HOLD;
                }
            }
        }



    }

    void Scheduler::tickle() {
        SYLAR_LOG_INFO(g_logger) << "tickle";
    }

    bool Scheduler::stopping() {
        MutexType::Lock lock(m_mutex);
        return m_autoStop && m_stopping
               && m_fibers.empty() && m_activeThreadCount == 0;
    }

    void Scheduler::idle() {
        SYLAR_LOG_INFO(g_logger) << "idle";
        while(!stopping()){
            sylar::Fiber::YieldToHold();
        }
    }

}


