#include "StackMapTest.hpp"

#include "StackMap.hpp"
#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>

void StackTest::onRun() {
    Stack<int> stack;

    // Test empty stack
    try {
        stack.getTop();
        OATPP_ASSERT(false);
    } catch (StackEmpty) {
    }
    try {
        stack.pop();
        OATPP_ASSERT(false);
    } catch (StackEmpty) {
    }

    // Test push
    stack.push(1);
    OATPP_ASSERT(stack.getTop() == 1);
    stack.push(2);
    OATPP_ASSERT(stack.getTop() == 2);
    stack.push(3);
    OATPP_ASSERT(stack.getTop() == 3);
    stack.push(4);
    OATPP_ASSERT(stack.getTop() == 4);
    stack.push(5);
    OATPP_ASSERT(stack.getTop() == 5);

    // Test pop
    OATPP_ASSERT(stack.pop() == 5);
    OATPP_ASSERT(stack.getTop() == 4);
    OATPP_ASSERT(stack.pop() == 4);
    OATPP_ASSERT(stack.getTop() == 3);
    OATPP_ASSERT(stack.pop() == 3);
    OATPP_ASSERT(stack.getTop() == 2);
    OATPP_ASSERT(stack.pop() == 2);
    OATPP_ASSERT(stack.getTop() == 1);
    OATPP_ASSERT(stack.pop() == 1);
    try {
        stack.getTop();
        OATPP_ASSERT(false);
    } catch (StackEmpty) {
    }
}

void StackConcurrentTest::onRun() {
    Stack<int> stack;
    stack.push(1);
    stack.push(2);
    stack.push(3);

    std::vector<std::thread> threads;

    std::vector<int> popElements[3];
    std::mutex popElemMutexes[3];

    for (int i = 0; i < 3; ++i) {
        auto cloneThread = std::thread([&, i]() mutable {
            auto stk =
                Stack(stack); // `stack` is captured by reference to make it
                              // copies here, testing concurrent copying.
            auto &popElem = popElements[i];
            auto &popElemMu = popElemMutexes[i];

            std::vector<std::thread> threads;

            for (int j = 0; j < 3; ++j) {
                auto sharedThread = std::thread([&]() mutable {
                    stk.push(4);
                    stk.push(5);

                    while (true) {
                        try {
                            auto elem = stk.pop();
                            std::unique_lock lock(popElemMu);
                            popElem.push_back(std::move(elem));
                        } catch (StackEmpty) {
                            break;
                        }
                    }
                });
                threads.push_back(std::move(sharedThread));
            }

            for (auto &t : threads) {
                t.join();
            }
        });
        threads.push_back(std::move(cloneThread));
    }
    for (auto &t : threads) {
        t.join();
    }

    std::vector<int> expectedPop{1, 2, 3, 4, 4, 4, 5, 5, 5};
    for (int i = 0; i < 3; ++i) {
        auto &popElem = popElements[i];
        std::sort(popElem.begin(), popElem.end());
        OATPP_ASSERT(popElem == expectedPop);
    }
}

void StackMapConcurrentTest::onRun() {
    StackMap<std::string, int> stackMap;
    stackMap.create("default");
    {
        auto [lock, stk] = stackMap.getStack("default");
        stk.push(1);
        stk.push(2);
        stk.push(3);
    }
    std::vector<std::thread> threads;

    std::vector<int> popElements[3];
    std::mutex popElemMutexes[3];

    for (int i = 0; i < 3; ++i) {
        auto cloneThread = std::thread([&, i]() mutable {
            std::stringstream stkNameStream;
            stkNameStream << "stack-" << i;
            std::string stkName = stkNameStream.str();

            stackMap.copy("default", std::string(stkName));

            auto &popElem = popElements[i];
            auto &popElemMu = popElemMutexes[i];

            std::vector<std::thread> threads;

            for (int j = 0; j < 3; ++j) {
                auto sharedThread = std::thread([&]() mutable {
                    {
                        auto [lock, stk] = stackMap.getStack(stkName);
                        stk.push(4);
                        stk.push(5);
                    }

                    while (true) {
                        try {
                            auto elem = stackMap.getStack(stkName).second.pop();
                            std::unique_lock lock(popElemMu);
                            popElem.push_back(std::move(elem));
                        } catch (StackEmpty) {
                            break;
                        }
                    }
                });
                threads.push_back(std::move(sharedThread));
            }

            for (auto &t : threads) {
                t.join();
            }
        });
        threads.push_back(std::move(cloneThread));
    }
    for (auto &t : threads) {
        t.join();
    }

    std::vector<int> expectedPop{1, 2, 3, 4, 4, 4, 5, 5, 5};
    for (int i = 0; i < 3; ++i) {
        auto &popElem = popElements[i];
        std::sort(popElem.begin(), popElem.end());
        OATPP_ASSERT(popElem == expectedPop);
    }
}
