#include <imgui.h>
#include <chrono>
#include <string>
#include "../../layout/ThemeUI.h"
#include "../../network/WebSocketHandler.h"

struct PositionParameters
{
    const char *label;
    const char *json_key;
    const int type; // 0 - double, 1 - string
};

class PositionsRenderer
{
private:
    WebSocketHandler &ws_client;
    static constexpr size_t BUFFER_SIZE = 256;
    static constexpr int REFRESH_INTERVAL = 10;
    bool firstLoad = true;
    bool m_show_window = true;

    static constexpr PositionParameters PARAMS[] = {
        {"Instr. Name", "instrument_name", 1},
        {"Kind", "kind", 1},
        {"Direction", "direction", 1},
        {"Total Profit/Loss", "total_profit_loss", 0},
        // {"Average Price", "average_price", 0},
        {"Delta", "delta", 0},
        // {"Size", "size", 0},                   // in USD
        // {"Size Currency", "size_currency", 0}, // in Base Currency
        // {"Est. Liq. Price", "estimated_liquidation_price", 0},
        // {"Floating Profit/Loss", "floating_profit_loss", 0},
        {"Index Price", "index_price", 0},
        // {"Interest Value", "interest_value", 0},
        // {"Leverage", "leverage", 0},
        // {"Mark Price", "mark_price", 0},
        // {"Open Orders Margin", "open_orders_margin", 0},
        // {"Realized Funding", "realized_funding", 0},
        // {"Realized Profit/Loss", "realized_profit_loss", 0},
        // {"Settlement Price", "settlement_price", 0},

    };

    char buffer[BUFFER_SIZE];
    std::chrono::system_clock::time_point last_refresh;

    void RenderPositionsTable(const nlohmann::json &positions)
    {
        if (!ImGui::BeginTable("AccPosition", std::size(PARAMS), ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
        {
            return;
        }

        for (const auto &param : PARAMS)
        {
            ImGui::TableSetupColumn(param.label, param.type ? ImGuiTableColumnFlags_WidthStretch : ImGuiTableColumnFlags_WidthFixed);
        }
        ImGui::TableHeadersRow();

        for (const auto &item : positions)
        {
            ImGui::TableNextRow();
            for (const auto &param : PARAMS)
            {
                ImGui::TableNextColumn();

                if (item.contains(param.json_key) && !item[param.json_key].is_null())
                {
                    // snprintf(buffer, BUFFER_SIZE, "%s", item[param.json_key].get<std::string>().c_str());
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
        }

        ImGui::EndTable();
    }

    void RenderLastUpdateTime(const std::chrono::system_clock::time_point &req_time)
    {
        auto now = std::chrono::system_clock::now();
        auto age = std::chrono::duration_cast<std::chrono::seconds>(now - req_time).count();

        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize("updated 10 secs ago").x - ImGui::GetStyle().ItemSpacing.x);
        ImGui::TextColored(
            GetColorFromImCol32(textDarkerColor),
            "updated %ld secs ago",
            age);
    }

    bool ShouldAutoRefresh() const
    {
        auto now = std::chrono::system_clock::now();
        return std::chrono::duration_cast<std::chrono::seconds>(now - last_refresh).count() > REFRESH_INTERVAL;
    }

public:
    PositionsRenderer(WebSocketHandler &ws_client) : ws_client(ws_client) {}

    void Render()
    {
        if (m_show_window == false)
            return;

        if (firstLoad || ShouldAutoRefresh())
        {
            ws_client.FetchPositions();
            last_refresh = std::chrono::system_clock::now();
            firstLoad = false;
        }

        ImGui::Begin("Account Positions", nullptr,
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);

        const auto &positions = ws_client.GetPositions();
        if (!positions.empty())
        {
            RenderPositionsTable(positions);
            RenderLastUpdateTime(ws_client.GetPositionsReqTime());
        }
        else
            ImGui::TextDisabled("No data available");

        ImGui::End();
    }

    bool IsVisible() const { return m_show_window; }
    void Show() { m_show_window = true; }
    void Hide() { m_show_window = false; }
    void Toggle() { m_show_window = !m_show_window; }
};
