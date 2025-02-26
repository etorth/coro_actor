#pragma once
#include <coroutine>
#include "message.hpp"

class Actor;
struct SendMsgCoro;

struct SendMsgCoroPromise;
struct SendMsgCoroPromiseFinalAwaiter
{
    bool await_ready() const noexcept { return false; }
    void await_suspend(std::coroutine_handle<SendMsgCoroPromise>) noexcept;
    void await_resume() noexcept {}
};

struct SendMsgCoroPromise
{
    Actor *actor;
    std::coroutine_handle<> continuation;

    template<typename... Args> SendMsgCoroPromise(Actor &self, Args && ...)
        : actor(&self)
    {}

    SendMsgCoro get_return_object() noexcept;
    void return_value(Message);

    std::suspend_always         initial_suspend() const noexcept { return {}; }
    SendMsgCoroPromiseFinalAwaiter final_suspend() const noexcept { return {}; }


    void unhandled_exception()
    {
        std::terminate();
    }
};

struct SendMsgCoroAwaitable
{
    std::coroutine_handle<SendMsgCoroPromise> handle;

    bool await_ready() const noexcept
    {
        return false;
    }

    void await_suspend(std::coroutine_handle<> h) noexcept;
    Message await_resume() noexcept;
};

struct SendMsgCoro
{
    using promise_type = SendMsgCoroPromise;
    std::coroutine_handle<promise_type> handle;

    SendMsgCoro(std::coroutine_handle<promise_type> h)
        : handle(h)
    {}

    SendMsgCoroAwaitable operator co_await() && noexcept
    {
        return {handle};
    }
};
