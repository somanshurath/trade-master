#include "../WebSocketHandler.h"

void WebSocketHandler::Authenticate(const std::string &client_id, const std::string &client_secret)
{
    SetClientId(client_id);
    SetClientSecret(client_secret);
    std::cout << "Authenticating..." << std::endl;

    nlohmann::json auth_params = {
        {"grant_type", "client_credentials"},
        {"client_id", client_id},
        {"client_secret", client_secret}};

    SendRequest("public/auth", auth_params, [this](const nlohmann::json &result)
                { HandleAuthResponse(result); });
}

void WebSocketHandler::RefreshToken()
{
    nlohmann::json auth_params = {
        {"grant_type", "refresh_token"},
        {"refresh_token", refresh_token}};

    SendRequest("public/auth", auth_params, [this](const nlohmann::json &result)
                { HandleAuthResponse(result); });
}
