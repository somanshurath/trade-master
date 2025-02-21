#pragma once
#include <imgui.h>
#include <chrono>
#include <string>
#include "../utils/layout/ThemeUI.h"
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
    ControlPanel(WebSocketHandler &ws_client_inp) : ws_client(ws_client_inp)
    {};

    void InitRenderers()
    {
        renderers.push_back(std::make_unique<AccountSummaryRenderer>(ws_client));
        renderers.push_back(std::make_unique<PositionsRenderer>(ws_client));
        renderers.push_back(std::make_unique<OpenOrdersRenderer>(ws_client));
        renderers.push_back(std::make_unique<BookSummaryRenderer>(ws_client));
        renderers.push_back(std::make_unique<ChartDataRenderer>(ws_client));
    }

    void Render()
    {
        ImGui::Begin("Control Panel", nullptr,
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);

        ImGui::SeparatorText("Account Management");

        for (auto &renderer : renderers)
        {
            bool toBeRendered = renderer->IsVisible();
            if (!toBeRendered)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, GetColorFromImCol32(backgroundColor));

                if (ImGui::Button(renderer->tabName().c_str(), ImVec2(140, 0)))
                {
                    renderer->Toggle();
                }
                ImGui::PopStyleColor();
            }
            else
            {
                if (ImGui::Button(renderer->tabName().c_str(), ImVec2(140, 0)))
                {
                    renderer->Toggle();
                }
                renderer->Render();
            }
            ImGui::SameLine();

            ImGui::PushFont(g_iconsFont);
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize("Z").x - ImGui::GetStyle().ItemSpacing.x - 10);
            ImGui::PushID(&renderer - &renderers[0]);
            if (ImGui::Button("Z"))
            {
                renderer->FetchData();
                renderer->Show();
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

        ImGui::End();
    }

private:
    std::vector<std::unique_ptr<BaseRenderer>> renderers;
    WebSocketHandler &ws_client;
};
