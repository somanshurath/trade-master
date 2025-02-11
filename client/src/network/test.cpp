// #include <websocketpp/config/asio_no_tls_client.hpp>
// #include <websocketpp/client.hpp>

// #include <iostream>
// #include <string>

// typedef websocketpp::client<websocketpp::config::asio_client> client;

// class TradingApp {
// public:
//     TradingApp() {
//         m_client.init_asio();
//         m_client.set_open_handler(bind(&TradingApp::on_open, this, ::_1));
//         m_client.set_message_handler(bind(&TradingApp::on_message, this, ::_1, ::_2));
//         m_client.set_close_handler(bind(&TradingApp::on_close, this, ::_1));
//     }

//     void connect(const std::string& uri) {
//         websocketpp::lib::error_code ec;
//         client::connection_ptr con = m_client.get_connection(uri, ec);
//         if (ec) {
//             std::cout << "Could not create connection because: " << ec.message() << std::endl;
//             return;
//         }
//         m_client.connect(con);
//         m_client.run();
//     }

// private:
//     void on_open(websocketpp::connection_hdl hdl) {
//         std::cout << "Connection opened" << std::endl;
//         m_hdl = hdl;
//         m_client.send(hdl, "Hello, server!", websocketpp::frame::opcode::text);
//     }

//     void on_message(websocketpp::connection_hdl hdl, client::message_ptr msg) {
//         std::cout << "Received message: " << msg->get_payload() << std::endl;
//     }

//     void on_close(websocketpp::connection_hdl hdl) {
//         std::cout << "Connection closed" << std::endl;
//     }

//     client m_client;
//     websocketpp::connection_hdl m_hdl;
// };

// int main() {
//     TradingApp app;
//     std::string uri = "ws://echo.websocket.org";
//     app.connect(uri);
//     return 0;
// }
