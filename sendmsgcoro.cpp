#include "sendmsgcoro.hpp"
#include "actor.hpp"

void SendMsgCoroPromiseFinalAwaiter::await_suspend(std::coroutine_handle<SendMsgCoroPromise> handle) noexcept
{
    handle.promise().continuation.resume();
    handle.destroy();
}

SendMsgCoro SendMsgCoroPromise::get_return_object() noexcept
{
    return {std::coroutine_handle<SendMsgCoroPromise>::from_promise(*this)};
}

void SendMsgCoroPromise::return_value(Message msg)
{
    actor->updateLastMsg(std::move(msg));
}

void SendMsgCoroAwaitable::await_suspend(std::coroutine_handle<> h) noexcept
{
    handle.promise().continuation = h;
    handle.resume();
}

Message SendMsgCoroAwaitable::await_resume() noexcept
{
    return handle.promise().actor->getLastMsg().value();
}
