#include <iostream>
#include <sstream>
#include <functional>
#include <vector>
#include "Logger.h"

using namespace std;

void func(int id, Logger& logger) {
    // do something else.
    for (size_t i = 0; i < 74; ++i) {
        stringstream ss;
        ss << "The No." << id << " thread's test for the " << i << " times to call Logger's log.";
        logger.log(ss.str());
    }
}

void Test() {
    Logger logger;
    vector<thread> myThreads;
    for (size_t i = 0; i < 10; ++i) {
        myThreads.emplace_back(func, i, ref(logger));
    }
    for (auto& aThread : myThreads) {
        aThread.join();
    }
}

int main()
{
    Test();
    return 0;
}
