#include "threadpool.hpp"
#include "actor.hpp"

ThreadPool::ThreadPool(size_t numThreads)
{
    for (size_t i = 0; i < numThreads; ++i) {
        m_workers.emplace_back([this]
        {
            while (true) {
                Actor *pendingActor;
                {
                    std::unique_lock<std::mutex> lock(this->m_queueLock);
                    this->m_cond.wait(lock, [this]
                    {
                        return this->stop || !this->m_pendingActors.empty();
                    });

                    if(this->stop && this->m_pendingActors.empty()){
                        return;
                    }

                    pendingActor = this->m_pendingActors.front();
                    this->m_pendingActors.pop();
                }

                if(pendingActor->trySetProcessing()){
                    pendingActor->consumeMessages();
                    pendingActor->resetProcessing();
                }
            }
        });
    }
}

void ThreadPool::registerActor(Actor *actor)
{
    actor->m_mailbox.push_back(Message // no need to lock
    {
        .type = MPK_INIT,
    });

    {
        const std::unique_lock<std::mutex> lock(actorsMutex);
        actors[actor->getAddress()] = actor;
    }
    scheduleActor(actor);
}

Actor *ThreadPool::getActor(int address)
{
    std::unique_lock<std::mutex> lock(actorsMutex);
    if(auto p = actors.find(address); p != actors.end()){
        return p->second;
    }
    else{
        return nullptr;
    }
}

void ThreadPool::scheduleActor(Actor *actor)
{
    {
        std::unique_lock<std::mutex> lock(m_queueLock);
        if (stop){
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }
        m_pendingActors.push(actor);
    }
    m_cond.notify_one();
}
