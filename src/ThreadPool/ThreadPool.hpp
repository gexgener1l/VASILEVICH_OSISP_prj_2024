
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
    std::condition_variable newJobCond;      
    std::condition_variable noJobCond;        
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
