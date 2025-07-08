#include "crow/app.h"
#include "utils/env.utils.hpp"

int main()
{
    Utils::LoadEnvFile();

    const auto PORT = std::stoi(Utils::GetEnv("PORT", "8000"));

    crow::SimpleApp app;

    CROW_ROUTE(app, "/")
    ([] { return "Hello world!"; });

    app.port(PORT).multithreaded().run();

    return 0;
}
