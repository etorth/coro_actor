#pragma once
#include <unordered_map>
#include <stdexcept>
#include <atomic>
#include <optional>

#include "corof.hpp"
#include "utils.hpp"
#include "message.hpp"
#include "threadpool.hpp"

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
        std::unordered_map<int, std::coroutine_handle<corof::awaitable<Message>::promise_type>> m_respHandlerList;

    private:
        size_t m_recvMsgCount = 0; // micmic that message can change actor's internal state, no functional usage
        size_t m_badAddrCount = 0;
        size_t m_replyCharCount = 0;

    private:
        struct RegisterContinuationAwaiter
        {
            Actor *            const actor;
            std::optional<int> const seqID;

            bool await_ready() const
            {
                return false;
            }

            bool await_suspend(std::coroutine_handle<corof::awaitable<Message>::promise_type> handle)
            {
                if(seqID.has_value()){
                    actor->m_respHandlerList.emplace(seqID.value(), handle);
                    return true;
                }
                else{
                    handle.promise().return_value(Message
                    {
                        .type = MPK_BADADDR,
                    });
                    return false;
                }
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
        bool                      post(const std::pair<int, int> &, MessagePack);
        corof::awaitable<Message> send(const std::pair<int, int> &, MessagePack);

    private:
        std::optional<int> doPost(const std::pair<int, int> &, bool, MessagePack);

    public:
        void receive(const Message &);
        void consumeMessages();

    public:
        corof::awaitable<bool> queryBool(int);

    public:
        corof::awaitable<> onFreeMessage(Message);
        void            onCoroMessage(Message);

    private:
        corof::awaitable<> on_MPK_INIT     (Message);
        corof::awaitable<> on_MPK_HELLO    (Message);
        corof::awaitable<> on_MPK_QUERYNAME(Message);
        corof::awaitable<> on_MPK_QUERYBOOL(Message);
};
