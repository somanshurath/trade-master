#pragma once
#include <string>
#include <imgui.h>
#include "../../network/WebSocketHandler.h"
#include "../../utils/env/env.h"
#include "../../utils/layout/ThemeUI.h"
#include "../../utils/layout/CenterView.h"

class Login
{
public:
    Login(WebSocketHandler &ws_client_inp)
        : ws_client(ws_client_inp)
    {
        strncpy(client_id, CLIENT_ID.c_str(), sizeof(client_id) - 1);
        client_id[sizeof(client_id) - 1] = '\0';

        strncpy(client_secret, CLIENT_SECRET.c_str(), sizeof(client_secret) - 1);
        client_secret[sizeof(client_secret) - 1] = '\0';
    }

    void Render()
    {
        ImGui::Begin("Trade Master Terminal", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
        bool status = ws_client.GetConnectionStatus();
        ImVec4 color = GetColorFromImCol32(textDarkerColor);
        std::string statusString = "";
        if (status == false)
        {
            statusString = "Establishing Connection...";
        }
        else
        {
            statusString = "Connection Established";
        }
        ImGui::TextColored(color, statusString.c_str());
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f, 5.0f));
        ImGui::BeginDisabled(!status);
        ImGui::Text("Client ID");
        ImGui::InputText("##client_id", client_id, IM_ARRAYSIZE(client_id));
        ImGui::Text("Client Secret");
        ImGui::InputText("##client_secret", client_secret, IM_ARRAYSIZE(client_secret), ImGuiInputTextFlags_Password);
        ImGui::Spacing();
        //  std::string loginResponse = ws_client.GetLoginResponse();
        ImGui::Dummy(ImVec2(0.0f, 5.0f));
        if (CENTER(ImGui::Button("Log In")))
        {
            ws_client.Authenticate(client_id, client_secret);
        };
        ImGui::EndDisabled();
        ImGui::End();
    }

private:
    WebSocketHandler &ws_client;
    char client_id[128];
    char client_secret[128];
};
