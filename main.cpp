#include <ctime>
#include <cstdio>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <functional>
#include <unistd.h>

#include "message.hpp"
#include "threadpool.hpp"
#include "actor.hpp"
#include "utils.hpp"

std::unique_ptr<ThreadPool> pool;
std::vector<std::unique_ptr<Actor>> actors;

int main(int argc, char **argv)
{
    std::srand(std::time(nullptr));
    std::printf("Press any key to start, pid %ld\n\n", static_cast<long>(getpid()));
    std::getchar();

    const int threadNum = argc > 1 ? std::stoi(argv[1]) :  4;
    const int  actorNum = argc > 2 ? std::stoi(argv[2]) : 20;

    pool = std::make_unique<ThreadPool>(threadNum);

    for (int i = 0; i < actorNum; ++i) {
        actors.push_back(std::make_unique<Actor>(*pool, i + 1));
        pool->registerActor(actors.back().get());
    }

    std::getchar();

    pool.reset();
    actors.clear();

    std::printf("Gracefully exiting main\n");
    return 0;
}
