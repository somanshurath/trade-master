#include "../WebSocketHandler.h"

void WebSocketHandler::FetchAccountSummary()
{
    if (!authenticated)
    {
        std::cerr << "Cannot fetch account summary: not authenticated" << std::endl;
        return;
    }

    nlohmann::json params = {
        {"currency", "BTC"},
        {"subaccount_id", this->user_id},
        {"extended", false}};

    SendRequest("private/get_account_summary", params, [this](const nlohmann::json &result)
                {   std::lock_guard<std::mutex> lock(m_account_summary_mutex);
                    m_account_summary = result;
                    m_account_summary_req_time = std::chrono::system_clock::now(); });
}

void WebSocketHandler::FetchAccessLog()
{
    if (!authenticated)
    {
        std::cerr << "Cannot fetch access log: not authenticated" << std::endl;
        return;
    }

    nlohmann::json params = {
        {"count", 5}};

    SendRequest("private/get_access_log", params, [this](const nlohmann::json &result)
                {   std::lock_guard<std::mutex> lock(m_access_log_mutex);
                    m_access_log = result;
                    m_access_log_req_time = std::chrono::system_clock::now(); });
}

void WebSocketHandler::FetchPositions()
{
    if (!authenticated)
    {
        std::cerr << "Cannot fetch positions: not authenticated" << std::endl;
        return;
    }

    nlohmann::json params = {};

    SendRequest("private/get_positions", params, [this](const nlohmann::json &result)
                {   std::lock_guard<std::mutex> lock(m_positions_mutex);
                    m_positions = result;
                    m_positions_req_time = std::chrono::system_clock::now(); });
}
