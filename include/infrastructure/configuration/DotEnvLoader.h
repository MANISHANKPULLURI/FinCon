#pragma once

#include <string>
#include <unordered_map>

namespace fincon
{
    class DotEnvLoader
    {
    public:
        explicit DotEnvLoader(std::string filePath);

        std::string get(const std::string& key) const;
        bool contains(const std::string& key) const;

    private:
        std::unordered_map<std::string, std::string> values_;
    };
}