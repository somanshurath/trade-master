#include "WebSocketHandler.h"

WebSocketHandler::WebSocketHandler(const std::string &uri) : uri(uri)
{
    client.init_asio();
    client.set_tls_init_handler([](websocketpp::connection_hdl)
                                { return websocketpp::lib::make_shared<asio::ssl::context>(
                                      asio::ssl::context::tlsv12); });

    client.set_access_channels(websocketpp::log::alevel::all);
    client.clear_access_channels(websocketpp::log::alevel::frame_payload);

    client.set_message_handler(
        websocketpp::lib::bind(&WebSocketHandler::OnMessage, this,
                               websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));

    client.set_open_handler(
        websocketpp::lib::bind(&WebSocketHandler::OnOpen, this,
                               websocketpp::lib::placeholders::_1));

    client.set_close_handler(
        websocketpp::lib::bind(&WebSocketHandler::OnClose, this,
                               websocketpp::lib::placeholders::_1));
}

void WebSocketHandler::Connect()
{
    websocketpp::lib::error_code ec;
    ws_client::connection_ptr con = client.get_connection(uri, ec);

    if (ec)
    {
        std::cerr << "Connection initialization failed: " << ec.message() << std::endl;
        return;
    }

    connection = con->get_handle();
    client.connect(con);

    std::thread([this]()
                {
        try {
            client.run();
        } catch (const std::exception &e) {
            std::cerr << "WebSocket run error: " << e.what() << std::endl;
        } })
        .detach();
}

void WebSocketHandler::OnMessage(websocketpp::connection_hdl hdl, message_ptr msg)
{
    if (!msg || msg->get_payload().empty())
    {
        return;
    }
    try
    {
        if (msg->get_payload().size() > 1024 * 1024)
        { // 1MB limit
            std::cerr << "Message too large" << std::endl;
            return;
        }

        auto payload = nlohmann::json::parse(msg->get_payload());
        std::cout << "Received: " << payload.dump(2) << std::endl;

        // Handle errors
        if (payload.contains("error"))
        {
            HandleError(payload["error"]);
            return;
        }

        // Handle responses to specific requests
        if (payload.contains("id") && payload["id"].is_number())
        {
            int response_id = payload["id"];
            std::lock_guard<std::mutex> lock(handlers_mutex);
            auto it = pending_requests.find(response_id);
            if (it != pending_requests.end())
            {
                if (payload.contains("result"))
                {
                    it->second(payload["result"]);
                }
                pending_requests.erase(it);
                return;
            }
        }

        // Handle subscriptions and other method-based messages
        if (payload.contains("method"))
        {
            std::string method = payload["method"];
            auto it = message_handlers.find(method);
            if (it != message_handlers.end())
            {
                it->second(payload.contains("params") ? payload["params"] : nlohmann::json());
            }
            else
            {
                std::cout << "No handler registered for method: " << method << std::endl;
            }
        }
    }
    catch (const nlohmann::json::exception &e)
    {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error processing message: " << e.what() << std::endl;
    }
}

void WebSocketHandler::OnOpen(websocketpp::connection_hdl hdl)
{
    std::cout << "WebSocket connection established" << std::endl;
    connected = true;
}

void WebSocketHandler::OnClose(websocketpp::connection_hdl hdl)
{
    std::cout << "WebSocket connection closed" << std::endl;
    connected = false;
    authenticated = false;

    // should i even be doing this?
    // TODO: Improve this shit
    closing = false;
    if (!closing)
    {
        std::cout << "Reconnecting..." << std::endl;
        Connect();
    }
}

void WebSocketHandler::OnError(websocketpp::connection_hdl hdl)
{
    std::cerr << "WebSocket error occurred" << std::endl;
}

void WebSocketHandler::SendMessage(const std::string &message)
{
    if (!connected)
    {
        std::cerr << "WebSocket is not connected" << std::endl;
        return;
    }

    websocketpp::lib::error_code ec;
    client.send(connection, message, websocketpp::frame::opcode::text, ec);
    if (ec)
    {
        std::cerr << "Send failed: " << ec.message() << std::endl;
    }
}

void WebSocketHandler::RegisterMessageHandler(const std::string &method,
                                              std::function<void(const nlohmann::json &)> handler)
{
    message_handlers[method] = std::move(handler);
}

int WebSocketHandler::GetNextRequestId()
{
    return request_id++;
}

void WebSocketHandler::SendRequest(const std::string &method, const nlohmann::json &params,
                                   std::function<void(const nlohmann::json &)> callback)
{
    int id = GetNextRequestId();
    nlohmann::json request = {
        {"jsonrpc", "2.0"},
        {"method", method},
        {"params", params},
        {"id", id}};

    if (callback)
    {
        std::lock_guard<std::mutex> lock(handlers_mutex);
        pending_requests[id] = std::move(callback);
    }

    SendMessage(request.dump());
}

void WebSocketHandler::HandleError(const nlohmann::json &error)
{
    std::cerr << "Received error: " << error.dump(2) << std::endl;
    // Additional error handling logic can be implemented here
}

void WebSocketHandler::HandleAuthResponse(const nlohmann::json &response)
{
    if (response.contains("access_token"))
    {
        access_token = response["access_token"];
        if (response.contains("refresh_token"))
        {
            refresh_token = response["refresh_token"];
        }
        if (response.contains("expires_in"))
        {
            expires_in = response["expires_in"];
        }
        authenticated = true;
        std::cout << "Authentication successful" << std::endl;
    }
    else
    {
        std::cerr << "Authentication failed: missing access token" << std::endl;
    }
}

void WebSocketHandler::HandleSubscriptionResponse(const nlohmann::json &response)
{
    // Implement subscription response handling logic here
    std::cout << "Subscription response received: " << response.dump(2) << std::endl;
}

void WebSocketHandler::Close()
{
    if (!connected)
    {
        return;
    }

    websocketpp::lib::error_code ec;
    client.close(connection, websocketpp::close::status::normal, "Closing connection", ec);
    if (ec)
    {
        std::cerr << "Error closing connection: " << ec.message() << std::endl;
    }

    connected = false;
}
