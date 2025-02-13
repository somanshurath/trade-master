#include <imgui.h>
#include <chrono>
#include <string>
#include "../network/WebSocketHandler.h"

struct AccountMetric
{
    const char *label;
    const char *json_key;
    const char *format;
};

class AccountSummaryRenderer
{
private:
    static constexpr size_t BUFFER_SIZE = 256;
    static constexpr int REFRESH_INTERVAL = 30;

    static constexpr AccountMetric METRICS[] = {
        {"Equity", "equity", "%.8f"},
        {"Available Funds", "available_funds", "%.8f"},
        {"Balance", "balance", "%.8f"},
        {"Initial Margin", "initial_margin", "%.8f"},
        {"Maintenance Margin", "maintenance_margin", "%.8f"}};

    char buffer[BUFFER_SIZE];
    std::chrono::system_clock::time_point last_refresh;

    void RenderMetric(const nlohmann::json &summary, const AccountMetric &metric)
    {
        if (summary.contains(metric.json_key))
        {
            double value = summary[metric.json_key].get<double>();
            snprintf(buffer, BUFFER_SIZE, "%s: %.8f", metric.label, value);
            ImGui::TextUnformatted(buffer);
        }
    }

    void RenderLastUpdateTime(const std::chrono::system_clock::time_point &req_time)
    {
        auto now = std::chrono::system_clock::now();
        auto age = std::chrono::duration_cast<std::chrono::seconds>(now - req_time).count();

        ImGui::TextColored(
            age > REFRESH_INTERVAL ? ImVec4(1.0f, 0.4f, 0.4f, 1.0f) : ImVec4(0.4f, 1.0f, 0.4f, 1.0f),
            "Last updated: %ld seconds ago",
            age);
    }

    bool ShouldAutoRefresh() const
    {
        auto now = std::chrono::system_clock::now();
        return std::chrono::duration_cast<std::chrono::seconds>(now - last_refresh).count() > REFRESH_INTERVAL;
    }

public:
    void Render(WebSocketHandler &ws_client)
    {
        // Control Panel Window
        ImGui::Begin("Trade Master Terminal", nullptr,
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);

        if (ImGui::Button("Refresh Account Summary") || ShouldAutoRefresh())
        {
            ws_client.FetchAccountSummary();
            last_refresh = std::chrono::system_clock::now();
        }

        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::TextUnformatted("Auto-refreshes every 30 seconds");
            ImGui::EndTooltip();
        }

        ImGui::End();

        // Account Summary Window
        ImGui::Begin("Account Summary", nullptr,
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);

        const auto &summary = ws_client.GetAccountSummary();
        if (!summary.empty())
        {
            RenderLastUpdateTime(ws_client.GetAccountSummaryReqTime());

            ImGui::Separator();

            // Render all metrics
            for (const auto &metric : METRICS)
            {
                RenderMetric(summary, metric);
            }

            // Add visual indicator for account health
            if (summary.contains("equity") && summary.contains("maintenance_margin"))
            {
                double equity = summary["equity"].get<double>();
                double margin = summary["maintenance_margin"].get<double>();
                double ratio = (equity - 100) / equity;

                ImGui::Separator();
                ImGui::Text("Account Health:");

                ImVec4 health_color;
                if (ratio > 0.0)
                    health_color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // Green
                else
                    health_color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red

                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, health_color);
                ImGui::ProgressBar(static_cast<float>(ratio / 2.0),
                                   ImVec2(-1, 0),
                                   std::to_string(ratio).c_str());
                ImGui::PopStyleColor(1);
            }
        }
        else
        {
            ImGui::TextDisabled("No data available");
        }

        ImGui::End();
    }
};

void RenderAccountSummary(WebSocketHandler &ws_client)
{
    static AccountSummaryRenderer renderer;
    renderer.Render(ws_client);
}
