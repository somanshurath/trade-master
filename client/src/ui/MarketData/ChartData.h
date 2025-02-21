#pragma once
#include <imgui.h>
#include <chrono>
#include <string>
#include "../BaseRenderer.h"
#include "../../utils/layout/ThemeUI.h"
#include "../../network/WebSocketHandler.h"
#include <implot.h>
#include <implot_internal.h>

class ChartData
{
public:
    std::vector<double> ticks;
    std::vector<double> open;
    std::vector<double> high;
    std::vector<double> low;
    std::vector<double> close;
    std::string instrument;

    uint32_t y_min;
    uint32_t y_max;

    ChartData() = default;
    ChartData(const nlohmann::json &data, const std::string &instrument)
    {
        this->instrument = instrument;
        if (data.contains("ticks") && data["ticks"].is_array())
            ticks = data["ticks"].get<std::vector<double>>();
        if (data.contains("open") && data["open"].is_array())
            open = data["open"].get<std::vector<double>>();
        if (data.contains("high") && data["high"].is_array())
            high = data["high"].get<std::vector<double>>();
        if (data.contains("low") && data["low"].is_array())
            low = data["low"].get<std::vector<double>>();
        if (data.contains("close") && data["close"].is_array())
            close = data["close"].get<std::vector<double>>();

        if (low.size() > 0 && high.size() > 0)
        {
            y_min = *std::min_element(low.begin(), low.end());
            y_max = *std::max_element(high.begin(), high.end());
        }

        if (ticks.size() > 0)
        {
            for (auto &tick : ticks)
                tick /= 1000;
        }
    }
};

class ChartDataRenderer : public BaseRenderer
{
private:
    WebSocketHandler &ws_client;
    static constexpr int REFRESH_INTERVAL = 60;
    std::chrono::system_clock::time_point last_refresh;
    ChartData chart_data;

    // ImPlot Candlestick Plot Functions
    template <typename T>
    int BinarySearch(const T *arr, int l, int r, T x)
    {
        if (r >= l)
        {
            int mid = l + (r - l) / 2;
            if (arr[mid] == x)
                return mid;
            if (arr[mid] > x)
                return BinarySearch(arr, l, mid - 1, x);
            return BinarySearch(arr, mid + 1, r, x);
        }
        return -1;
    }
    void PlotCandlestick(const char *label_id, const double *xs, const double *opens, const double *closes, const double *lows, const double *highs, int count, bool tooltip, float width_percent, ImVec4 bullCol, ImVec4 bearCol)
    {
        // get ImGui window DrawList
        ImDrawList *draw_list = ImPlot::GetPlotDrawList();
        // calc real value width
        double half_width = count > 1 ? (xs[1] - xs[0]) * width_percent : width_percent;

        // custom tool
        if (ImPlot::IsPlotHovered() && tooltip)
        {
            ImPlotPoint mouse = ImPlot::GetPlotMousePos();
            mouse.x = ImPlot::RoundTime(ImPlotTime::FromDouble(mouse.x), ImPlotTimeUnit_Day).ToDouble();
            float tool_l = ImPlot::PlotToPixels(mouse.x - half_width * 1.5, mouse.y).x;
            float tool_r = ImPlot::PlotToPixels(mouse.x + half_width * 1.5, mouse.y).x;
            float tool_t = ImPlot::GetPlotPos().y;
            float tool_b = tool_t + ImPlot::GetPlotSize().y;
            ImPlot::PushPlotClipRect();
            draw_list->AddRectFilled(ImVec2(tool_l, tool_t), ImVec2(tool_r, tool_b), IM_COL32(128, 128, 128, 64));
            ImPlot::PopPlotClipRect();
            // find mouse location index
            int idx = BinarySearch(xs, 0, count - 1, mouse.x);
            // render tool tip (won't be affected by plot clip rect)
            if (idx != -1)
            {
                ImGui::BeginTooltip();
                char buff[32];
                std::time_t time = static_cast<std::time_t>(xs[idx]);
                std::strftime(buff, sizeof(buff), "%Y-%m-%d", std::localtime(&time));
                ImGui::Text("Day:   %s", buff);
                ImGui::Text("Open:  $%.2f", opens[idx]);
                ImGui::Text("Close: $%.2f", closes[idx]);
                ImGui::Text("Low:   $%.2f", lows[idx]);
                ImGui::Text("High:  $%.2f", highs[idx]);
                ImGui::EndTooltip();
            }
        }

        // begin plot item
        if (ImPlot::BeginItem(label_id))
        {
            // override legend icon color
            ImPlot::GetCurrentItem()->Color = IM_COL32(64, 64, 64, 255);
            // fit data if requested
            if (ImPlot::FitThisFrame())
            {
                for (int i = 0; i < count; ++i)
                {
                    ImPlot::FitPoint(ImPlotPoint(xs[i], lows[i]));
                    ImPlot::FitPoint(ImPlotPoint(xs[i], highs[i]));
                }
            }
            // render data
            for (int i = 0; i < count; ++i)
            {
                ImVec2 open_pos = ImPlot::PlotToPixels(xs[i] - half_width, opens[i]);
                ImVec2 close_pos = ImPlot::PlotToPixels(xs[i] + half_width, closes[i]);
                ImVec2 low_pos = ImPlot::PlotToPixels(xs[i], lows[i]);
                ImVec2 high_pos = ImPlot::PlotToPixels(xs[i], highs[i]);
                ImU32 color = ImGui::GetColorU32(opens[i] > closes[i] ? bearCol : bullCol);
                draw_list->AddLine(low_pos, high_pos, color);
                draw_list->AddRectFilled(open_pos, close_pos, color);
            }

            // end plot item
            ImPlot::EndItem();
        }
    }

    void PlotChart(ChartData &chart_data)
    {
        const double *dates = chart_data.ticks.data();
        const double *opens = chart_data.open.data();
        const double *closes = chart_data.close.data();
        const double *lows = chart_data.low.data();
        const double *highs = chart_data.high.data();
        const int count = chart_data.ticks.size();
        const char *instrument = chart_data.instrument.c_str();
        static bool tooltip = true;
        static ImVec4 bullCol = ImVec4(0.000f, 1.000f, 0.441f, 1.000f);
        static ImVec4 bearCol = ImVec4(0.853f, 0.050f, 0.310f, 1.000f);

        if (ImPlot::BeginPlot("##Candlestick Chart", ImVec2(-1, -1)))
        {
            ImPlot::SetupAxes(nullptr, nullptr, 0, ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit);
            ImPlot::SetupAxesLimits(dates[0], dates[count - 1], chart_data.y_min, chart_data.y_max);
            ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
            ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, dates[0], dates[count - 1]);
            ImPlot::SetupAxisZoomConstraints(ImAxis_X1, 60 * 60 * 24, static_cast<double>(dates[count - 1] - dates[0]));
            ImPlot::SetupAxisFormat(ImAxis_Y1, "$%.0f");
            PlotCandlestick(instrument, dates, opens, closes, lows, highs, count, tooltip, 0.25f, bullCol, bearCol);
            ImPlot::EndPlot();
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
    ChartDataRenderer(WebSocketHandler &ws_client) : ws_client(ws_client)
    {
        this->FetchData();
        last_refresh = std::chrono::system_clock::now();
    }

    void FetchData() override
    {
        time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        time_t start_time = now - 7 * 24 * 60 * 60;
        ws_client.FetchChartData("BTC-PERPETUAL", start_time * 1000, now * 1000, "60");
        nlohmann::json chart_data_json = ws_client.GetChartData();
        chart_data = ChartData(chart_data_json, "BTC-PERPETUAL");
    }

    void Render() override
    {
        ImGui::Begin("Chart Data", nullptr, ImGuiWindowFlags_NoTitleBar);

        if (chart_data.ticks.empty())
        {
            FetchData();
            last_refresh = std::chrono::system_clock::now();
            ImGui::Text("No data available");
            ImGui::End();
            return;
        }
        PlotChart(chart_data);

        if (ShouldAutoRefresh())
        {
            FetchData();
            last_refresh = std::chrono::system_clock::now();
        }

        ImGui::End();
    }

    std::string tabName() override { return "Chart Data"; }
    std::vector<double> ticks;
    std::vector<double> open;
    std::vector<double> high;
    std::vector<double> low;
    std::vector<double> close;
};
