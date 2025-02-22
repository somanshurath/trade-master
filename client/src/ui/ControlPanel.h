#pragma once
#include <imgui.h>
#include <chrono>
#include <string>
#include "../utils/layout/ThemeUI.h"
#include "../utils/layout/CenterView.h"
#include "../utils/fonts/Fonts.h"
#include "./BaseRenderer.h"
#include "../network/WebSocketHandler.h"

#include "AccountManagement/AccountSummary.h"
#include "AccountManagement/Positions.h"
#include "Orders/OpenOrders.h"
#include "MarketData/BookSummary.h"
#include "MarketData/ChartData.h"
#include <bitset>

class ControlPanel
{
public:
    ControlPanel(WebSocketHandler &ws_client_inp) : ws_client(ws_client_inp) {};

    void InitRenderers()
    {
        renderers.push_back(std::make_unique<BookSummaryRenderer>(ws_client));
        renderers.push_back(std::make_unique<ChartDataRenderer>(ws_client));
        renderers.push_back(std::make_unique<AccountSummaryRenderer>(ws_client));
        renderers.push_back(std::make_unique<PositionsRenderer>(ws_client));
        renderers.push_back(std::make_unique<OpenOrdersRenderer>(ws_client));
    }

    void Render()
    {
        ImGui::Begin("Control Panel", nullptr,
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);

        int demarcationIndex = 0;

        for (int i = 0; i < renderers.size(); i++)
        {
            if (demarcationIndex < demarcation.size() && i == demarcation[demarcationIndex])
            {
                ImGui::SeparatorText(demarcationNames[demarcationIndex].c_str());
                demarcationIndex++;
            }

            bool toBeRendered = renderers[i]->IsVisible();
            if (!toBeRendered)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, GetColorFromImCol32(backgroundColor));

                if (ImGui::Button(renderers[i]->tabName().c_str(), ImVec2(140, 0)))
                {
                    renderers[i]->Toggle();
                }
                ImGui::PopStyleColor();
            }
            else
            {
                if (ImGui::Button(renderers[i]->tabName().c_str(), ImVec2(140, 0)))
                {
                    renderers[i]->Toggle();
                }
                renderers[i]->Render();
            }
            ImGui::SameLine();

            ImGui::PushFont(g_iconsFont);
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize("Z").x - ImGui::GetStyle().ItemSpacing.x - 10);
            ImGui::PushID(i);
            if (ImGui::Button("Z"))
            {
                renderers[i]->FetchData();
                renderers[i]->Show();
            }
            ImGui::PopID();
            ImGui::PopFont();
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::TextUnformatted("Auto-refreshes every 10 seconds");
                ImGui::EndTooltip();
            }
        }

        ImGui::Dummy(ImVec2(0.0f, 5.0f));
        if (CENTER(ImGui::Button("Logout", ImVec2(100, 0))))
        {
            ws_client.SetClosing();
            ws_client.Close();
        };

        ImGui::End();
    }

private:
    std::vector<std::unique_ptr<BaseRenderer>> renderers;
    WebSocketHandler &ws_client;
    std::array<int, 2> demarcation = {0, 2}; // 0 - Market Data, 1 - Account Management
    std::array<std::string, 2> demarcationNames = {"Market Data", "Account Management"};
};
