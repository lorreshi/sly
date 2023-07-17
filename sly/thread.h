//
// Created by sly on 6/14/23.
//

#ifndef SLY_THREAD_H
#define SLY_THREAD_H

#include <thread>
#include <functional>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <iostream>
#include <atomic>

namespace sylar{
    /**
     * 信号量类
     */
    class Semaphore{
    public:
        Semaphore(uint32_t count = 0);
        ~Semaphore();

        void wait();
        void notify();
    private:
        // 拷贝构造
        Semaphore(const Semaphore&) = delete;
        //移动构造
        Semaphore(const Semaphore&&) = delete;
        //运算符重载
        Semaphore& operator=(const Semaphore&) = delete;
    private:
        sem_t m_semaphore;
    };


    /// 互斥量类模板
    /// \tparam T
    template<class T>
    struct ScopedLockImpl{
    public:
        ScopedLockImpl(T& mutex):m_mutex(mutex){
            m_mutex.lock();
            m_locked = true;
        }

        ~ScopedLockImpl(){
            unlock();
        }

        void lock(){
            if(!m_locked){
                m_mutex.lock();
                m_locked = true;
            }
        }

        void unlock(){
            if(m_locked){
                m_mutex.unlock();
                m_locked = false;
            }
        }

    private:
        T& m_mutex;
        bool m_locked;
    };

    /// 读锁模板类
    /// \tparam T
    template<class T>
    struct ReadScopedLockImpl{
    public:
        ReadScopedLockImpl(T& mutex):m_mutex(mutex){
            m_mutex.rdlock();
            m_locked = true;
        }

        ~ReadScopedLockImpl(){
            unlock();
        }

        void lock(){
            if(!m_locked){
                m_mutex.rdlock();
                m_locked = true;
            }
        }

        void unlock(){
            if(m_locked){
                m_mutex.unlock();
                m_locked = false;
            }
        }

    private:
        T& m_mutex;
        bool m_locked;
    };

    /// 写锁模板类
    /// \tparam T
    template<class T>
    struct WriteScopedLockImpl{
    public:
        WriteScopedLockImpl(T& mutex):m_mutex(mutex){
            m_mutex.wrlock();
            m_locked = true;
        }

        ~WriteScopedLockImpl(){
            unlock();
        }

        void lock(){
            if(!m_locked){
                m_mutex.wrlock();
                m_locked = true;
            }
        }

        void unlock(){
            if(m_locked){
                m_mutex.unlock();
                m_locked = false;
            }
        }

    private:
        T& m_mutex;
        bool m_locked;
    };

    class Mutex{
    public:
        typedef  ScopedLockImpl<Mutex> Lock;
        Mutex(){
            pthread_mutex_init(&m_mutex, nullptr);
        }

        ~Mutex(){
            pthread_mutex_destroy(&m_mutex);
        }

        void lock(){
            pthread_mutex_lock(&m_mutex);
        }

        void unlock(){
            pthread_mutex_unlock(&m_mutex);
        }

    private:
        pthread_mutex_t m_mutex;
    };

    //空类 比较家所与不家所 增加代码一致性
    class NullMute{
    public:
        typedef  ScopedLockImpl<Mutex> Lock;
        NullMute(){}
        ~NullMute(){}
        void lock(){}
        void unlock(){}
    private:
    };


    class RWMutex{
    public:
        typedef ReadScopedLockImpl<RWMutex> ReadLock;
        typedef WriteScopedLockImpl<RWMutex> WriteLock;
        RWMutex(){
            pthread_rwlock_init(&m_lock, nullptr);
        }

        ~RWMutex(){
            pthread_rwlock_destroy(&m_lock);
        }

        void rdlock(){
            pthread_rwlock_rdlock(&m_lock);
        }

        void wrlock(){
            pthread_rwlock_wrlock(&m_lock);
        }

        void unlock(){
            pthread_rwlock_unlock(&m_lock);
        }

    private:
        pthread_rwlock_t m_lock;
    };


    // 空类，验证有所无所区别
    class NullRWMutex{
    public:
        typedef ReadScopedLockImpl<NullRWMutex> ReadLock;
        typedef ReadScopedLockImpl<NullRWMutex> WriteLock;

        NullRWMutex(){}
        ~NullRWMutex(){}
        void rdlock(){}
        void wrlock(){}
        void unlock(){}

    private:
    };

    /**
     * 自旋锁类
     */
    class Spinlock{
    public:
        /// 局部锁
        typedef ScopedLockImpl<Spinlock> Lock;

        /**
         * @brief 构造函数
         */
        Spinlock() {
            pthread_spin_init(&m_mutex, 0);
        }

        /**
         * @brief 析构函数
         */
        ~Spinlock() {
            pthread_spin_destroy(&m_mutex);
        }

        /**
         * @brief 上锁
         */
        void lock() {
            pthread_spin_lock(&m_mutex);
        }

        /**
         * @brief 解锁
         */
        void unlock() {
            pthread_spin_unlock(&m_mutex);
        }
    private:
        /// 自旋锁
        pthread_spinlock_t m_mutex;
    };

    /**
     * @brief 原子锁
     */
    class CASLock{
    public:
        /// 局部锁
        typedef ScopedLockImpl<CASLock> Lock;

        /**
         * @brief 构造函数
         */
        CASLock() {
            m_mutex.clear();
        }

        /**
         * @brief 析构函数
         */
        ~CASLock() {
        }

        /**
         * @brief 上锁
         */
        void lock() {
            while(std::atomic_flag_test_and_set_explicit(&m_mutex, std::memory_order_acquire));
        }

        /**
         * @brief 解锁
         */
        void unlock() {
            std::atomic_flag_clear_explicit(&m_mutex, std::memory_order_release);
        }
    private:
        /// 原子状态
        volatile std::atomic_flag m_mutex;
    };


    /**
    * @brief 线程类
    */
    class Thread{
    public:
        /// 线程智能指针类型
        typedef std::shared_ptr<Thread> ptr;
        /**
         * @brief 构造函数
         * @param[in] cb 线程执行函数
         * @param[in] name 线程名称
        */
        Thread(std::function<void()> cb, const std::string& name);

        ~Thread();

        /**
         * @brief 线程ID
         */
        pid_t getId() const { return m_id;}

        /**
         * @brief 线程名称
         */
        const std::string& getName() const { return m_name;}

        /**
         * @brief 等待线程执行完成
         */
        void join();


        /**
         * @brief 获取当前的线程指针
         */
        static Thread* GetThis();
        /**
         * @brief 获取当前的线程名称
         */
        static const std::string& GetName();
        /**
         * 设置线程名称
         */
        static void SetName(const std::string& name);



    private:
        //禁止线程拷贝函数，因为互斥与信号量不能拷贝
        Thread(const Thread&) = delete;
        Thread(const Thread&&) = delete;
        Thread& operator=(const Thread&) = delete;

        static void* run(void* arg);
    private:
        /// 线程id
        pid_t m_id = -1;
        /// 线程结构
        pthread_t m_thread = 0;
        /// 线程执行函数
        std::function<void()> m_cb;
        /// 线程名称
        std::string m_name;
        /// 信号量
        Semaphore m_semaphore;
    };
}

#endif //SLY_THREAD_H
