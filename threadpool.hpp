#pragma once
#include <vector>
#include <queue>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <memory>

class Actor;
class ThreadPool
{
    private:
        std::vector<std::thread> m_workers;
        std::queue<Actor *> m_pendingActors;
        std::mutex m_queueLock;
        std::condition_variable m_cond;
        bool stop = false;

        std::unordered_map<int, Actor *> actors;
        std::mutex actorsMutex;

    public:
        ThreadPool(size_t);
        ~ThreadPool()
        {
            {
                std::unique_lock<std::mutex> lock(m_queueLock);
                stop = true;
            }
            m_cond.notify_all();
            for (std::thread &worker : m_workers){
                worker.join();
            }
        }

    public:
        void registerActor(Actor *);
        Actor *getActor(int address);

    public:
        void scheduleActor(Actor *);
};
