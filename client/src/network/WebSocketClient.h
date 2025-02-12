#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#define ASIO_STANDALONE

#include <websocketpp/config/asio_no_tls_client.hpp>
// #include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <nlohmann/json.hpp>

#include <iostream>
#include <thread>

// typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
typedef websocketpp::client<websocketpp::config::asio_client> client;

class WebSocketClient
{
public:
    WebSocketClient(const std::string &uri);
    void Connect();
    void SendMessage(const std::string &message);
    void Authenticate(const std::string &client_id, const std::string &client_secret);
    void Close();

private:
    void OnMessage(websocketpp::connection_hdl hdl, client::message_ptr msg);
    void OnOpen(websocketpp::connection_hdl hdl);

    client ws_client;
    websocketpp::connection_hdl connection;
    std::string uri;
    bool connected = false;
};

#endif // WEBSOCKET_CLIENT_H
