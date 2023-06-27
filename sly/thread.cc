//
// Created by sly on 6/14/23.
//

#include "thread.h"
#include "log.h"
#include "util.h"

namespace sylar{

    ///定义线程局部变量，用于写GetThis()
    ///一个ThreadLocal在一个线程中是共享的，在不同线程之间又是隔离的（每个线程都只能看到自己线程的值）
    /**
     * 这是一个静态线程局部变量，每个线程都会有一个独立的副本。在这段代码中，它被定义为一个指向Thread对象的指针，
     * 并初始化为nullptr。它用于存储当前线程的实例
     */
    static thread_local  Thread* t_thread = nullptr;
    /**
     * 这是另一个静态线程局部变量，也是每个线程的独立副本。它被定义为一个std::string类型的对象，
     * 并初始化为"UNKNOW"。它用于存储当前线程的名称。
     */
    static thread_local std::string t_thread_name = "UNKNOW";

    static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

    Semaphore::Semaphore(uint32_t count) {
        if(sem_init(&m_semaphore, 0, count)){
            throw std::logic_error("sem_init error");
        }
    }

    Semaphore::~Semaphore() {
        sem_destroy(&m_semaphore);
    }

    void Semaphore::wait() {
            if(sem_wait(&m_semaphore)){
                throw std::logic_error("sem_wait error");
            }
    }

    void Semaphore::notify() {
        if(sem_post(&m_semaphore)){
            throw std::logic_error("sem_post error");
        }

    }


    Thread* Thread::GetThis(){
        return t_thread;
    }

    const std::string& Thread::GetName(){
        return t_thread_name;
    }

    void Thread::SetName(const std::string& name){
        if(t_thread){
            t_thread->m_name = name;
        }
        t_thread_name = name;
    }

    Thread::Thread(std::function<void()> cb, const std::string& name)
        :m_cb(cb)
        ,m_name(name) {
        if(name.empty()){
            m_name = "UNKNOW";
        }
        int rt = pthread_create(&m_thread, nullptr, &Thread::run, this);
        if(rt){
            SYLAR_LOG_ERROR(g_logger) << "pthread_create thread fail, rt= " << rt
                                      << "name= " << name;
            throw std::logic_error("pthread_create error");
        }
        // 加上一直不运行  等待，直到创建出的线程开始执行，run()
        //m_semaphore.wait();
    }

    Thread::~Thread(){
        if(m_thread){
            pthread_detach(m_thread);
        }
    }


    void Thread::join(){
        if(m_thread){
            //pthread_join() 函数成功等到了目标线程执行结束返回值为数字 0；反之如果执行失败，函数会根据失败原因返回相应的非零值，
            int rt = pthread_join(m_thread, nullptr);
            if(rt){
                SYLAR_LOG_ERROR(g_logger) << "pthread_join thread fail, rt= " << rt
                                          << "name= " << m_name;
                throw std::logic_error("pthread_join error");
            }
            m_thread = 0;
        }
    }

    void* Thread::run(void* arg){
        // 传入的arg强制转换成Thread类性，获得了线程对象指针
        Thread* thread = (Thread*)arg;
        // 将全局的静态线程局部变量t_thread设置为当前线程的对象指针，这样就可以在其他地方通过Thread::GetThis()函数获取到当前线程的实例
        t_thread = thread;
        t_thread_name = thread->m_name;
        // 用于记录线程的ID。
        thread->m_id = sylar::GetThreadId();
        // 使用pthread_setname_np函数设置当前线程的名称，将线程对象的成员变量m_name的前15个字符作为线程名。
        pthread_setname_np(pthread_self(), thread->m_name.substr(0, 15).c_str());
        // 声明了一个空的std::function对象cb，用于保存线程对象中的任务函数。
        std::function<void()> cb;
        // 将线程对象中的任务函数（通过Thread::setCallback()设置）与cb进行交换，以便在后续执行
        cb.swap(thread->m_cb);
        // 执行任务函数，即调用线程对象中设置的回调函数。
        cb();
        thread->m_semaphore.notify();

        return 0;
    }
}
