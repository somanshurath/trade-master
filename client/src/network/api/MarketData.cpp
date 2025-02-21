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
}
