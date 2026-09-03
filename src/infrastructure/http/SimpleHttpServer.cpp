#include "infrastructure/http/SimpleHttpServer.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <iostream>



namespace fincon
{
    namespace
    {
        std::string trim(const std::string& value)
        {
            const std::size_t first =
                value.find_first_not_of(" \t\r\n");

            if (first == std::string::npos)
                return {};

            const std::size_t last =
                value.find_last_not_of(" \t\r\n");

            return value.substr(
                first,
                last - first + 1
            );
        }

        std::string statusText(int statusCode)
        {
            switch (statusCode)
            {
            case 200:
                return "OK";

            case 201:
                return "Created";

            case 400:
                return "Bad Request";

            case 404:
                return "Not Found";

            case 405:
                return "Method Not Allowed";

            case 500:
                return "Internal Server Error";

            default:
                return "Unknown";
            }
        }

        std::string toLower(std::string s)
        {
            for (char &c : s) c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));
            return s;
        }

        std::string headerValue(
            const std::string& header,
            const std::string& name)
        {
            std::istringstream stream(header);
            std::string line;
            const std::string lowerName = toLower(name) + ":";

            while (std::getline(stream, line))
            {
                std::string lowerLine = toLower(line);
                if (lowerLine.starts_with(lowerName))
                    return trim(line.substr(lowerName.size()));
            }

            return {};
        }

        bool receiveRequest(
            int clientSocket,
            std::string& requestData
        )
        {
            char buffer[4096];
            std::size_t headerEnd = std::string::npos;
            std::size_t contentLength = 0;

            while (true)
            {
                headerEnd = requestData.find("\r\n\r\n");
                if (headerEnd != std::string::npos)
                {
                    std::string header = requestData.substr(0, headerEnd);
                    std::string cl = headerValue(header, "Content-Length");
                    if (!cl.empty())
                    {
                        try { contentLength = std::stoull(cl); } catch (...) { return false; }
                        std::size_t bodyStart = headerEnd + 4;
                        std::size_t haveBody = requestData.size() > bodyStart ? requestData.size() - bodyStart : 0;
                        if (haveBody >= contentLength)
                            return true;
                    }
                    else
                    {
                        return true;
                    }
                }
                if (requestData.size() > 1024 * 1024 + 4096)
                    return false;
                ssize_t received = recv(clientSocket, buffer, sizeof(buffer), 0);
                if (received <= 0)
                    return headerEnd != std::string::npos;
                requestData.append(buffer, static_cast<std::size_t>(received));
                if (requestData.size() > 1024 * 1024 + 4096)
                    return false;
            }
        }

        HttpRequest parseRequest(
            const std::string& requestData
        )
        {
            const std::size_t headerEnd =
                requestData.find("\r\n\r\n");

            const std::string header =
                requestData.substr(
                    0,
                    headerEnd
                );

            const std::string body =
                requestData.substr(headerEnd + 4);

            std::istringstream stream(header);

            std::string requestLine;

            std::getline(
                stream,
                requestLine
            );

            requestLine = trim(requestLine);

            std::istringstream requestLineStream(
                requestLine
            );

            HttpRequest request;

            requestLineStream >>
                request.method >>
                request.path;

            const std::string contentLength =
                headerValue(header, "Content-Length");
            if (!contentLength.empty())
            {
                const std::size_t length =
                    std::stoull(contentLength);
                if (length > 1024 * 1024 || body.size() < length)
                    throw std::invalid_argument("Invalid request body length.");
                request.body = body.substr(0, length);
            }
            else
            {
                request.body = body;
            }

            return request;
        }

        void sendResponse(
            int clientSocket,
            const HttpResponse& response
        )
        {
            std::ostringstream output;

            output
                << "HTTP/1.1 "
                << response.statusCode
                << " "
                << statusText(response.statusCode)
                << "\r\n";

            output
                << "Content-Type: "
                << response.contentType
                << "\r\n";

            if (!response.keepAlive)
            {
                output
                    << "Content-Length: "
                    << response.body.size()
                    << "\r\n";
            }

            output
                << "Access-Control-Allow-Origin: "
                << response.accessControlAllowOrigin
                << "\r\n";

            output
                << "Connection: "
                << (response.keepAlive ? "keep-alive" : "close")
                << "\r\n";

            output << "\r\n";

            output << response.body;

            const std::string data =
                output.str();

            std::size_t sentTotal = 0;

            while (sentTotal < data.size())
            {
                const ssize_t sent =
                    send(
                        clientSocket,
                        data.data() + sentTotal,
                        data.size() - sentTotal,
                        0
                    );

                if (sent <= 0)
                    break;

                sentTotal +=
                    static_cast<std::size_t>(sent);
            }
        }
    }

    SimpleHttpServer::~SimpleHttpServer()
    {
        stop();
    }

    void SimpleHttpServer::registerHandler(
        std::string method,
        std::string path,
        HttpHandler handler
    )
    {
        if (method.empty())
            throw std::invalid_argument(
                "HTTP method cannot be empty."
            );

        if (path.empty())
            throw std::invalid_argument(
                "HTTP path cannot be empty."
            );

        if (!handler)
            throw std::invalid_argument(
                "HTTP handler cannot be empty."
            );

        std::lock_guard<std::mutex> lock(mutex_);

        const std::string key =
            makeRouteKey(
                method,
                path
            );

        routes_[key] = Route{
            std::move(method),
            std::move(path),
            std::move(handler)
        };
    }

    void SimpleHttpServer::start(int port)
    {
        if (port <= 0 || port > 65535)
        {
            throw std::invalid_argument(
                "Invalid HTTP server port."
            );
        }

        bool expected = false;

        if (!running_.compare_exchange_strong(
                expected,
                true))
        {
            return;
        }

        serverThread_ =
            std::thread(
                &SimpleHttpServer::run,
                this,
                port
            );
    }

    void SimpleHttpServer::stop()
    {
        bool expected = true;

        if (!running_.compare_exchange_strong(
                expected,
                false))
        {
            if (serverThread_.joinable())
                serverThread_.join();

            return;
        }

        const int socket =
            serverSocket_;

        if (socket != -1)
        {
            shutdown(
                socket,
                SHUT_RDWR
            );

            close(
                socket
            );

            serverSocket_ = -1;
        }

        if (serverThread_.joinable())
            serverThread_.join();

        {
            std::lock_guard<std::mutex> lock(clientMutex_);
            clientThreads_.clear();
        }
    }

    std::string SimpleHttpServer::makeRouteKey(
        const std::string& method,
        const std::string& path
    ) const
    {
        return method + " " + path;
    }

    HttpResponse SimpleHttpServer::handleRequest(
        const HttpRequest& request
    ) const
    {
        HttpHandler handler;

        {
            std::lock_guard<std::mutex> lock(mutex_);

            const std::string key =
                makeRouteKey(
                    request.method,
                    request.path
                );

            auto iterator =
                routes_.find(key);

            if (iterator == routes_.end())
            {
                std::size_t bestSuffixLength = 0;
                for (auto candidate = routes_.begin();
                     candidate != routes_.end();
                     ++candidate)
                {
                    const std::string marker = "{id}";
                    const std::size_t markerPosition =
                        candidate->second.path.find(marker);
                    const std::string routePrefix =
                        candidate->second.path.substr(
                            0,
                            markerPosition
                        );
                    const std::string routeSuffix =
                        candidate->second.path.substr(
                            markerPosition + marker.size()
                        );
                    const bool hasPrefix =
                        request.path.starts_with(routePrefix);
                    const bool hasSuffix =
                        routeSuffix.empty() || request.path.ends_with(routeSuffix);
                    const std::size_t idStart = routePrefix.size();
                    const std::size_t idEnd =
                        routeSuffix.empty()
                            ? request.path.size()
                            : request.path.size() - routeSuffix.size();

                    if (candidate->second.method == request.method &&
                        markerPosition != std::string::npos &&
                        hasPrefix && hasSuffix && idEnd > idStart &&
                        routeSuffix.size() >= bestSuffixLength)
                    {
                        iterator = candidate;
                        bestSuffixLength = routeSuffix.size();
                    }
                }

                if (iterator == routes_.end())
                {
                    return HttpResponse{
                        404,
                        "application/json",
                        R"({"error":"Route not found"})"
                    };
                }
            }

            handler =
                iterator->second.handler;
        }

        try
        {
            return handler(request);
        }
        catch (const std::exception&)
        {
            return HttpResponse{
                500,
                "application/json",
                std::string(R"({"error":")") +
                "Internal server error." +
                R"("})"
            };
        }
        catch (...)
        {
            return HttpResponse{
                500,
                "application/json",
                R"({"error":"Internal server error"})"
            };
        }
    }

    void SimpleHttpServer::run(int port)
    {
        const int socketFd =
            socket(
                AF_INET,
                SOCK_STREAM,
                0
            );

        if (socketFd < 0)
        {
            std::cerr
                << "HTTP server socket failed: "
                << std::strerror(errno)
                << '\n';

            running_ = false;

            return;
        }

        serverSocket_ =
            socketFd;

        int reuseAddress = 1;

        if (setsockopt(
                serverSocket_,
                SOL_SOCKET,
                SO_REUSEADDR,
                &reuseAddress,
                sizeof(reuseAddress)
            ) < 0)
        {
            std::cerr
                << "HTTP server setsockopt failed: "
                << std::strerror(errno)
                << '\n';
        }

        sockaddr_in address{};

        address.sin_family =
            AF_INET;

        address.sin_addr.s_addr =
            htonl(INADDR_ANY);

        address.sin_port =
            htons(
                static_cast<std::uint16_t>(port)
            );

        if (bind(
                serverSocket_,
                reinterpret_cast<sockaddr*>(&address),
                sizeof(address)
            ) < 0)
        {
            std::cerr
                << "HTTP server bind failed on port "
                << port
                << ": "
                << std::strerror(errno)
                << '\n';

            close(serverSocket_);

            serverSocket_ = -1;

            running_ = false;

            return;
        }

        if (listen(
                serverSocket_,
                128
            ) < 0)
        {
            std::cerr
                << "HTTP server listen failed: "
                << std::strerror(errno)
                << '\n';

            close(serverSocket_);

            serverSocket_ = -1;

            running_ = false;

            return;
        }

        std::cout
            << "HTTP server listening on 0.0.0.0:"
            << port
            << '\n';

        while (running_)
        {
            sockaddr_in clientAddress{};

            socklen_t clientAddressLength =
                sizeof(clientAddress);

            const int clientSocket =
                accept(
                    serverSocket_,
                    reinterpret_cast<sockaddr*>(
                        &clientAddress
                    ),
                    &clientAddressLength
                );

            if (clientSocket < 0)
            {
                if (!running_)
                    break;

                std::cerr
                    << "HTTP accept failed: "
                    << std::strerror(errno)
                    << '\n';

                continue;
            }

            {
                std::lock_guard<std::mutex> lock(clientMutex_);
                std::vector<std::thread> stillAlive;
                stillAlive.reserve(clientThreads_.size());
                for (auto &t : clientThreads_)
                {
                    if (t.joinable())
                        stillAlive.push_back(std::move(t));
                }
                clientThreads_.swap(stillAlive);
                if (clientThreads_.size() >= kMaxClients)
                {
                    HttpResponse r{503, "application/json", R"({"error":"Too many connections"})"};
                    sendResponse(clientSocket, r);
                    close(clientSocket);
                    continue;
                }
                clientThreads_.emplace_back(
                    &SimpleHttpServer::handleClient,
                    this,
                    clientSocket
                );
                if (clientThreads_.back().joinable())
                    clientThreads_.back().detach();
            }
        }

        if (serverSocket_ != -1)
        {
            close(
                serverSocket_
            );

            serverSocket_ = -1;
        }
    }

    void SimpleHttpServer::handleClient(int clientSocket)
    {
        try
        {
            std::string requestData;
            if (receiveRequest(clientSocket, requestData))
            {
                const HttpRequest request = parseRequest(requestData);
                HttpResponse response = handleRequest(request);
                sendResponse(clientSocket, response);

                if (response.keepAlive)
                {
                    while (running_)
                    {
                        std::this_thread::sleep_for(
                            std::chrono::seconds(1)
                        );

                        if (!running_)
                            break;

                        response = handleRequest(request);
                        const std::string event = response.body;
                        const ssize_t sent = send(
                            clientSocket,
                            event.data(),
                            event.size(),
                            MSG_NOSIGNAL
                        );

                        if (sent <= 0)
                            break;
                    }
                }
            }
        }
        catch (...)
        {
            sendResponse(
                clientSocket,
                HttpResponse{
                    400,
                    "application/json",
                    R"({"error":{"code":"BAD_REQUEST","message":"Malformed request."}})"
                }
            );
        }

        shutdown(clientSocket, SHUT_RDWR);
        close(clientSocket);
    }
}