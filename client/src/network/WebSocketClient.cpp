#include "WebSocketClient.h"

WebSocketClient::WebSocketClient(const std::string &uri) : uri(uri)
{
    ws_client.init_asio();
    // ws_client.set_tls_init_handler([](websocketpp::connection_hdl)
    //                               { return websocketpp::lib::make_shared<boost::asio::ssl::context>(
    //                                     boost::asio::ssl::context::tlsv12); });

    ws_client.set_access_channels(websocketpp::log::alevel::all);
    ws_client.clear_access_channels(websocketpp::log::alevel::frame_payload);

    //     // Define a message handler
    //     ws_client.set_message_handler([&](websocketpp::connection_hdl hdl, client::message_ptr msg)
    //                                   { std::cout << "Received: " << msg->get_payload() << std::endl; });

    //     // Define an open handler
    //     ws_client.set_open_handler([&](websocketpp::connection_hdl hdl)
    //                                {
    // std::string request = "username=" + username + "&password=" + password;
    // ws_client.send(hdl, request, websocketpp::frame::opcode::text); });

    //     // Define a close handler
    //     ws_client.set_close_handler([&](websocketpp::connection_hdl hdl)
    //                                 { std::cout << "Connection closed" << std::endl; });

    ws_client.set_message_handler(
        websocketpp::lib::bind(&WebSocketClient::OnMessage, this, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));
    ws_client.set_open_handler(
        websocketpp::lib::bind(&WebSocketClient::OnOpen, this, websocketpp::lib::placeholders::_1));
}

void WebSocketClient::Connect()
{
    websocketpp::lib::error_code ec;
    client::connection_ptr con = ws_client.get_connection(uri, ec);

    if (ec)
    {
        std::cerr << "Connection failed: " << ec.message() << std::endl;
        return;
    }

    connection = con->get_handle();
    ws_client.connect(con);

    std::thread([this]()
                { ws_client.run(); })
        .detach(); // Run WebSocket event loop in a separate thread
}

void WebSocketClient::SendMessage(const std::string &message)
{
    if (!connected)
    {
        std::cerr << "WebSocket is not connected yet." << std::endl;
        return;
    }

    websocketpp::lib::error_code ec;
    ws_client.send(connection, message, websocketpp::frame::opcode::text, ec);
    if (ec)
    {
        std::cerr << "Send failed: " << ec.message() << std::endl;
    }
}

void WebSocketClient::Authenticate(const std::string &client_id, const std::string &client_secret)
{
    std::cout << "Authenticating..." << std::endl;
    nlohmann::json auth_request = {
        {"method", "public/auth"},
        {"params", {{"grant_type", "client_credentials"}, {"client_id", client_id}, {"client_secret", client_secret}}},
        {"jsonrpc", "2.0"},
        {"id", 0}};

    SendMessage(auth_request.dump()); // Convert JSON to string and send
}

void WebSocketClient::Close()
{
    websocketpp::lib::error_code ec;
    ws_client.close(connection, websocketpp::close::status::normal, "Closing", ec);
    if (ec)
    {
        std::cerr << "Close failed: " << ec.message() << std::endl;
    }
}

void WebSocketClient::OnOpen(websocketpp::connection_hdl hdl)
{
    std::cout << "Connected to WebSocket server!" << std::endl;
    connected = true;
}

void WebSocketClient::OnMessage(websocketpp::connection_hdl hdl, client::message_ptr msg)
{
    std::cout << "Received: " << msg->get_payload() << std::endl;
}
