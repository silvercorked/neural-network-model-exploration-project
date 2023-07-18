
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <queue>
#include <vector>

namespace ThreadManagement {
    class ThreadPool {
        const uint32_t numberOfThreads;
        bool shouldTerminate;
        std::mutex queueMutex;
        std::condition_variable mutexCondition;
        std::vector<std::thread> threads;
        std::queue<std::function<void()>> tasks;

        void threadLoop();
    public:
        ThreadPool(const uint32_t);
        void queueTask(const std::function<void()>&);
        auto unassignedTasks();
        bool busy();
        ~ThreadPool();
    };

    ThreadPool::ThreadPool(const uint32_t numbOfthreads = std::thread::hardware_concurrency())
        : numberOfThreads(numbOfthreads != 0 ? numbOfthreads : 4) // default 4
        , shouldTerminate(false)
        , threads(std::vector<std::thread>())
        , tasks(std::queue<std::function<void()>>())
    {
        threads.reserve(this->numberOfThreads);
        for (uint32_t i = 0; i < this->numberOfThreads; i++) {
            threads.push_back(
                std::thread(
                    [this]() { this->threadLoop(); }
                )
            );
        }
    }
    void ThreadPool::threadLoop() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(this->queueMutex);
                this->mutexCondition.wait(lock, [this] {
                    return !this->tasks.empty() || this->shouldTerminate;
                });
                if (this->shouldTerminate) return;
                task = this->tasks.front();
                this->tasks.pop();
            }
            task();
        }
    }
    void ThreadPool::queueTask(const std::function<void()>& task) {
        {
            std::unique_lock<std::mutex> lock(this->queueMutex);
            this->tasks.push(task);
        }
        this->mutexCondition.notify_one();
    }
    auto ThreadPool::unassignedTasks() {
        std::queue<std::function<void()>>::size_type len;
        {
            std::unique_lock<std::mutex> lock(this->queueMutex);
            len = this->tasks.size();
        }
        return len;

    }
    bool ThreadPool::busy() {
        bool poolBusy;
        {
            std::unique_lock<std::mutex> lock(this->queueMutex);
            poolBusy = !this->tasks.empty();
        }
        return poolBusy;
    }
    ThreadPool::~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(this->queueMutex);
            this->shouldTerminate = true;
        }
        this->mutexCondition.notify_all();
        for(std::thread& activeThread : this->threads) {
            activeThread.join();
        }
    }
}