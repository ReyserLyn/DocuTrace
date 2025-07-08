#include "routes/health.hpp"

namespace HealthAPI
{

    void RegisterRoutes(crow::SimpleApp& app)
    {
        CROW_ROUTE(app, "/ping").methods(crow::HTTPMethod::GET)([] { return "pong"; });
    }

} // namespace HealthAPI
