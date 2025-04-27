#pragma once
#include <coroutine>
#include <exception>

namespace corof
{
    class entrance
    {
        public:
            struct EntranceFinalAwaiter
            {
                bool await_ready  ()       const noexcept { return false; }
                void await_suspend(auto h) const noexcept { h.destroy() ; }
                void await_resume ()       const noexcept {}
            };

            struct promise_type
            {
                entrance get_return_object()
                {
                    return {std::coroutine_handle<promise_type>::from_promise(*this)};
                }

                std::suspend_always initial_suspend() noexcept { return {}; }
                EntranceFinalAwaiter  final_suspend() noexcept { return {}; }

                void return_void() {}
                void unhandled_exception()
                {
                    std::rethrow_exception(std::current_exception());
                }
            };

        private:
            std::coroutine_handle<promise_type> m_handle;

        public:
            entrance() = default;

        private:
            entrance(std::coroutine_handle<promise_type> h)
                : m_handle(h)
            {}

        public:
            void operator()() &&
            {
                if(m_handle){
                    m_handle.resume();
                }
            }
    };
}
