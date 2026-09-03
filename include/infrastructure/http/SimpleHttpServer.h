#pragma once

#include "infrastructure/http/HttpServer.h"

#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

namespace fincon
{
    class SimpleHttpServer final : public HttpServer
    {
    public:
        SimpleHttpServer() = default;

        ~SimpleHttpServer() override;

        void registerHandler(
            std::string method,
            std::string path,
            HttpHandler handler
        ) override;

        void start(
            int port
        ) override;

        void stop() override;

    private:
        struct Route
        {
            std::string method;
            std::string path;
            HttpHandler handler;
        };

        std::string makeRouteKey(
            const std::string& method,
            const std::string& path
        ) const;

        HttpResponse handleRequest(
            const HttpRequest& request
        ) const;

        void run(
            int port
        );

        std::unordered_map<std::string, Route> routes_;

        mutable std::mutex mutex_;

        std::atomic<bool> running_{false};

        std::thread serverThread_;

        int serverSocket_ = -1;
    };
}