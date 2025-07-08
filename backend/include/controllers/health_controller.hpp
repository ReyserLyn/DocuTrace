#pragma once

#include "crow/app.h"
#include "crow/middlewares/cors.h"

namespace DocuTrace::Controllers
{
    class HealthController
    {
      public:
        HealthController() = default;
        ~HealthController() = default;

        // No copyable pero movible
        HealthController(const HealthController&) = delete;
        HealthController& operator=(const HealthController&) = delete;
        HealthController(HealthController&&) = default;
        HealthController& operator=(HealthController&&) = default;

        /**
         * @brief Registra todas las rutas de salud en la aplicación
         * @param app Aplicación Crow donde registrar las rutas
         */
        void RegisterRoutes(crow::App<crow::CORSHandler>& app);

        /**
         * @brief Handler para endpoint básico de salud
         * @return Respuesta HTTP con estado de salud
         */
        static crow::response HandleHealth(const crow::request& req);

        /**
         * @brief Handler para endpoint detallado de salud de API
         * @return Respuesta HTTP con información detallada del servicio
         */
        static crow::response HandleApiHealth(const crow::request& req);
    };

} // namespace DocuTrace::Controllers