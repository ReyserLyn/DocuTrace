#pragma once

#include <string>
#include <string_view>

namespace Env
{
    std::string GetEnv(std::string_view varName, std::string_view defaultValue);
    void LoadEnvFile(const std::string& filename = ".env");
} // namespace Env
