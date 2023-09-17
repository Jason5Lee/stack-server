
#ifndef StackApiTestClient_hpp
#define StackApiTestClient_hpp

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/web/client/ApiClient.hpp"

/* Begin Api Client code generation */
#include OATPP_CODEGEN_BEGIN(ApiClient)

/**
 * Test API client.
 * Use this client to call application APIs.
 */
class StackApiTestClient : public oatpp::web::client::ApiClient {

    API_CLIENT_INIT(StackApiTestClient)

    API_CALL("GET", "/{name}/top", getTop, PATH(String, name))

    API_CALL("POST", "/{name}/push", push, PATH(String, name),
             BODY_STRING(String, body, "text/plain"))

    API_CALL("POST", "/{name}/pop", pop, PATH(String, name))

    API_CALL("POST", "/{name}", create, PATH(String, name))

    API_CALL("DELETE", "/{name}", remove, PATH(String, name))

    API_CALL("POST", "/{from}/copy", copy, PATH(String, from),
             QUERY(String, to))
};

/* End Api Client code generation */
#include OATPP_CODEGEN_END(ApiClient)

#endif // StackApiTestClient_hpp
