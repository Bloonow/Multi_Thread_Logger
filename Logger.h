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
    // ���캯��������̨�߳�
    Logger() : isExited(false) {
        mThread = std::thread{&Logger::process, this};
    }

    // ��ʾɾ�����ƹ��캯���͸�ֵ�����
    Logger(const Logger& src) = delete;
    Logger& operator=(const Logger& rhs) = delete;

    // �����������Ѻ�̨�̣߳�����Ϣ�жӵ���Ϣȫ��д�����ļ������ٶ���
    ~Logger() {
        {
            // ���� (���ڴ˻����Ŵ��������Ч�������������)
            std::unique_lock<std::mutex> lock(mut);
            isExited = true;
            // ���Ѻ�̨�߳�
            condVar.notify_all();
        }
        // ��ʱ�����ͷţ���̨�߳̿ɻ����
        mThread.join();
    }

    // �ͻ����ý�ڣ�����Ϣ�ж�д��һ����Ϣ�������Ѻ�̨�߳�
    void log(const std::string& entry) {
        std::unique_lock<std::mutex> lock(mut);
        messagesQueue.push(entry);
        condVar.notify_all();
    }

private:
    // ��̨�̣߳�����Ϣ�ж��е���Ϣд���ļ�
    void process() {
        std::ofstream ofs("log.txt", std::ios::out /*| std::ios::app*/);
        if (ofs.fail()) {
            std::cerr << "Failed to open logFile." << std::endl;
        }
        // ����
        std::unique_lock<std::mutex> lock(mut);
        while (true) {
            // �ж��Ƿ��˳������˳�Ϊ�٣��ȴ�����
            if (!isExited) {
                condVar.wait(lock);
            }
            // �ͷ���
            lock.unlock();
            while (true) {
                // ������ÿ�δ���һ����Ϣ��ÿ�δ���ʱ�������ͷ�������ֹ�߳�����ʱ�����
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

    std::queue<std::string> messagesQueue; //! ��Ϣ�ж�
    std::atomic<bool> isExited; //! �˳�״̬ ԭ��bool����
    std::mutex mut; //! �Ƕ�ʱ������
    std::condition_variable condVar; //! ��������
    std::thread mThread; //! �߳�
};

#endif // LOGGER_MULTI_THREAD_H
