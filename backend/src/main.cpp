#include "crow/app.h"
#include "routes/health.hpp"
#include "utils/env.utils.hpp"

int main()
{
    Utils::LoadEnvFile();

    const auto PORT = std::stoi(Utils::GetEnv("PORT", "8000"));

    crow::SimpleApp app;

    HealthAPI::RegisterRoutes(app);

    app.port(PORT).multithreaded().run();

    return 0;
}
