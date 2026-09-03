#include "infrastructure/investigation/LibcurlHttpClient.h"

#include <curl/curl.h>

#include <stdexcept>
#include <string>

namespace fincon
{
    namespace
    {
        std::size_t writeCallback(
            char* data,
            std::size_t size,
            std::size_t count,
            void* userData)
        {
            const std::size_t totalSize = size * count;
            auto* response = static_cast<std::string*>(userData);
            response->append(data, totalSize);
            return totalSize;
        }
    }

    std::string LibcurlHttpClient::post(
        const std::string& url,
        const std::string& body,
        const std::string& authorization) const
    {
        CURL* curl = curl_easy_init();

        if (curl == nullptr)
            throw std::runtime_error("Failed to initialize libcurl");

        std::string response;

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(
            headers,
            "Content-Type: application/json"
        );

        if (url.find("anthropic.com") != std::string::npos)
        {
            std::string key = authorization;
            if (key.rfind("Bearer ", 0) == 0) key = key.substr(7);
            headers = curl_slist_append(
                headers,
                ("x-api-key: " + key).c_str()
            );
            headers = curl_slist_append(
                headers,
                "anthropic-version: 2023-06-01"
            );
        }
        else
        {
            std::string auth = authorization;
            if (auth.rfind("Bearer ", 0) != 0 && auth.rfind("x-api-key", 0) != 0)
                auth = "Bearer " + auth;
            headers = curl_slist_append(
                headers,
                ("Authorization: " + auth).c_str()
            );
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body.size());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

        const CURLcode result = curl_easy_perform(curl);

        long statusCode = 0;
        curl_easy_getinfo(
            curl,
            CURLINFO_RESPONSE_CODE,
            &statusCode
        );

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (result != CURLE_OK)
            throw std::runtime_error(curl_easy_strerror(result));

        if (statusCode < 200 || statusCode >= 300)
            throw std::runtime_error(
                "HTTP request failed with status " +
                std::to_string(statusCode)
            );

        return response;
    }
}