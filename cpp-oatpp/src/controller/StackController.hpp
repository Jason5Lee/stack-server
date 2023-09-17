#ifndef StackController_hpp
#define StackController_hpp

#include "StackMap.hpp"

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/web/server/api/ApiController.hpp"
#include <memory>

#include OATPP_CODEGEN_BEGIN(ApiController) //<-- Begin Codegen

/**
 * Sample Api Controller.
 */
class StackController : public oatpp::web::server::api::ApiController {
public:
    /**
     * Constructor with object mapper.
     * @param objectMapper - default object mapper used to serialize/deserialize
     * DTOs.
     */
    StackController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>,
                                    objectMapper))
        : oatpp::web::server::api::ApiController(objectMapper) {}

public:
    ENDPOINT("GET", "/{name}/top", getTop, PATH(String, name)) {
        return this->run([&]() mutable {
            auto result = this->map.getStack(name).second.getTop();
            return createResponse(Status::CODE_200, result);
        });
    }

    ENDPOINT("POST", "/{name}/push", push,
             BODY_STRING(String, body, "text/plain"), PATH(String, name)) {
        return this->run([&]() mutable {
            auto s = this->map.getStack(name);
            s.second.push(String(body));
            return createResponse(Status::CODE_204, "");
        });
    }

    ENDPOINT("POST", "/{name}/pop", pop, PATH(String, name)) {
        return this->run([&]() mutable {
            return createResponse(Status::CODE_200,
                                  this->map.getStack(name).second.pop());
        });
    }

    ENDPOINT("POST", "/{name}", create, PATH(String, name)) {
        return this->run([&]() mutable {
            this->map.create(String(name));
            return createResponse(Status::CODE_201, "");
        });
    }

    ENDPOINT("DELETE", "/{name}", remove, PATH(String, name)) {
        return this->run([&]() mutable {
            this->map.remove(String(name));
            return createResponse(Status::CODE_204, "");
        });
    }

    ENDPOINT("POST", "/{from}/copy", copy, PATH(String, from),
             QUERY(String, to)) {
        return this->run([&]() mutable {
            this->map.copy(from, String(to));
            return createResponse(Status::CODE_204, "");
        });
    }

private:
    StackMap<String, String> map;

    template <typename ApiImplFn>
    std::shared_ptr<OutgoingResponse> run(ApiImplFn apiImpl) {
        try {
            return apiImpl();
        } catch (StackNameAlreadyExists) {
            return createResponse(Status::CODE_409,
                                  "STACK_NAME_ALREADY_EXISTS");
        } catch (StackNameNotFound) {
            return createResponse(Status::CODE_404, "STACK_NAME_NOT_FOUND");
        } catch (StackEmpty) {
            return createResponse(Status::CODE_405, "STACK_EMPTY");
        }
    }
};

#include OATPP_CODEGEN_END(ApiController) //<-- End Codegen

#endif /* StackController_hpp */
