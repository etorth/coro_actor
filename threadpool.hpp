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

    private:
        std::mutex m_queueLock;
        std::condition_variable m_cond;
        bool m_stop = false;

    private:
        std::mutex m_actorsLock;
        std::unordered_map<int, Actor *> m_actors;

    public:
        ThreadPool(size_t);
        ~ThreadPool()
        {
            {
                const std::unique_lock<std::mutex> lock(m_queueLock);
                m_stop = true;
            }

            m_cond.notify_all();
            for (std::thread &worker : m_workers){
                worker.join();
            }
        }

    public:
        void   registerActor(Actor *);
        void deregisterActor(int);

    public:
        Actor *getActor(int address);

    public:
        void scheduleActor(Actor *);
};
