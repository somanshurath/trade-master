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
