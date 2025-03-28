#pragma once
#include <unordered_map>
#include <stdexcept>
#include <atomic>
#include <optional>

#include "utils.hpp"
#include "message.hpp"
#include "threadpool.hpp"
#include "sendmsgcoro.hpp"
#include "freemsgcoro.hpp"

class Actor
{
    private:
        friend class ThreadPool;

    private:
        ThreadPool &m_pool;

    private:
        const int m_address;
        const std::string m_name;

    private:
        std::mutex m_mailboxLock;
        std::vector<Message> m_mailbox;

    private:
        std::atomic_flag m_processing;
        std::atomic<int> m_sequence {1}; // start sequence from 1, as 0 is used for messages that don't need a response

    private:
        std::unordered_map<int, std::coroutine_handle<SendMsgCoro::promise_type>> m_respHandlerList;

    private:
        size_t m_recvMsgCount = 0; // micmic that message can change actor's internal state, no functional usage
        size_t m_badAddrCount = 0;
        size_t m_replyCharCount = 0;

    private:
        struct RegisterContinuationAwaiter
        {
            Actor *actor;
            const int seqID;

            bool await_ready() const
            {
                return false;
            }

            void await_suspend(std::coroutine_handle<SendMsgCoro::promise_type> handle)
            {
                actor->m_respHandlerList.emplace(seqID, handle);
            }

            void await_resume(){}
        };

    public:
        Actor(ThreadPool &pool, int address)
            : m_pool(pool)
            , m_address(address)
            , m_name(randstr(6))
        {}

        int getAddress() const
        {
            return m_address;
        }

        bool trySetProcessing()
        {
            return !m_processing.test_and_set();
        }

        void resetProcessing()
        {
            m_processing.clear();
        }

    public:
        bool        post(const std::pair<int, int> &, MessagePack);
        SendMsgCoro send(const std::pair<int, int> &, MessagePack);

    private:
        std::optional<int> doPost(const std::pair<int, int> &, bool, MessagePack);

    public:
        void receive(const Message &);
        void consumeMessages();

    public:
        FreeMsgCoro onFreeMessage(Message);
        void        onCoroMessage(Message);

    private:
        FreeMsgCoro on_MPK_INIT     (Message);
        FreeMsgCoro on_MPK_HELLO    (Message);
        FreeMsgCoro on_MPK_QUERYNAME(Message);
};
