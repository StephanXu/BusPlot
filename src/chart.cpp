#include <memory>

#include "gl.hpp"
#include "chart.hpp"

auto Chart::AddSeries(const std::string &name) -> std::shared_ptr<Series> {
    auto series = std::make_shared<Series>();
    return AddSeries(name, series)
           ? series
           : nullptr;
}

auto Chart::AddSeries(const std::string &name, const std::shared_ptr<Series> &series) -> bool {
    return m_Series.insert(std::make_pair(name, series)).second;
}

auto Chart::GetSeriesOrDefault(const std::string &name) const -> std::shared_ptr<Series> {
    auto it = m_Series.find(name);
    return it == m_Series.end() ? nullptr : it->second;
}

auto Chart::RenderPlot() -> void {
    const auto timeNow = std::chrono::time_point_cast<Duration>(Clock::now());
    double xMin = std::chrono::duration_cast<Duration>(
            (timeNow - m_TimeLimit + m_TimeZoneDiff).time_since_epoch()).count();
    double xMax = std::chrono::duration_cast<Duration>((timeNow + m_TimeZoneDiff).time_since_epoch()).count();
    xMin /= 1000000.f;
    xMax /= 1000000.f;
    ImPlot::FitNextPlotAxes(false, true);
    ImPlot::SetNextPlotLimitsX(xMin, xMax, ImGuiCond_Always);
    if (ImPlot::BeginPlot("##RealtimeGraph", nullptr, nullptr, ImVec2(-1, -1), ImPlotFlags_None,
                          ImPlotAxisFlags_Time)) {
        for (const auto &item : m_Series) {
            const auto &[seriesName, series] = item;
            auto buffer = series->GenerateDots(timeNow - m_TimeLimit, timeNow);
            for (auto &dot : buffer) {
                dot.m_Time += std::chrono::duration_cast<std::chrono::seconds>(m_TimeZoneDiff).count();
            }
            const Dot *beginPtr = &buffer.front();
            ImPlot::PlotLine(seriesName.c_str(), &beginPtr->m_Time, &beginPtr->m_Value, buffer.size(), 0, sizeof(Dot));
        }
        ImPlot::EndPlot();
    }
}

auto Chart::Sparkline(const char *id, const std::vector<Dot> &dots, const ImVec4 &col, const ImVec2 &size) -> void {
    ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, ImVec2(0, 0));
    const auto timeNow = std::chrono::time_point_cast<Duration>(Clock::now());
    double xMin = std::chrono::duration_cast<Duration>(
            (timeNow - m_TimeLimit + m_TimeZoneDiff).time_since_epoch()).count();
    double xMax = std::chrono::duration_cast<Duration>((timeNow + m_TimeZoneDiff).time_since_epoch()).count();
    xMin /= 1000000.f;
    xMax /= 1000000.f;
    ImPlot::FitNextPlotAxes(false, true);
    ImPlot::SetNextPlotLimitsX(xMin, xMax, ImGuiCond_Always);
    if (ImPlot::BeginPlot(id, nullptr, nullptr, size,
                          ImPlotFlags_CanvasOnly | ImPlotFlags_NoChild,
                          ImPlotAxisFlags_NoDecorations | ImPlotAxisFlags_Time,
                          ImPlotAxisFlags_NoDecorations)) {
        const auto &beginDot = dots.front();
        ImPlot::PushStyleColor(ImPlotCol_Line, col);
        ImPlot::PlotLine(id, &beginDot.m_Time, &beginDot.m_Value, dots.size(), 0, sizeof(Dot));
        ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.25f);
        ImPlot::PlotShaded(id, &beginDot.m_Time, &beginDot.m_Value, dots.size(), 0, 0, sizeof(Dot));
        ImPlot::PopStyleVar();
        ImPlot::PopStyleColor();
        ImPlot::EndPlot();
    }
    ImPlot::PopStyleVar();
}

auto Chart::RenderTable() -> void {
    const auto timeNow = std::chrono::time_point_cast<Duration>(Clock::now());
    const ImGuiTableFlags tableFlags = ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("##ReadtimeTable", 3, tableFlags, ImVec2(-1, 0))) {
        ImGui::TableSetupColumn("Variable", ImGuiTableColumnFlags_WidthFixed, 75.0f);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, 75.0f);
        ImGui::TableSetupColumn("Plot");
        ImGui::TableHeadersRow();
        ImPlot::PushColormap(ImPlotColormap_Cool);
        for (auto it = m_Series.cbegin(); it != m_Series.cend(); ++it) {
            const auto &[seriesName, series]=*it;
            const auto row = std::distance(m_Series.cbegin(), it);
            auto buffer = series->GenerateDots(timeNow - m_TimeLimit, timeNow);
            for (auto &dot : buffer) {
                dot.m_Time += std::chrono::duration_cast<std::chrono::seconds>(m_TimeZoneDiff).count();
            }
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", seriesName.c_str());
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%.3f", buffer.back().m_Value);
            ImGui::TableSetColumnIndex(2);
            ImGui::PushID(row);
            Sparkline("##spark", buffer, ImPlot::GetColormapColor(row), ImVec2(-1, 35));
            ImGui::PopID();
        }
        ImPlot::PopColormap();
        ImGui::EndTable();
    }
}

auto Chart::TimeLimit() const noexcept -> std::chrono::microseconds { return m_TimeLimit; }
