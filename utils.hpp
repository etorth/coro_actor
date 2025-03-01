#pragma once
#include <cstdlib>
#include <cstdarg>
#include <mutex>
#include <string>
#include <chrono>

inline int randint()
{
    static std::mutex lock;
    const std::lock_guard<std::mutex> lockGuard(lock);
    return std::rand();
}

inline std::string str_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    char buffer[1024];
    std::vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    return std::string(buffer);
}

inline std::string randstr(size_t length)
{
    std::string s;
    s.reserve(length);

    for(size_t i = 0; i < length; ++i){
        s.push_back('a' + randint() % 26);
    }
    return s;
}

inline void printMessage(const char * [[maybe_unused]] format, ...)
{
#ifndef DISABLE_PRINT
    static std::mutex coutMutex;
    const  std::lock_guard<std::mutex> lock(coutMutex);
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    fflush(stdout); // Flush the output buffer
#endif
}
