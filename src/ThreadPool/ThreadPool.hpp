
#include <thread>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>

class job {
public:
    job(std::function<void(void *)> func, void *args);
    std::function<void (void *)> func;
    void *args;
};

class ThreadPool {
public:
    std::mutex              workMutex;
    std::condition_variable newJobCond;         // Signals when there's a new job to be processed
    std::condition_variable noJobCond;          // Signals when all threads are not working
    std::queue<job>         jobQueue;
    bool                    stop;
    size_t                  workingCount;
    size_t                  threadCount;
    size_t                  maxJobCount;

    void init(size_t threadCount = std::thread::hardware_concurrency(), size_t maxJobCount = 0);

    void shutdown(bool finishRemainingJobs = true);

    bool addJob(job newJob);

    void waitForAllJobsDone();
};
