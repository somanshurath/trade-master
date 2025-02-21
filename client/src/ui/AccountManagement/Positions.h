#pragma once
#include <imgui.h>
#include <chrono>
#include <string>
#include "../BaseRenderer.h"
#include "../../utils/layout/ThemeUI.h"
#include "../../network/WebSocketHandler.h"

#define COLOR 1
#define NO_COLOR 0

struct PositionParameters
{
    const char *label;
    const char *json_key;
    const int type; // 0 - double, 1 - string
    const int color;
};

class PositionsRenderer : public BaseRenderer
{
private:
    WebSocketHandler &ws_client;
    static constexpr size_t BUFFER_SIZE = 256;
    static constexpr int REFRESH_INTERVAL = 10;
    char buffer[BUFFER_SIZE];
    std::chrono::system_clock::time_point last_refresh;

    static constexpr PositionParameters PARAMS[] = {
        {"Instr. Name", "instrument_name", 1, NO_COLOR},
        {"Kind", "kind", 1, NO_COLOR},
        {"Direction", "direction", 1, NO_COLOR},
        {"Amount", "size", 0, NO_COLOR},         // in USD
        {"Value", "size_currency", 0, NO_COLOR}, // in Base Currency
        {"Avg Price", "average_price", 0, NO_COLOR},
        {"ELP", "estimated_liquidation_price", 0, NO_COLOR},
        {"PNL", "floating_profit_loss", 0, COLOR},
        {"ROI", "$roi", 0, COLOR},
        {"Index Price", "index_price", 0, NO_COLOR},
        // {"Interest Value", "interest_value", 0, NO_COLOR},
        // {"Leverage", "leverage", 0, NO_COLOR},
        {"Market Price", "mark_price", 0, NO_COLOR},
        {"Initial Margin", "initial_margin", 0, NO_COLOR},
        {"Mainten. Margin", "maintenance_margin", 0, NO_COLOR},
        {"Delta", "delta", 0, NO_COLOR},

        // {"Open Orders Margin", "open_orders_margin", 0, NO_COLOR},
        // {"Realized Funding", "realized_funding", 0, NO_COLOR},
        {"Realized P/L", "realized_profit_loss", 0, NO_COLOR},
        {"Settlement Price", "settlement_price", 0, NO_COLOR},
        {"Total P/L", "total_profit_loss", 0, NO_COLOR},
    };

    bool selectedItems[std::size(PARAMS)] = {true};

    void RenderPositionsTable(const nlohmann::json &positions)
    {
        int column_count = std::count(std::begin(selectedItems), std::end(selectedItems), true);
        if (column_count == 0)
            return;

        if (!ImGui::BeginTable("AccPosition", column_count, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
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

        for (const auto &item : positions)
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
                        snprintf(buffer, BUFFER_SIZE, "%.4f", value);
                        ImVec4 color;
                        if (param.color == COLOR)
                        {
                            if (value > 0)
                                color = ImVec4(0.000f, 1.000f, 0.441f, 1.000f);
                            else if (value < 0)
                                color = ImVec4(0.853f, 0.050f, 0.310f, 1.000f);
                            ImGui::PushStyleColor(ImGuiCol_Text, color);
                        }
                    }
                    ImGui::TextUnformatted(buffer);
                    if (param.color == COLOR)
                    {
                        ImGui::PopStyleColor();
                    }
                }
                else
                {
                    if (param.json_key[0] == '$')
                    {
                        double value, term1, term2;
                        if (item["kind"] == "future")
                        {
                            term1 = item["floating_profit_loss"].get<double>();
                            term2 = item["initial_margin"].get<double>();
                            value = term1 / term2 * 100;
                        }
                        else
                        {
                            term1 = item["mark_price"].get<double>();
                            term2 = item["index_price"].get<double>();
                            value = (term1 - term2) / term2 * 100;
                        }
                        snprintf(buffer, BUFFER_SIZE, "%.4f", value);
                        ImVec4 color;
                        if (param.color == COLOR)
                        {
                            if (value > 0)
                                color = ImVec4(0.000f, 1.000f, 0.441f, 1.000f);
                            else if (value < 0)
                                color = ImVec4(0.853f, 0.050f, 0.310f, 1.000f);
                            ImGui::PushStyleColor(ImGuiCol_Text, color);
                        }
                        ImGui::TextUnformatted(buffer);
                        if (param.color == COLOR)
                        {
                            ImGui::PopStyleColor();
                        }
                    }
                    else
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
        if (ImGui::BeginCombo("##PositionParamsToDisplay", "Selected Parameters", ImGuiComboFlags_NoPreview))
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
    PositionsRenderer(WebSocketHandler &ws_client) : ws_client(ws_client)
    {
        this->FetchData();

        selectedItems[0] = true;
        selectedItems[1] = true;
        selectedItems[2] = true;
        selectedItems[3] = true;
        selectedItems[4] = true;
        selectedItems[5] = true;
        selectedItems[6] = true;
        selectedItems[7] = true;
        selectedItems[8] = true;
    };

    void FetchData() override
    {
        ws_client.FetchPositions();
        last_refresh = std::chrono::system_clock::now();
    }

    void Render() override
    {
        ImGui::Begin("Account Positions", nullptr,
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);

        const auto &positions = ws_client.GetPositions();
        if (!positions.empty())
        {
            RenderPositionsTable(positions);
            RenderParamCombo();
            RenderLastUpdateTime(ws_client.GetPositionsReqTime());
        }
        else
            ImGui::TextDisabled("Loading data...");

        ImGui::End();

        if (ShouldAutoRefresh())
        {
            FetchData();
        }
    }

    std::string tabName() override { return "Positions"; }
};
