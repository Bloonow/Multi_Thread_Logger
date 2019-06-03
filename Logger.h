#ifndef LOGGER_MULTI_THREAD_H
#define LOGGER_MULTI_THREAD_H

#include <fstream>
#include <string>
#include <queue>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>

class Logger {
public:
    // 构造函数创建后台线程
    Logger() : isExited(false) {
        mThread = std::thread{&Logger::process, this};
    }

    // 显示删除复制构造函数和赋值运算符
    Logger(const Logger& src) = delete;
    Logger& operator=(const Logger& rhs) = delete;

    // 析构函数唤醒后台线程，将消息列队的信息全部写出到文件后，销毁对象
    ~Logger() {
        {
            // 上锁 (锁在此花括号代码块中有效，避免出现死锁)
            std::unique_lock<std::mutex> lock(mut);
            isExited = true;
            // 唤醒后台线程
            condVar.notify_all();
        }
        // 此时锁已释放，后台线程可获得锁
        mThread.join();
    }

    // 客户调用借口，向消息列队写入一条消息，并唤醒后台线程
    void log(const std::string& entry) {
        std::unique_lock<std::mutex> lock(mut);
        messagesQueue.push(entry);
        condVar.notify_all();
    }

private:
    // 后台线程，将消息列队中的信息写入文件
    void process() {
        std::ofstream ofs("log.txt", std::ios::out /*| std::ios::app*/);
        if (ofs.fail()) {
            std::cerr << "Failed to open logFile." << std::endl;
        }
        // 上锁
        std::unique_lock<std::mutex> lock(mut);
        while (true) {
            // 判断是否退出，若退出为假，等待唤醒
            if (!isExited) {
                condVar.wait(lock);
            }
            // 释放锁
            lock.unlock();
            while (true) {
                // 上锁，每次处理一条信息，每次处理时上锁和释放锁，防止线程阻塞时间过长
                lock.lock();
                if (messagesQueue.empty()) { break; }
                else {
                    ofs << messagesQueue.front() << std::endl;
                    messagesQueue.pop();
                }
                lock.unlock();
            }
            if (isExited) break;
        }
    }

    std::queue<std::string> messagesQueue; //! 消息列队
    std::atomic<bool> isExited; //! 退出状态 原子bool类型
    std::mutex mut; //! 非定时互斥体
    std::condition_variable condVar; //! 条件变量
    std::thread mThread; //! 线程
};

#endif // LOGGER_MULTI_THREAD_H
