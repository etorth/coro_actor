#include <stdexcept>
#include "actor.hpp"

std::optional<int> Actor::doPost(const std::pair<int, int> &toAddr, bool waitResp, MessagePack mpk)
{
    if(auto target = m_pool.getActor(toAddr.first)){
        Message msg
        {
            .type = mpk.type,
            .content = std::move(mpk.content),
            .from = getAddress(),
            .seqID = waitResp ? m_sequence++ : 0,
            .respID = toAddr.second,
        };

        printMessage("Actor %2d send message to   actor %2d: %s\n", getAddress(), toAddr.first, msg.str().c_str());
        {
            const std::unique_lock<std::mutex> lock(target->m_mailboxLock);
            target->m_mailbox.push_back(std::move(msg));
        }

        m_pool.scheduleActor(target);
        return msg.seqID;
    }
    return std::nullopt;
}

bool Actor::post(const std::pair<int, int> &toAddr, MessagePack mpk)
{
    return doPost(toAddr, false, std::move(mpk)).has_value();
}

corof::awaitable<Message> Actor::send(const std::pair<int, int> &toAddr, MessagePack mpk)
{
    co_await RegisterContinuationAwaiter
    {
        .actor = this,
        .seqID = doPost(toAddr, true, std::move(mpk)),
    };
}

void Actor::receive(const Message& msg)
{
    printMessage("Actor %2d recv message from actor %2d: %s\n", getAddress(), msg.from, msg.str().c_str());
    m_recvMsgCount++;

    if(msg.respID > 0){
        onCoroMessage(msg);
    }
    else{
        onFreeMessage(msg)();
    }
}

void Actor::consumeMessages()
{
    while(true){
        std::vector<Message> messages;
        {
            std::unique_lock<std::mutex> lock(m_mailboxLock);
            std::swap(messages, m_mailbox);
        }

        if(messages.empty()){
            break;
        }

        for(auto& message: messages) {
            receive(std::move(message));
        }
    }
}

corof::entrance Actor::onFreeMessage(Message msg)
{
    switch(msg.type){
        case MPK_INIT     : return on_MPK_INIT     (std::move(msg));
        case MPK_HELLO    : return on_MPK_HELLO    (std::move(msg));
        case MPK_QUERYNAME: return on_MPK_QUERYNAME(std::move(msg));
        default           : throw  std::runtime_error(str_printf("Unknown message: %s", msg.str().c_str()));
    }
}

void Actor::onCoroMessage(Message msg)
{
    if(auto p = m_respHandlerList.find(msg.respID); p != m_respHandlerList.end()){
        p->second.promise().result = std::move(msg);
        p->second.resume();
        m_respHandlerList.erase(p);
    }
    else{
        throw std::runtime_error(str_printf("No handler for response: %s", msg.str().c_str()));
    }
}
