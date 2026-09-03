#pragma once

#include "application/controller/FinanceControllerFacade.h"
#include "application/ingestion/FinancialFieldMapper.h"
#include "application/ingestion/MessageQueue.h"
#include "infrastructure/http/HttpServer.h"

#include <string>

namespace fincon
{
    class FinanceControllerApi
    {
    public:
        explicit FinanceControllerApi(
            FinanceControllerFacade& facade,
            MessageQueue<FinancialDataBatch>& queue,
            FinanceControllerState& state
        );

        void registerRoutes(
            HttpServer& server
        );

    private:
        HttpResponse health(
            const HttpRequest& request
        ) const;

        HttpResponse dashboard(
            const HttpRequest& request
        ) const;

        HttpResponse incidents(
            const HttpRequest& request
        ) const;

        HttpResponse investigations(
            const HttpRequest& request
        ) const;

        HttpResponse investigation(
            const HttpRequest& request
        ) const;

        HttpResponse investigate(
            const HttpRequest& request
        ) const;

        HttpResponse ingest(
            const HttpRequest& request
        ) const;

        HttpResponse audit(
            const HttpRequest& request
        ) const;

        HttpResponse events(
            const HttpRequest& request
        ) const;

        HttpResponse evaluation(
            const HttpRequest& request
        ) const;

        FinanceControllerFacade& facade_;
        MessageQueue<FinancialDataBatch>& queue_;
        FinanceControllerState& state_;
        FinancialFieldMapper mapper_;
    };
}