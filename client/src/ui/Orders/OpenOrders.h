#pragma once
#include <imgui.h>
#include <chrono>
#include <string>
#include "../BaseRenderer.h"
#include "../../utils/layout/ThemeUI.h"
#include "../../network/WebSocketHandler.h"

struct OpenOrderParameters
{
    const char *label;
    const char *json_key;
    const int type; // 0 - double, 1 - string
};

class OpenOrdersRenderer : public BaseRenderer
{
private:
    WebSocketHandler &ws_client;
    static constexpr size_t BUFFER_SIZE = 256;
    static constexpr int REFRESH_INTERVAL = 10;

    static constexpr OpenOrderParameters PARAMS[] = {
        {"Instrument Name", "instrument_name", 1},
        {"Direction", "direction", 1},
        {"Order Type", "order_type", 1},
        {"Amount", "amount", 0},
        {"Price", "price", 0},
        // {"Average Price", "average_price", 0},
        {"Time in Force", "time_in_force", 1},
        // {"Contracts", "contracts", 0},
        // {"Creation Timestamp", "creation_timestamp", 0},
        // {"Filled Amount", "filled_amount", 0},
        // {"Is Liquidation", "is_liquidation", 1},
        // {"Label", "label", 1},
        // {"Last Update Timestamp", "last_update_timestamp", 0},
        // {"Max Show", "max_show", 0},
        // {"MMP", "mmp", 1},
        // {"Order ID", "order_id", 1},
        // {"Order State", "order_state", 1},
        // {"Post Only", "post_only", 1},
        // {"Reduce Only", "reduce_only", 1},
        // {"Replaced", "replaced", 1},
        // {"Risk Reducing", "risk_reducing", 1},
        // {"Web", "web", 1},
        // {"API", "api", 1},
    };

    bool selectedItems[std::size(PARAMS)] = {true};
    char buffer[BUFFER_SIZE];
    std::chrono::system_clock::time_point last_refresh;

    void RenderOpenOrdersTable(const nlohmann::json &openorders)
    {
        int column_count = std::count(std::begin(selectedItems), std::end(selectedItems), true);
        if (column_count == 0)
            return;

        if (!ImGui::BeginTable("AccOpenOrder", column_count, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
        {
            return;
        }

        for (size_t i = 0; i < std::size(PARAMS); ++i)
        {
            if (!selectedItems[i])
                continue;
            ImGui::TableSetupColumn(PARAMS[i].label, PARAMS[i].type ? ImGuiTableColumnFlags_WidthStretch : ImGuiTableColumnFlags_WidthFixed);
        }
        ImGui::TableHeadersRow();

        for (const auto &item : openorders)
        {
            ImGui::TableNextRow();

            bool active = item["direction"].get<std::string>() == "zero";
            if (active)
                ImGui::PushStyleColor(ImGuiCol_Text, GetColorFromImCol32(textDarkerColor));

            for (size_t i = 0; i < std::size(PARAMS); ++i)
            {
                if (!selectedItems[i])
                    continue;

                const auto &param = PARAMS[i];

                ImGui::TableNextColumn();

                if (item.contains(param.json_key) && !item[param.json_key].is_null())
                {
                    if (param.type)
                        snprintf(buffer, BUFFER_SIZE, "%s", item[param.json_key].get<std::string>().c_str());
                    else
                    {
                        double value = item[param.json_key].get<double>();
                        snprintf(buffer, BUFFER_SIZE, "%.8f", value);
                    }

                    ImGui::TextUnformatted(buffer);
                }
                else
                {
                    ImGui::TextUnformatted("-");
                }
            }

            if (active)
                ImGui::PopStyleColor();
        }

        ImGui::EndTable();
    }

    void RenderParamCombo()
    {
        if (ImGui::BeginCombo("##OpenOrderParamsToDisplay", "Selected Parameters", ImGuiComboFlags_WidthFitPreview))
        {
            for (size_t i = 3; i < std::size(PARAMS); ++i)
            {
                ImGui::Selectable(PARAMS[i].label, &selectedItems[i], ImGuiSelectableFlags_DontClosePopups);
            }
            ImGui::EndCombo();
        }

        ImGui::SameLine();
    }

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
    OpenOrdersRenderer(WebSocketHandler &ws_client) : ws_client(ws_client)
    {
        this->FetchData();

        selectedItems[0] = true;
        selectedItems[1] = true;
        selectedItems[2] = true;
        selectedItems[3] = true;
        selectedItems[4] = true;
        selectedItems[5] = true;
    };

    void FetchData() override
    {
        ws_client.FetchOpenOrders();
        last_refresh = std::chrono::system_clock::now();
    }

    void Render() override
    {
        ImGui::Begin("Open Orders", nullptr,
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);

        const auto &openorders = ws_client.GetOpenOrders();
        if (!openorders.empty())
        {
            RenderOpenOrdersTable(openorders);
            RenderParamCombo();
            RenderLastUpdateTime(ws_client.GetOpenOrdersReqTime());
        }
        else
        {
            ImGui::TextDisabled("No open orders");
        }

        ImGui::End();

        if (ShouldAutoRefresh())
        {
            FetchData();
        }
    }

    std::string tabName() override { return "Open Orders"; }
};
