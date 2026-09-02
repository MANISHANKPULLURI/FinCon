#include "infrastructure/configuration/DotEnvLoader.h"

#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace fincon
{
    namespace
    {
        std::string trim(const std::string& value)
        {
            const std::size_t first = value.find_first_not_of(" \t\r\n");

            if (first == std::string::npos)
                return {};

            const std::size_t last = value.find_last_not_of(" \t\r\n");

            return value.substr(first, last - first + 1);
        }
    }

    DotEnvLoader::DotEnvLoader(std::string filePath)
    {
        std::ifstream file(std::move(filePath));

        if (!file.is_open())
            throw std::runtime_error("Failed to open .env file");

        std::string line;

        while (std::getline(file, line))
        {
            line = trim(line);

            if (line.empty() || line.front() == '#')
                continue;

            const std::size_t separator = line.find('=');

            if (separator == std::string::npos)
                continue;

            const std::string key =
                trim(line.substr(0, separator));

            std::string value =
                trim(line.substr(separator + 1));

            if (value.size() >= 2 &&
                ((value.front() == '"' && value.back() == '"') ||
                 (value.front() == '\'' && value.back() == '\'')))
            {
                value = value.substr(1, value.size() - 2);
            }

            if (!key.empty())
                values_[key] = value;
        }
    }

    std::string DotEnvLoader::get(const std::string& key) const
    {
        const auto iterator = values_.find(key);

        if (iterator == values_.end())
            throw std::runtime_error(
                "Missing environment configuration: " + key
            );

        return iterator->second;
    }

    bool DotEnvLoader::contains(const std::string& key) const
    {
        return values_.find(key) != values_.end();
    }
}