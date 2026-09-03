#pragma once

#include "application/controller/FinanceControllerFacade.h"
#include "infrastructure/http/HttpServer.h"

#include <string>

namespace fincon
{
    class FinanceControllerApi
    {
    public:
        explicit FinanceControllerApi(
            FinanceControllerFacade& facade
        );

        void registerRoutes(
            HttpServer& server
        );

    private:
        HttpResponse health(
            const HttpRequest& request
        ) const;

        HttpResponse investigate(
            const HttpRequest& request
        ) const;

        HttpResponse audit(
            const HttpRequest& request
        ) const;

        FinanceControllerFacade& facade_;
    };
}