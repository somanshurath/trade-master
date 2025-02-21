#pragma once
#include <imgui.h>
#include <chrono>
#include <string>
#include "../BaseRenderer.h"
#include "../../utils/layout/ThemeUI.h"
#include "../../network/WebSocketHandler.h"

struct AccountMetric
{
    const char *label;
    const char *json_key;
};

class AccountSummaryRenderer : public BaseRenderer
{
private:
    WebSocketHandler &ws_client;
    static constexpr size_t BUFFER_SIZE = 256;
    static constexpr int REFRESH_INTERVAL = 20;
    char buffer[BUFFER_SIZE];
    std::chrono::system_clock::time_point last_refresh;

    static constexpr AccountMetric METRICS[] = {
        {"Equity", "equity"},
        {"Available Funds", "available_funds"},
        {"Balance", "balance"},
        {"Initial Margin", "initial_margin"},
        {"Maintenance Margin", "maintenance_margin"}};

    void RenderMetric(const nlohmann::json &summary, const AccountMetric &metric)
    {
        if (summary.contains(metric.json_key))
        {
            double value = summary[metric.json_key].get<double>();
            snprintf(buffer, BUFFER_SIZE, "%s: %.4f", metric.label, value);
            ImGui::TextUnformatted(buffer);
        }
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
    AccountSummaryRenderer(WebSocketHandler &ws_client) : ws_client(ws_client) {
        this->FetchData();
    }

    void FetchData() override
    {
        ws_client.FetchAccountSummary();
        last_refresh = std::chrono::system_clock::now();
    }

    void Render() override
    {
        ImGui::Begin("Account Summary", nullptr,
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);

        const auto &summary = ws_client.GetAccountSummary();
        if (!summary.empty())
        {
            ImGui::Spacing();

            for (const auto &metric : METRICS)
            {
                RenderMetric(summary, metric);
            }

            if (summary.contains("equity") && summary.contains("maintenance_margin"))
            {
                double equity = summary["equity"].get<double>();
                // double initial_margin = summary["initial_margin"].get<double>();
                // double maintenance_margin = summary["maintenance_margin"].get<double>();
                // double ratio = equity / (initial_margin + maintenance_margin);
                double net_profit = equity - 100;

                ImGui::Separator();
                ImGui::Text("Account Health:");

                ImVec4 health_color;
                if (net_profit > 0.0)
                    health_color = ImVec4(0.000f, 1.000f, 0.441f, 1.000f);
                else
                    health_color = ImVec4(0.853f, 0.050f, 0.310f, 1.000f);

                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, health_color);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
                ImGui::ProgressBar(static_cast<float>(1),
                                   ImVec2(-1, 0),
                                   std::to_string(net_profit).c_str());
                ImGui::PopStyleColor(2);
            }
            RenderLastUpdateTime(ws_client.GetAccountSummaryReqTime());
        }
        else
            ImGui::TextDisabled("Loading data...");

        ImGui::End();

        if (ShouldAutoRefresh())
        {
            FetchData();
        }
    }

    std::string tabName() override { return "Account Summary"; }
};
