#include "../../network/WebSocketHandler.h"

void WebSocketHandler::FetchBookSummary(const std::string &symbol, const std::string &kind)
{
    if (!authenticated)
    {
        std::cerr << "Cannot fetch account summary: not authenticated" << std::endl;
        return;
    }
    nlohmann::json params;
    if (kind != "")
        params = {
            {"currency", symbol},
            {"kind", kind}};
    else
        params = {
            {"currency", symbol}};

    SendRequest("public/get_book_summary_by_currency", params, [this, symbol, kind](const nlohmann::json &result)
                {   std::lock_guard<std::mutex> lock(m_book_summary_mutex);
                    if (kind != "")
                    {
                        m_book_summary[symbol][kind] = result;
                    }
                    else
                    {
                        m_book_summary[symbol]["all"] = result;
                    }
                    m_book_summary_req_time = std::chrono::system_clock::now(); });
};

// instrument, start_timestamp and end_timestamp in UNIX epoch time and resolution
void WebSocketHandler::FetchChartData(const std::string &instrument, const time_t &start_timestamp, const time_t &end_timestamp, const std::string &resolution)
{
    if (!authenticated)
    {
        std::cerr << "Cannot fetch chart data: not authenticated" << std::endl;
        return;
    }

    nlohmann::json params = {
        {"instrument_name", instrument},
        {"start_timestamp", start_timestamp},
        {"end_timestamp", end_timestamp},
        {"resolution", resolution}};

    SendRequest("public/get_tradingview_chart_data", params, [this](const nlohmann::json &result)
                {   std::lock_guard<std::mutex> lock(m_chart_data_mutex);
                    m_chart_data = result;
                    m_chart_data_req_time = std::chrono::system_clock::now(); });
}
