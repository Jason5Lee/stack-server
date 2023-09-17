#include "StackControllerTest.hpp"

#include "controller/StackController.hpp"

#include "app/StackApiTestClient.hpp"
#include "app/TestComponent.hpp"

#include "oatpp/web/client/HttpRequestExecutor.hpp"

#include "oatpp-test/web/ClientServerTestRunner.hpp"
#include <oatpp/core/base/Environment.hpp>

#include <sstream>

void StackControllerTest::onRun() {

    /* Register test components */
    TestComponent component;

    /* Create client-server test runner */
    oatpp::test::web::ClientServerTestRunner runner;

    /* Add StackController endpoints to the router of the test server */
    runner.addController(std::make_shared<StackController>());

    /* Run test */
    runner.run(
        [this, &runner] {
            /* Get client connection provider for Api Client */
            OATPP_COMPONENT(
                std::shared_ptr<oatpp::network::ClientConnectionProvider>,
                clientConnectionProvider);

            /* Get object mapper component */
            OATPP_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>,
                            objectMapper);

            /* Create http request executor for Api Client */
            auto requestExecutor =
                oatpp::web::client::HttpRequestExecutor::createShared(
                    clientConnectionProvider);

            /* Create Test API client */
            auto client =
                StackApiTestClient::createShared(requestExecutor, objectMapper);

            /* Test not found */
            OATPP_ASSERT(client->getTop("not-exists")->getStatusCode() == 404);
            OATPP_ASSERT(
                client->push("not-exists", "content")->getStatusCode() == 404);
            OATPP_ASSERT(client->pop("not-exists")->getStatusCode() == 404);
            OATPP_ASSERT(client->remove("not-exists")->getStatusCode() == 404);
            OATPP_ASSERT(client->copy("not-exists", "new")->getStatusCode() ==
                         404);

            /* Test confliction */
            OATPP_ASSERT(client->create("stack")->getStatusCode() == 201);
            OATPP_ASSERT(client->create("stack")->getStatusCode() == 409);
            OATPP_ASSERT(client->create("new-stack")->getStatusCode() == 201);
            OATPP_ASSERT(client->copy("stack", "new-stack")->getStatusCode() ==
                         409);

            /* Concurrent Test */
            OATPP_ASSERT(client->push("stack", "1")->getStatusCode() == 204);
            OATPP_ASSERT(client->push("stack", "2")->getStatusCode() == 204);
            OATPP_ASSERT(client->push("stack", "3")->getStatusCode() == 204);

            std::vector<std::thread> threads;

            std::vector<std::string> popElements[3];
            std::mutex popElemMutexes[3];

            for (int i = 0; i < 3; ++i) {
                auto cloneThread = std::thread([&, i]() mutable {
                    std::stringstream stkNameStream;
                    stkNameStream << "stack-" << i;
                    std::string stkName = stkNameStream.str();

                    OATPP_ASSERT(
                        client->copy("stack", stkName)->getStatusCode() == 204);

                    auto &popElem = popElements[i];
                    auto &popElemMu = popElemMutexes[i];

                    std::vector<std::thread> threads;

                    for (int j = 0; j < 3; ++j) {
                        auto sharedThread = std::thread([&]() mutable {
                            {
                                OATPP_ASSERT(client->push(stkName, "4")
                                                 ->getStatusCode() == 204);
                                OATPP_ASSERT(client->push(stkName, "5")
                                                 ->getStatusCode() == 204);
                            }

                            while (true) {
                                auto resp = client->pop(stkName);
                                if (resp->getStatusCode() == 405) {
                                    break;
                                }
                                OATPP_ASSERT(resp->getStatusCode() == 200);

                                auto elem = resp->readBodyToString();
                                std::unique_lock lock(popElemMu);
                                popElem.push_back(std::move(elem));
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

            std::vector<std::string> expectedPop{"1", "2", "3", "4", "4",
                                                 "4", "5", "5", "5"};
            for (int i = 0; i < 3; ++i) {
                auto &popElem = popElements[i];
                std::sort(popElem.begin(), popElem.end());
                OATPP_ASSERT(popElem == expectedPop);
            }
        },
        std::chrono::minutes(10) /* test timeout */);

    /* wait all server threads finished */
    std::this_thread::sleep_for(std::chrono::seconds(1));
}
