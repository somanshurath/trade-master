#pragma once
#include <imgui.h>
#include <chrono>
#include <string>
#include "../utils/layout/ThemeUI.h"
#include "../utils/fonts/Fonts.h"
#include "../network/WebSocketHandler.h"

#include "AccountManagement/AccountSummary.h"
#include "AccountManagement/Positions.h"
#include <bitset>

class ControlPanel
{
public:
    ControlPanel(WebSocketHandler &ws_client_inp)
        : ws_client(ws_client_inp), account_summary_renderer(ws_client_inp), positions_renderer(ws_client_inp)
    {
        tabToggleState.set();
    }

    void Render()
    {
        ImGui::Begin("Control Panel", nullptr,
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);

        ImGui::SeparatorText("Account Management");

        const char *tabs[] = {"Account Summary", "Open Positions"};
        bool changed = false;

        for (int i = 0; i < 2; ++i)
        {
            if (!tabToggleState[i])
            {
                ImGui::PushStyleColor(ImGuiCol_Button, GetColorFromImCol32(backgroundColor));
                changed = true;
            }

            if (ImGui::Button(tabs[i], ImVec2(120, 0)))
            {
                if (i == 0)
                {
                    account_summary_renderer.Toggle();
                }
                else if (i == 1)
                {
                    positions_renderer.Toggle();
                }
                tabToggleState[i].flip();
            }

            if (changed)
            {
                ImGui::PopStyleColor();
                changed = false;
            }

            if (i == 0)
            {
                account_summary_renderer.Render();
            }
            else if (i == 1)
            {
                positions_renderer.Render();
            }

            ImGui::SameLine();

            ImGui::PushFont(g_iconsFont);
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize("Z").x - ImGui::GetStyle().ItemSpacing.x - 10);
            ImGui::PushID(i);
            if (ImGui::Button("Z"))
            {
                if (i == 0)
                {
                    ws_client.FetchAccountSummary();
                    account_summary_renderer.Show();
                }
                else if (i == 1)
                {
                    ws_client.FetchPositions();
                    positions_renderer.Show();
                }
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
    AccountSummaryRenderer account_summary_renderer;
    PositionsRenderer positions_renderer;
    WebSocketHandler &ws_client;
    std::bitset<2> tabToggleState;
};
