#pragma once
#include <optional>
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
                    return entrance(std::coroutine_handle<promise_type>::from_promise(*this));
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
            explicit entrance(std::coroutine_handle<promise_type> h)
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
    namespace _details
    {
        struct awaitable_promise_with_void
        {
            void return_void() {}
        };

        template<typename T> struct awaitable_promise_with_type
        {
            std::optional<T> m_result;
            void return_value(T t)
            {
                m_result = std::move(t);
            }
        };
    }

    template<typename T = void> class [[nodiscard]] awaitable
    {
        private:
            class AwaitableAsAwaiter;

        public:
            class promise_type: public std::conditional_t<std::is_void_v<T>, _details::awaitable_promise_with_void, _details::awaitable_promise_with_type<T>>
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
                    std::coroutine_handle<> m_continuation;

                public:
                    awaitable get_return_object() noexcept
                    {
                        return awaitable(std::coroutine_handle<promise_type>::from_promise(*this));
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
                    explicit AwaitableAsAwaiter(std::coroutine_handle<promise_type> h)
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
                        if constexpr(std::is_void_v<T>){
                            return;
                        }
                        else{
                            return m_handle.promise().m_result.value();
                        }
                    }
            };

        private:
            std::coroutine_handle<promise_type> m_handle;

        public:
            explicit awaitable(std::coroutine_handle<awaitable::promise_type> h)
                : m_handle(h)
            {}

        public:
            awaitable(awaitable && other) noexcept
            {
                std::swap(m_handle, other.m_handle);
            }

        public:
            awaitable & operator = (awaitable && other) noexcept
            {
                std::swap(m_handle, other.m_handle);
                return *this;
            }

        public:
            awaitable              (const awaitable &) = delete;
            awaitable & operator = (const awaitable &) = delete;

        public:
            auto operator co_await() && noexcept
            {
                return AwaitableAsAwaiter(m_handle);
            }
    };
}
