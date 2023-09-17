#ifndef StackControllerTest_hpp
#define StackControllerTest_hpp

#include "oatpp-test/UnitTest.hpp"

class StackControllerTest : public oatpp::test::UnitTest {
public:
    StackControllerTest() : UnitTest("TEST[StackControllerTest]") {}
    void onRun() override;
};

#endif // StackControllerTest_hpp
