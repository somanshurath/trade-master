#pragma once
#include <imgui.h>
#include <chrono>
#include <string>
#include "../../utils/layout/ThemeUI.h"
#include "../../network/WebSocketHandler.h"

class OrderPlacementRenderer

{
private:
    WebSocketHandler &ws_client;
    static constexpr size_t BUFFER_SIZE = 256;
    static constexpr int REFRESH_INTERVAL = 10;
    bool firstLoad = true;
    bool m_show_window = true;

    char buffer[BUFFER_SIZE];
    std::chrono::system_clock::time_point last_refresh;

    void RenderPlacementSection()
    {
        ImGui::Text("Place Order");

        ImGui::Separator();

        ImGui::Text("Instrument Name");
        // use imgui combo box to select instrument name
        if (ImGui::BeginCombo("##InstrumentName", "Select Instrument Name", ImGuiComboFlags_WidthFitPreview))
        {
            // for (size_t i = 0; i < ws_client.GetInstruments().size(); ++i)
            // {
            //     ImGui::Selectable(ws_client.GetInstruments()[i].get("instrument_name").get<std::string>().c_str());
            // }
            ImGui::EndCombo();
        }

        ImGui::Text("Direction");
        ImGui::InputText("##Direction", buffer, BUFFER_SIZE);

        ImGui::Text("Order Type");
        ImGui::InputText("##OrderType", buffer, BUFFER_SIZE);

        ImGui::Text("Amount");
        ImGui::InputText("##Amount", buffer, BUFFER_SIZE);

        ImGui::Text("Price");
        ImGui::InputText("##Price", buffer, BUFFER_SIZE);

        ImGui::Text("Time in Force");
        ImGui::InputText("##TimeInForce", buffer, BUFFER_SIZE);

        ImGui::Separator();

        if (ImGui::Button("Place Order"))
        {
            // ws_client.PlaceOrder();
        }
    }

    enum class type
    {
        limit,
        stop_limit,
        take_limit,
        market,
        stop_market,
        take_market,
        market_limit,
        trailing_stop,
    };


    // void RenderParamCombo()
    // {
    //     if (ImGui::BeginCombo("##OpenOrderParamsToDisplay", "Selected Parameters", ImGuiComboFlags_WidthFitPreview))
    //     {
    //         for (size_t i = 0; i < static_cast<size_t>(OrderParams::COUNT); ++i)
    //         {
    //             ImGui::Selectable(PARAMS[i], &selectedItems[i], ImGuiSelectableFlags_DontClosePopups);
    //         }
    //         ImGui::EndCombo();
    //     }

    //     ImGui::SameLine();
    // }

    void RenderLastUpdateTime(const std::chrono::system_clock::time_point &req_time)
    {
        auto now = std::chrono::system_clock::now();
        auto age = std::chrono::duration_cast<std::chrono::seconds>(now - req_time).count();

        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize("10s").x - ImGui::GetStyle().ItemSpacing.x);
        ImGui::TextColored(
            GetColorFromImCol32(textDarkerColor),
            "%lds",
            age);
    }

    bool ShouldAutoRefresh() const
    {
        auto now = std::chrono::system_clock::now();
        return std::chrono::duration_cast<std::chrono::seconds>(now - last_refresh).count() > REFRESH_INTERVAL;
    }

public:
    OrderPlacementRenderer(WebSocketHandler &ws_client) : ws_client(ws_client)
    {
    };

    void Render()
    {
        if (m_show_window == false)
            return;

        ImGui::Begin("Place Order", nullptr, ImGuiWindowFlags_NoCollapse);

        if (ws_client.IsAuthenticated())
        {
            RenderPlacementSection();
        }
        else
            ImGui::TextDisabled("Not available");

        ImGui::End();
    }

    bool IsVisible() const { return m_show_window; }
    void Show() { m_show_window = true; }
    void Hide() { m_show_window = false; }
    void Toggle() { m_show_window = !m_show_window; }
};
