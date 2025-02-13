#include <imgui.h>
#include "../../network/WebSocketHandler.h"
#include "utils/env/env.h"

class Login
{
public:
    Login(WebSocketHandler &ws_client_inp)
        : ws_client(ws_client_inp)
    {
    }

    void Render()
    {
        ImGui::Begin("Trade Master Terminal", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
        ImGui::Text("Client ID");
        static char client_id[128];
        strncpy(client_id, CLIENT_ID.c_str(), sizeof(client_id) - 1);
        client_id[sizeof(client_id) - 1] = '\0';
        ImGui::InputText("##client_id", client_id, IM_ARRAYSIZE(client_id));
        ImGui::Text("Client Secret");
        static char client_secret[128];
        strncpy(client_secret, CLIENT_SECRET.c_str(), sizeof(client_secret) - 1);
        client_secret[sizeof(client_secret) - 1] = '\0';
        ImGui::InputText("##client_secret", client_secret, IM_ARRAYSIZE(client_secret), ImGuiInputTextFlags_Password);
        ImGui::Spacing();
        if (ImGui::Button("Log In"))
        {
            ws_client.Authenticate(client_id, client_secret);
        }
        ImGui::End();
    }

private:
    WebSocketHandler &ws_client;
};
