//
// Created by sly on 6/19/23. 协程头文件
//

#ifndef SLY_FIBER_H
#define SLY_FIBER_H

#include <memory>
#include <functional>
#include <ucontext.h>

namespace sylar{

class Scheduler;
class Fiber : public std::enable_shared_from_this<Fiber>{

friend class Scheduler;
public:
    typedef std::shared_ptr<Fiber> ptr;

    /**
    * @brief 协程状态
    */
    enum State {
        /// 初始化状态
        INIT,
        /// 暂停状态
        HOLD,
        /// 执行中状态
        EXEC,
        /// 结束状态
        TERM,
        /// 可执行状态
        READY,
        /// 异常状态
        EXCEPT
    };

private:
    /**
     * @brief 无参构造函数
     * @attention 每个线程第一个协程的构造,不允许创造默认构造函数
     * 主协程函数由此创造
     */
    Fiber();

public:
    /**
     * @brief 构造函数
     * @param[in] cb 协程执行的函数
     * @param[in] stacksize 协程栈大小
     * @param[in] use_caller 是否在MainFiber上调度
     */
    Fiber(std::function<void()> cb, size_t stacksize = 0, bool use_caller = false);

    /**
     * @brief 析构函数
     */
    ~Fiber();

    /**
     * @brief 重置协程执行函数,并设置状态
     * @pre getState() 为 INIT, TERM, EXCEPT
     * @post getState() = INIT
     */
    void reset(std::function<void()> cb);

    // 将当前协程切换到运行状态， Scheduler调度线程 --> 当前线程
    void swapIn();

    // 退出当前协程，当前协程 --> Scheduler调度协程
    void swapOut();

    // 将当前协程切换到执行状态 主协程-->当前协程
    void call();

    // 将当前协程切换到后台，当前协程-->主协程
    void back();

    /**
     * @brief 返回协程状态
     */
    State getState() const { return m_state;}
    /**
    * @brief 返回协程id
    */
    uint64_t getId() const { return m_id;}

    /**
     * @brief 设置当前线程的运行协程
     * @param[in] f 运行协程
     */
    static void SetThis(Fiber* f);

    /**
     * @brief 返回当前所在的协程
     */
    static Fiber::ptr GetThis();

    /**
     * @brief 将当前协程切换到后台,并设置为READY状态
     * @post getState() = READY
     */
    static void YieldToReady();

    /**
     * @brief 将当前协程切换到后台,并设置为HOLD状态
     * @post getState() = HOLD
     */
    static void YieldToHold();

    /**
     * @brief 返回当前协程的总数量
     */
    static uint64_t TotalFibers();

    /**
     * @brief 协程执行函数
     * @post 执行完成返回到线程主协程
     */
    static void MainFunc();

    /**
     * @brief 协程执行函数
     * @post 执行完成返回到线程调度协程
     */
    static void CallerMainFunc();

    /**
    * @brief 获取当前协程的id
    */
    static uint64_t GetFiberId();

private:
    /// 协程id
    uint64_t m_id = 0;
    /// 协程运行栈大小
    uint32_t m_stacksize = 0;
    /// 协程状态
    State m_state = INIT;
    /// 协程上下文
    ucontext_t m_ctx;
    /// 协程运行栈指针
    void* m_stack = nullptr;
    /// 协程运行函数
    std::function<void()> m_cb;

};


}

#endif //SLY_FIBER_H
