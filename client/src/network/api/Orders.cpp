#include "../WebSocketHandler.h"

void WebSocketHandler::FetchOpenOrders()
{
    if (!authenticated)
    {
        std::cerr << "Cannot fetch account summary: not authenticated" << std::endl;
        return;
    }

    nlohmann::json params = {};

    SendRequest("private/get_open_orders", params, [this](const nlohmann::json &result)
                {   std::lock_guard<std::mutex> lock(m_open_orders_mutex);
                    m_open_orders = result;
                    m_open_orders_req_time = std::chrono::system_clock::now(); });
}

void WebSocketHandler::PlaceOrder(const std::string &instrument_name, const std::string &direction, const std::string &order_type, const std::string &amount, const std::string &contracts, const std::string &price, const std::string &time_in_force)
{
    if (!authenticated)
    {
        std::cerr << "Cannot place order: not authenticated" << std::endl;
        return;
    }

    nlohmann::json params = {
        {"instrument_name", instrument_name}, // Required
        {"direction", direction},             // Required
        {"amount", amount},                   // Required
        {"contracts", contracts},             // If present, should be equal to amount
        {"type", order_type},                 // Default: "limit"
        {"price", price},
        {"time_in_force", time_in_force},
    };

    std::string method = direction == "buy" ? "private/buy" : "private/sell";

    SendRequest(method, params, [this](const nlohmann::json &result)
                { std::cout << "Order placed: " << result << std::endl; });
}
