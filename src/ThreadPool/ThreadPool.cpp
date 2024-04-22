
#include "ThreadPool.hpp"

void worker(ThreadPool *pool);

job::job(std::function<void (void *)> func, void *args) {
    this->func = func;
    this->args = args;
}

void ThreadPool::init(size_t threadCount, size_t maxJobCount) {
    if (threadCount == 0) {
        return;
    }

    this->threadCount = threadCount;
    this->workingCount = 0;
    this->stop = false;
    this->maxJobCount = maxJobCount;
    
    for (size_t i = 0; i < threadCount; i++) {
        std::thread(worker, this).detach();
    }
}

void ThreadPool::shutdown(bool finishRemainingJobs) {
    this->workMutex.lock();
    while (!this->jobQueue.empty()) {
        if (finishRemainingJobs) {
            job currentJob = this->jobQueue.front();
            if (currentJob.func) {
                currentJob.func(currentJob.args);
            }
        }
        this->jobQueue.pop();
    }
    this->stop = true;
    this->newJobCond.notify_all(); 
    this->workMutex.unlock();
}

bool ThreadPool::addJob(job newJob) {
    this->workMutex.lock();

    if (maxJobCount == 0 || this->jobQueue.size() < maxJobCount) {
        this->jobQueue.push(newJob);
        this->newJobCond.notify_all();
        this->workMutex.unlock();
        return true;
    }
    else {
        this->workMutex.unlock();
        return false;
    }
}

void ThreadPool::waitForAllJobsDone() {
    std::unique_lock<std::mutex> lock(this->workMutex);
    for (;;) {
        if (this->workingCount != 0 || !this->jobQueue.empty()) {
            this->noJobCond.wait(lock);
        }
        else {
            break;
        }
    }
    lock.unlock();
}

void worker(ThreadPool *pool) {
    for (;;) {
        std::unique_lock<std::mutex> lock(pool->workMutex);

        while (pool->jobQueue.empty() && !pool->stop) {
            pool->newJobCond.wait(lock);
        }
        
        if (pool->stop) {
            pool->threadCount--;
            pool->noJobCond.notify_one();
            lock.unlock();
            return;
        }

        job currentJob = pool->jobQueue.front();
        pool->jobQueue.pop();
        pool->workingCount++;
        lock.unlock();

        if (currentJob.func) {
            currentJob.func(currentJob.args);
        }

        lock.lock();
        pool->workingCount--;
        if (pool->jobQueue.empty() && !pool->stop && pool->workingCount == 0) {
            pool->noJobCond.notify_one();
        }
        lock.unlock();
    }
}
