#pragma once

#define ASIO_STANDALONE

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <nlohmann/json.hpp>

#include <iostream>
#include <thread>

typedef websocketpp::client<websocketpp::config::asio_tls_client> ws_client;
typedef ws_client::message_ptr message_ptr;
typedef websocketpp::lib::shared_ptr<asio::ssl::context> context_ptr;

class WebSocketHandler
{
public:
    explicit WebSocketHandler(const std::string &uri);
    ~WebSocketHandler() = default;

    void Connect();
    void Close();
    void SendMessage(const std::string &message);

    // Authentication
    void Authenticate(const std::string &client_id, const std::string &client_secret);
    void RefreshToken();
    bool IsAuthenticated() const { return authenticated; }

    // Account
    void FetchAccountSummary();
    nlohmann::json GetAccountSummary() const
    {
        std::lock_guard<std::mutex> lock(m_account_summary_mutex);
        return m_account_summary;
    }

    void FetchPositions();
    nlohmann::json GetPositions() const
    {
        std::lock_guard<std::mutex> lock(m_positions_mutex);
        return m_positions;
    }

    // Orders
    void PlaceOrder(const std::string &instrument_name, const std::string &direction, const std::string &order_type, const std::string &amount, const std::string &contracts, const std::string &price, const std::string &time_in_force);
    void FetchOpenOrders();
    nlohmann::json GetOpenOrders() const
    {
        std::lock_guard<std::mutex> lock(m_open_orders_mutex);
        return m_open_orders;
    }

    void FetchAccessLog();
    nlohmann::json GetAccessLog() const
    {
        std::lock_guard<std::mutex> lock(m_access_log_mutex);
        return m_access_log;
    }

    // Market Data
    void FetchBookSummary(const std::string &symbol, const std::string &kind);
    nlohmann::json GetBookSummary(const std::string &symbol, const std::string &kind) const
    {
        std::lock_guard<std::mutex> lock(m_book_summary_mutex);
        std::string s_kind = kind;
        if (kind == "")
        {
            s_kind = "all";
        }
        if (m_book_summary.find(symbol) != m_book_summary.end() && m_book_summary.at(symbol).find(s_kind) != m_book_summary.at(symbol).end())
            return m_book_summary.at(symbol).at(s_kind);
        else
            return nlohmann::json();
    }

    void FetchChartData(const std::string &instrument, const time_t &start_timestamp, const time_t &end_timestamp, const std::string &resolution);
    nlohmann::json GetChartData()
    {
        std::lock_guard<std::mutex> lock(m_chart_data_mutex);
        return m_chart_data;
    }

    // Getters and setters
    void SetUserId(const std::string &id) { user_id = id; }
    std::string GetUserId() const { return user_id; }
    void SetClientId(const std::string &id) { client_id = id; }
    std::string GetClientId() const { return client_id; }
    void SetClientSecret(const std::string &secret) { client_secret = secret; }
    std::string GetClientSecret() const { return client_secret; }
    void SetAccessToken(const std::string &token) { access_token = token; }
    std::string GetAccessToken() const { return access_token; }
    void SetRefreshToken(const std::string &token) { refresh_token = token; }
    std::string GetRefreshToken() const { return refresh_token; }
    void SetExpiresIn(uint32_t expires) { expires_in = expires; }
    uint32_t GetExpiresIn() const { return expires_in; }

    std::chrono::system_clock::time_point GetAccountSummaryReqTime() const { return m_account_summary_req_time; }
    std::chrono::system_clock::time_point GetAccessLogReqTime() const { return m_access_log_req_time; }
    std::chrono::system_clock::time_point GetPositionsReqTime() const { return m_positions_req_time; }
    std::chrono::system_clock::time_point GetOpenOrdersReqTime() const { return m_open_orders_req_time; }
    std::chrono::system_clock::time_point GetBookSummaryReqTime() const { return m_book_summary_req_time; }
    std::chrono::system_clock::time_point GetChartDataReqTime() const { return m_chart_data_req_time; }

    // Message handling
    void RegisterMessageHandler(const std::string &method,
                                std::function<void(const nlohmann::json &)> handler);
    void SendRequest(const std::string &method, const nlohmann::json &params,
                     std::function<void(const nlohmann::json &)> callback = nullptr);

private:
    ws_client client;
    websocketpp::connection_hdl connection;
    std::string uri;
    bool connected{false};
    bool authenticated{false};

    // User information
    std::string user_id;
    std::string client_id;
    std::string client_secret;
    std::string access_token;
    std::string refresh_token;
    uint32_t expires_in{0};

    mutable std::mutex m_account_summary_mutex;
    nlohmann::json m_account_summary;
    std::chrono::system_clock::time_point m_account_summary_req_time;

    mutable std::mutex m_access_log_mutex;
    nlohmann::json m_access_log;
    std::chrono::system_clock::time_point m_access_log_req_time;

    mutable std::mutex m_positions_mutex;
    nlohmann::json m_positions;
    std::chrono::system_clock::time_point m_positions_req_time;

    mutable std::mutex m_open_orders_mutex;
    nlohmann::json m_open_orders;
    std::chrono::system_clock::time_point m_open_orders_req_time;

    mutable std::mutex m_book_summary_mutex;
    std::unordered_map<std::string, std::unordered_map<std::string, nlohmann::json>> m_book_summary;
    std::chrono::system_clock::time_point m_book_summary_req_time;

    mutable std::mutex m_chart_data_mutex;
    nlohmann::json m_chart_data;
    std::chrono::system_clock::time_point m_chart_data_req_time;

    // Message handling
    std::unordered_map<std::string, std::function<void(const nlohmann::json &)>> message_handlers;
    std::atomic<int> request_id{0};
    std::mutex handlers_mutex;
    std::unordered_map<int, std::function<void(const nlohmann::json &)>> pending_requests;

    // Event handlers
    void OnOpen(websocketpp::connection_hdl hdl);
    void OnMessage(websocketpp::connection_hdl hdl, message_ptr msg);
    void OnClose(websocketpp::connection_hdl hdl);
    void OnError(websocketpp::connection_hdl hdl);

    // Helper methods
    void HandleAuthResponse(const nlohmann::json &response);
    void HandleSubscriptionResponse(const nlohmann::json &response);
    void HandleError(const nlohmann::json &error);
    int GetNextRequestId();
};
