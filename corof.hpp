#pragma once
#include <coroutine>
#include <exception>

namespace corof
{
    class entrance
    {
        private:
            struct EntrancePromiseFinalAwaiter
            {
                bool await_ready  ()       const noexcept { return false; }
                void await_suspend(auto h) const noexcept { h.destroy() ; }
                void await_resume ()       const noexcept {}
            };

        public:
            struct promise_type
            {
                entrance get_return_object()
                {
                    return {std::coroutine_handle<promise_type>::from_promise(*this)};
                }

                std::suspend_always       initial_suspend() noexcept { return {}; }
                EntrancePromiseFinalAwaiter final_suspend() noexcept { return {}; }

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

namespace corof
{
    template<typename T> class awaitable
    {
        public:
            struct promise_type
            {
                struct AwaitablePromiseFinalAwaiter
                {
                    bool await_ready() const noexcept
                    {
                        return false;
                    }

                    void await_suspend(std::coroutine_handle<promise_type> handle) noexcept
                    {
                        handle.promise().continuation.resume();
                        handle.destroy();
                    }

                    void await_resume() const noexcept {}
                };

                std::optional<T> result;
                std::coroutine_handle<> continuation;

                awaitable get_return_object() noexcept
                {
                    return {std::coroutine_handle<promise_type>::from_promise(*this)};
                }

                void return_void() {}

                std::suspend_always        initial_suspend() const noexcept { return {}; }
                AwaitablePromiseFinalAwaiter final_suspend() const noexcept { return {}; }

                void unhandled_exception()
                {
                    std::rethrow_exception(std::current_exception());
                }
            };

        private:
            std::coroutine_handle<promise_type> m_handle;

        public:
            awaitable(std::coroutine_handle<promise_type> h)
                : m_handle(h)
            {}

        public:
            auto operator co_await() && noexcept
            {
                class awaiter
                {
                    private:
                        std::coroutine_handle<promise_type> m_handle;

                    public:
                        awaiter(std::coroutine_handle<promise_type> h)
                            : m_handle(h)
                        {}

                    public:
                        bool await_ready() const noexcept
                        {
                            return false;
                        }

                        void await_suspend(std::coroutine_handle<> h) noexcept
                        {
                            m_handle.promise().continuation = h;
                            m_handle.resume();
                        }

                        auto await_resume()
                        {
                            return m_handle.promise().result.value();
                        }
                };
                return awaiter{m_handle};
            }
    };
}
