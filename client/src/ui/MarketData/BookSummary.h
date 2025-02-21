#pragma once
#include <imgui.h>
#include <chrono>
#include <string>
#include "../BaseRenderer.h"
#include "../../utils/layout/ThemeUI.h"
#include "../../network/WebSocketHandler.h"

#define COLOR 1
#define NO_COLOR 0

struct BookSummayParameters
{
    const char *label;
    const char *json_key;
    const int type; // 0 - double, 1 - string
    const int color;
};

class BookSummaryRenderer : public BaseRenderer
{
private:
    WebSocketHandler &ws_client;
    static constexpr size_t BUFFER_SIZE = 256;
    static constexpr int REFRESH_INTERVAL = 10;
    char buffer[BUFFER_SIZE];
    std::chrono::system_clock::time_point last_refresh;

    static constexpr BookSummayParameters PARAMS[] = {
        {"Instrument", "instrument_name", 1, NO_COLOR},
        {"24h Low", "low", 0, NO_COLOR},
        {"24h High", "high", 0, NO_COLOR},
        {"24h Change", "price_change", 0, COLOR},
        {"24h Volume", "volume_notional", 0, NO_COLOR},
        {"Last", "last", 0, NO_COLOR},
        {"% Premium", "$custom", 0, COLOR},
        // {"Spread", "spread", 0},
    };

    bool selectedItems[std::size(PARAMS)] = {true};

    void RenderBookSummaryTable(const nlohmann::json &bookSummary)
    {
        int column_count = std::count(std::begin(selectedItems), std::end(selectedItems), true);
        if (column_count == 0)
            return;

        if (!ImGui::BeginTable("BookSummary", column_count, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
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

        for (const auto &item : bookSummary)
        {
            ImGui::TableNextRow();

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
                        double value;
                        double term1 = item["mark_price"].get<double>();
                        double term2 = item["estimated_delivery_price"].get<double>();
                        value = (term1 - term2) / term2 * 100;
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
        }

        ImGui::EndTable();
    }

    void RenderParamCombo()
    {
        if (ImGui::BeginCombo("##BookSummaryParamsToDisplay", "Selected Parameters", ImGuiComboFlags_NoPreview))
        {
            for (size_t i = 1; i < std::size(PARAMS); ++i)
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
    BookSummaryRenderer(WebSocketHandler &ws_client) : ws_client(ws_client)
    {
        this->FetchData();

        selectedItems[0] = true;
        selectedItems[1] = true;
        selectedItems[2] = true;
        selectedItems[3] = true;
        selectedItems[4] = true;
        selectedItems[5] = true;
        selectedItems[6] = true;
    }

    void FetchData() override
    {
        ws_client.FetchBookSummary("BTC", "future");
        ws_client.FetchBookSummary("ETH", "future");
        last_refresh = std::chrono::system_clock::now();
    }

    void Render() override
    {
        ImGui::Begin("Instrument Summary", nullptr,
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);

        ImGui::BeginTabBar("#BookSummaryTabBar");
        if (ImGui::BeginTabItem("BTC"))
        {
            auto bookSummary = ws_client.GetBookSummary("BTC", "future");
            if (!bookSummary.empty())
                RenderBookSummaryTable(bookSummary);
            else
                ImGui::TextDisabled("Loading data...");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("ETH"))
        {
            auto bookSummary = ws_client.GetBookSummary("ETH", "future");
            if (!bookSummary.empty())
                RenderBookSummaryTable(bookSummary);
            else
                ImGui::TextDisabled("Loading data...");
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
        RenderParamCombo();
        RenderLastUpdateTime(ws_client.GetBookSummaryReqTime());
        ImGui::End();

        if (ShouldAutoRefresh())
        {
            FetchData();
        }
    }

    std::string tabName() override
    {
        return "Instrument Summary";
    }
};
