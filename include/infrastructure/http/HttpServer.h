#pragma once

#include <functional>
#include <string>

namespace fincon
{
    struct HttpRequest
    {
        std::string method;
        std::string path;
        std::string body;
    };

    struct HttpResponse
    {
        int statusCode = 200;
        std::string contentType = "application/json";
        std::string body;
        std::string accessControlAllowOrigin = "http://127.0.0.1:5173";
        bool keepAlive = false;
    };

    using HttpHandler =
        std::function<HttpResponse(const HttpRequest&)>;

    class HttpServer
    {
    public:
        virtual ~HttpServer() = default;

        virtual void registerHandler(
            std::string method,
            std::string path,
            HttpHandler handler
        ) = 0;

        virtual void start(
            int port
        ) = 0;

        virtual void stop() = 0;
    };
}