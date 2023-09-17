#ifndef StackMapTest_hpp
#define StackMapTest_hpp

#include "oatpp-test/UnitTest.hpp"

class StackTest : public oatpp::test::UnitTest {
public:
    StackTest() : UnitTest("TEST[StackTest]") {}
    void onRun() override;
};
class StackConcurrentTest : public oatpp::test::UnitTest {
public:
    StackConcurrentTest() : UnitTest("TEST[StackConcurrentTest]") {}
    void onRun() override;
};
class StackMapConcurrentTest : public oatpp::test::UnitTest {
public:
    StackMapConcurrentTest() : UnitTest("TEST[StackMapConcurrentTest]") {}
    void onRun() override;
};

#endif // StackMapTest_hpp
