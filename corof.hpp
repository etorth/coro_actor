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
        private:
            class AwaitableAsAwaiter;

        public:
            class promise_type
            {
                private:
                    friend class AwaitableAsAwaiter;

                private:
                    struct AwaitablePromiseFinalAwaiter
                    {
                        bool await_ready() const noexcept
                        {
                            return false;
                        }

                        void await_suspend(std::coroutine_handle<promise_type> handle) noexcept
                        {
                            handle.promise().m_continuation.resume();
                            handle.destroy();
                        }

                        void await_resume() const noexcept {}
                    };

                private:
                    std::optional<T> m_result;
                    std::coroutine_handle<> m_continuation;

                public:
                    awaitable get_return_object() noexcept
                    {
                        return {std::coroutine_handle<promise_type>::from_promise(*this)};
                    }

                    void return_value(T t)
                    {
                        m_result = std::move(t);
                    }

                    std::suspend_always        initial_suspend() const noexcept { return {}; }
                    AwaitablePromiseFinalAwaiter final_suspend() const noexcept { return {}; }

                    void unhandled_exception()
                    {
                        std::rethrow_exception(std::current_exception());
                    }
            };

        private:
            class AwaitableAsAwaiter
            {
                private:
                    std::coroutine_handle<promise_type> m_handle;

                public:
                    AwaitableAsAwaiter(std::coroutine_handle<promise_type> h)
                        : m_handle(h)
                    {}

                public:
                    bool await_ready() const noexcept
                    {
                        return false;
                    }

                    void await_suspend(std::coroutine_handle<> h) noexcept
                    {
                        m_handle.promise().m_continuation = h;
                        m_handle.resume();
                    }

                    auto await_resume()
                    {
                        return m_handle.promise().m_result.value();
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
                return AwaitableAsAwaiter(m_handle);
            }
    };
}
