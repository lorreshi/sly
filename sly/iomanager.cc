//
// Created by sly on 6/26/23.
//
#include "iomanager.h"
#include "macro.h"
#include "log.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <string.h>
#include <unistd.h>

namespace sylar{

    static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

    IOManager::FdContext::EventContext& IOManager::FdContext::getContext(IOManager::Event event) {
        switch(event) {
            case IOManager::READ:
                return read;
            case IOManager::WRITE:
                return write;
            default:
                SYLAR_ASSERT2(false, "getContext");
        }
    }

    void IOManager::FdContext::resetContext(EventContext& ctx) {
        ctx.scheduler = nullptr;
        ctx.fiber.reset();
        ctx.cb = nullptr;
    }

    void IOManager::FdContext::triggerEvent(IOManager::Event event) {
        SYLAR_ASSERT(events & event);
        events = (Event)(events & ~event);
        EventContext& ctx = getContext(event);
        if(ctx.cb) {
            ctx.scheduler->schedule(&ctx.cb);
        } else {
            ctx.scheduler->schedule(&ctx.fiber);
        }
        ctx.scheduler = nullptr;
        return;
    }

    enum EpollCtlOp {
    };

    IOManager::IOManager(size_t threads, bool use_caller, const std::string& name)
            :Scheduler(threads, use_caller, name) {
        m_epfd = epoll_create(5000);
        SYLAR_ASSERT(m_epfd > 0);

        //给m_tickleFds[0]注册读事件，当加入任务时，可以往m_tickleFds[1]写，保证程序不会被阻塞，从而监听到新加入的任务
        int rt = pipe(m_tickleFds);
        SYLAR_ASSERT(!rt);

        epoll_event event;
        memset(&event, 0, sizeof(epoll_event));
        //搭配非阻塞轮询可以防止数据丢失
        event.events = EPOLLIN | EPOLLET; //监听读事件，边沿触发（一次触发后，之后不再触发，一般设置为非阻塞轮询）
        event.data.fd = m_tickleFds[0];

        //非阻塞轮询
        rt = fcntl(m_tickleFds[0], F_SETFL, O_NONBLOCK);
        SYLAR_ASSERT(!rt);

        rt = epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_tickleFds[0], &event);
        SYLAR_ASSERT(!rt);

        contextResize(32);

        start();    //初始化好后就开始Scheduler的start

    }


    IOManager::~IOManager() {
        stop();
        close(m_epfd);
        close(m_tickleFds[0]);
        close(m_tickleFds[1]);

        for(size_t i = 0; i < m_fdContexts.size(); ++i) {
            if(m_fdContexts[i]) {
                delete m_fdContexts[i];
            }
        }
    }

    void IOManager::contextResize(size_t size) {
        m_fdContexts.resize(size);

        for(size_t i = 0; i < m_fdContexts.size(); ++i) {
            if(!m_fdContexts[i]) {
                m_fdContexts[i] = new FdContext;
                m_fdContexts[i]->fd = i;
            }
        }
    }
    /**
     * 注册事件。主要有以下几个步骤
        从m_fdContexts中拿到对应的fd: fd_ctx
        修改fd_ctx
        添加到m_epfd
     */
    int IOManager::addEvent(int fd, Event event, std::function<void()> cb) {

        FdContext* fd_ctx = nullptr;
        RWMutexType::ReadLock lock(m_mutex);

        //如果成立表示存在上下文，取出对应的fd
        if((int)m_fdContexts.size() > fd) {
            fd_ctx = m_fdContexts[fd];
            lock.unlock();
        } else {
            lock.unlock();
            RWMutexType::WriteLock lock2(m_mutex);
            contextResize(fd * 1.5);
            fd_ctx = m_fdContexts[fd];
        }

        //修改fd
        FdContext::MutexType::Lock lock2(fd_ctx->mutex);
        //如果fd——ctx由此事件出错
        if(fd_ctx->events & event) {
            SYLAR_LOG_ERROR(g_logger) << "addEvent assert fd=" << fd
                                      << " event=" << (EPOLL_EVENTS)event
                                      << " fd_ctx.event=" << (EPOLL_EVENTS)fd_ctx->events;
            SYLAR_ASSERT(!(fd_ctx->events & event));
        }

        //添加事件到——mepfd
        int op = fd_ctx->events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
        epoll_event epevent;
        epevent.events = EPOLLET | fd_ctx->events | event;
        epevent.data.ptr = fd_ctx;

        int rt = epoll_ctl(m_epfd, op, fd, &epevent);
        if(rt) {
            SYLAR_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
                                      << (EpollCtlOp)op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
                                      << rt << " (" << errno << ") (" << strerror(errno) << ") fd_ctx->events="
                                      << (EPOLL_EVENTS)fd_ctx->events;
            return -1;
        }

        ++m_pendingEventCount;
        //下面这几句话感觉没用
        //修改——fdctx的events
        fd_ctx->events = (Event)(fd_ctx->events | event);
        // 获取事件上下文
        FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
        SYLAR_ASSERT(!event_ctx.scheduler
                     && !event_ctx.fiber
                     && !event_ctx.cb);

        event_ctx.scheduler = Scheduler::GetThis();
        if(cb) {
            event_ctx.cb.swap(cb);
        } else {
            event_ctx.fiber = Fiber::GetThis();
            SYLAR_ASSERT(event_ctx.fiber->getState() == Fiber::EXEC);
        }
        return 0;
    }

    /**
     * 删除事件。主要有以下几个步骤
        从m_fdContexts中拿到对应的fd: fd_ctx
        修改fd_ctx
        从m_epfd删除
     */
     bool IOManager::delEvent(int fd, sylar::IOManager::Event event) {

        RWMutexType::ReadLock lock(m_mutex);
        if((int)m_fdContexts.size() <= fd) {
            return false;
        }
        FdContext* fd_ctx = m_fdContexts[fd];
        lock.unlock();

        FdContext::MutexType::Lock lock2(fd_ctx->mutex);
        //如果事件不再mfdcontexts里面 返回false
        if(!(fd_ctx->events & event)) {
            return false;
        }
        //在的话，判断状态，然后删除事件
        Event new_events = (Event)(fd_ctx->events & ~event);
        int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
        epoll_event epevent;
        epevent.events = EPOLLET | new_events;
        epevent.data.ptr = fd_ctx;

        int rt = epoll_ctl(m_epfd, op, fd, &epevent);
        if(rt) {
            SYLAR_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
                                      << (EpollCtlOp)op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
                                      << rt << " (" << errno << ") (" << strerror(errno) << ")";
            return false;
        }

        --m_pendingEventCount;
        //下面这几句话感觉没用
        fd_ctx->events = new_events;
        FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
        fd_ctx->resetContext(event_ctx);
        return true;

     }

     bool IOManager::cancelEvent(int fd, sylar::IOManager::Event event) {

         RWMutexType::ReadLock lock(m_mutex);
         if((int)m_fdContexts.size() <= fd) {
             return false;
         }
         FdContext* fd_ctx = m_fdContexts[fd];
         lock.unlock();

         FdContext::MutexType::Lock lock2(fd_ctx->mutex);
         if(SYLAR_UNLIKELY(!(fd_ctx->events & event))) {
             return false;
         }

         Event new_events = (Event)(fd_ctx->events & ~event);
         int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
         epoll_event epevent;
         epevent.events = EPOLLET | new_events;
         epevent.data.ptr = fd_ctx;

         int rt = epoll_ctl(m_epfd, op, fd, &epevent);
         if(rt) {
             SYLAR_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
                                       << (EpollCtlOp)op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
                                       << rt << " (" << errno << ") (" << strerror(errno) << ")";
             return false;
         }

         fd_ctx->triggerEvent(event);
         --m_pendingEventCount;
         return true;
     }

     bool IOManager::cancelAll(int fd) {

         //1. 从m_fdContexts中拿到对应的fd: fd_ctx
         RWMutexType::ReadLock lock(m_mutex);
         if((int)m_fdContexts.size() <= fd) {
             return false;
         }
         FdContext* fd_ctx = m_fdContexts[fd];
         lock.unlock();

         //2. 从m_epfd上删除所有事件
         FdContext::MutexType::Lock lock2(fd_ctx->mutex);
         if(!fd_ctx->events) {
             return false;
         }

         int op = EPOLL_CTL_DEL;
         epoll_event epevent;
         epevent.events = 0;
         epevent.data.ptr = fd_ctx;

         int rt = epoll_ctl(m_epfd, op, fd, &epevent);
         if(rt) {
             SYLAR_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
                                       << (EpollCtlOp)op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
                                       << rt << " (" << errno << ") (" << strerror(errno) << ")";
             return false;
         }

         //3. 触发事件
         if(fd_ctx->events & READ) {
             fd_ctx->triggerEvent(READ);
             --m_pendingEventCount;
         }
         if(fd_ctx->events & WRITE) {
             fd_ctx->triggerEvent(WRITE);
             --m_pendingEventCount;
         }

         //所有事件已经删除完毕
         SYLAR_ASSERT(fd_ctx->events == 0);
         return true;
     }

    IOManager* IOManager::GetThis() {
        return dynamic_cast<IOManager*>(Scheduler::GetThis());
    }

    void IOManager::tickle(){
        if(!hasIdleThreads()) {
            return;
        }
        int rt = write(m_tickleFds[1], "T", 1);
        SYLAR_ASSERT(rt == 1);
     }

    bool IOManager::stopping(uint64_t& timeout) {
        timeout = getNextTimer();
        return timeout == ~0ull
               && m_pendingEventCount == 0
               && Scheduler::stopping();

    }

   bool IOManager::stopping(){
       uint64_t timeout = 0;
       return stopping(timeout);
    }

    /**
     * ilde()函数。调度器无调度任务时会阻塞idle协程上，对IO调度器而言，idle状态应该关注两件事，
     * 一是有没有新的调度任务，对应Schduler::schedule()，如果有新的调度任务，那应该立即退出idle状态，
     * 并执行对应的任务；二是关注当前注册的所有IO事件有没有触发，如果有触发，那么应该执行。
     */
    void IOManager::idle() {
        SYLAR_LOG_DEBUG(g_logger) << "idle";
        //一次epoll_wait最多检测256个就绪事件，如果就绪事件超过了这个数，那么会在下轮epoll_wati继续处理
        const uint64_t MAX_EVNETS = 64;
        epoll_event* events = new epoll_event[MAX_EVNETS]();
        std::shared_ptr<epoll_event> shared_events(events, [](epoll_event* ptr){
            delete[] ptr;
        });

        while(true){
            uint64_t next_timeout = 0;
            if(stopping(next_timeout)){
                    SYLAR_LOG_INFO(g_logger) << "name= " << getName() << " idle stopping exit";
                    break;

            }

            // 阻塞在epoll_wait上，等待事件发生
            int rt = 0;
            do{
                static const int MAX_TIMEOUT = 5000; //毫秒单位
                if(next_timeout != ~0ull){
                    next_timeout = (int)next_timeout > MAX_TIMEOUT ? MAX_TIMEOUT : next_timeout;
                }else{
                    next_timeout = MAX_TIMEOUT;
                }
                // epoll_wait返回值为事件发生的数量
                rt = epoll_wait(m_epfd, events, 64, (int)next_timeout);

                //如果wait失败或者操作系统返回时候中断了（EINTR）则继续循环
                if(rt<0 && errno == EINTR){
                    //continue; //自己家的增加可读性，不对删除
                }else{
                    break;
                }
            }while(true);

            std::vector<std::function<void()> > cbs;
            listExpiredCb(cbs);
            if(!cbs.empty()) {
                schedule(cbs.begin(), cbs.end());
                cbs.clear();
            }

            // 遍历所有发生的事件，根据epoll_event的私有指针找到对应的FdContext，进行事件处理
            for(int i=0; i<rt; ++i){
                epoll_event& event = events[i];
                if(event.data.fd == m_tickleFds[0]){
                    // m_ticklefds[0]用于通知协程调度，这时只需要把管道里的内容读完即可，本轮idle结束Scheduler::run会重新执行协程调度
                    uint8_t dummy;
                    while(read(m_tickleFds[0], &dummy, 1) == 1);
                    continue;
                }

                // 通过epoll_event的私有指针获取FdContext
                FdContext* fd_ctx = (FdContext*)event.data.ptr;
                FdContext::MutexType::Lock lock(fd_ctx->mutex);
                /**
                 * EPOLLERR: 出错，比如写读端已经关闭的pipe
                 * EPOLLHUP: 套接字对端关闭
                 * 出现这两种事件，应该同时触发fd的读和写事件，否则有可能出现注册的事件永远执行不到的情况
                 */
                if(event.events & (EPOLLERR | EPOLLHUP)) {
                    event.events |= (EPOLLIN | EPOLLOUT) & fd_ctx->events;
                }
                int real_events = NONE;
                if(event.events & EPOLLIN) {
                    real_events |= READ;
                }
                if(event.events & EPOLLOUT) {
                    real_events |= WRITE;
                }

                if((fd_ctx->events & real_events) == NONE) {
                    continue;
                }

                // 剔除已经发生的事件，将剩下的事件重新加入epoll_wait，
                // 如果剩下的事件为0，表示这个fd已经不需要关注了，直接从epoll中删除
                int left_events = (fd_ctx->events & ~real_events);
                int op = left_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
                event.events = EPOLLET | left_events;

                int rt2 = epoll_ctl(m_epfd, op, fd_ctx->fd, &event);
                if(rt2) {
                    SYLAR_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
                                              << (EpollCtlOp)op << ", " << fd_ctx->fd << ", " << (EPOLL_EVENTS)event.events << "):"
                                              << rt2 << " (" << errno << ") (" << strerror(errno) << ")";
                    continue;
                }

                // 处理已经发生的事件，也就是让调度器调度指定的函数或协程
                if(real_events & READ) {
                    fd_ctx->triggerEvent(READ);
                    --m_pendingEventCount;
                }
                if(real_events & WRITE) {
                    fd_ctx->triggerEvent(WRITE);
                    --m_pendingEventCount;
                }

            }
            /** 让出执行权
             * 一旦处理完所有的事件，idle协程yield，这样可以让调度协程(Scheduler::run)重新检查是否有新任务要调度
             * 上面triggerEvent实际也只是把对应的fiber重新加入调度，要执行的话还要等idle协程退出
             */
            Fiber::ptr cur = Fiber::GetThis();
            auto raw_ptr = cur.get();
            cur.reset();

            raw_ptr->swapOut();

        }

     }

    void IOManager::onTimerInsertedAtFront() {
        tickle();
    }





}