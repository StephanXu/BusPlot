#include <fmt/format.h>

#include <memory>

#include "gl.hpp"
#include "chart.hpp"

auto Chart::AddSeries(uint16_t seriesId) -> std::shared_ptr<Series> {
    auto series = std::make_shared<Series>(fmt::format("var{}", seriesId));
    return AddSeries(seriesId, series)
           ? series
           : nullptr;
}

auto Chart::AddSeries(uint16_t seriesId, const std::shared_ptr<Series> &series) -> bool {
    std::lock_guard<std::mutex> guard(m_Mutex);
    return m_Series.insert(std::make_pair(seriesId, series)).second;
}

auto Chart::GetSeriesOrDefault(uint16_t seriesId) const noexcept -> std::shared_ptr<Series> {
    auto it = m_Series.find(seriesId);
    return it == m_Series.end() ? nullptr : it->second;
}

auto Chart::GetOrAddSeries(uint16_t seriesId) -> std::shared_ptr<Series> {
    auto it = m_Series.find(seriesId);
    if (it == m_Series.end()) {
        return AddSeries(seriesId);
    }
    return it->second;
}

auto Chart::RemoveSeries(uint16_t seriesId) -> bool {
    return m_Series.erase(seriesId);
}

auto Chart::RenderPlot() -> void {
    std::lock_guard<std::mutex> guard(m_Mutex);
    auto timeLimit = m_TimeLimit.load();
    const auto timeNow = std::chrono::time_point_cast<Duration>(Clock::now());
    double xMin = std::chrono::duration_cast<Duration>(
            (timeNow - timeLimit + m_TimeZoneDiff).time_since_epoch()).count();
    double xMax = std::chrono::duration_cast<Duration>((timeNow + m_TimeZoneDiff).time_since_epoch()).count();
    xMin /= 1000000.f;
    xMax /= 1000000.f;
    ImPlot::FitNextPlotAxes(false, true);
    ImPlot::SetNextPlotLimitsX(xMin, xMax, ImGuiCond_Always);
    if (ImPlot::BeginPlot("##RealtimeGraph", nullptr, nullptr, ImVec2(-1, -1), ImPlotFlags_None,
                          ImPlotAxisFlags_Time)) {
        for (const auto &item : m_Series) {
            const auto &series = item.second;
            auto buffer = series->GenerateDots(timeNow - timeLimit, timeNow);
            if (!buffer.empty()) {
                for (auto &dot : buffer) {
                    dot.m_Time += std::chrono::duration_cast<std::chrono::seconds>(m_TimeZoneDiff).count();
                }
                const Dot *beginPtr = &buffer.front();
                ImPlot::PlotLine(series->Label().c_str(),
                                 &beginPtr->m_Time,
                                 &beginPtr->m_Value,
                                 buffer.size(),
                                 0,
                                 sizeof(Dot));
            }
        }
        ImPlot::EndPlot();
    }
}

auto Chart::Sparkline(const char *id, Series &series, const ImVec4 &col, const ImVec2 &size) -> void {
    ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, ImVec2(0, 0));
    auto timeLimit = m_TimeLimit.load();
    const auto timeNow = std::chrono::time_point_cast<Duration>(Clock::now());
    double xMin = std::chrono::duration_cast<Duration>(
            (timeNow - timeLimit + m_TimeZoneDiff).time_since_epoch()).count();
    double xMax = std::chrono::duration_cast<Duration>((timeNow + m_TimeZoneDiff).time_since_epoch()).count();
    xMin /= 1000000.f;
    xMax /= 1000000.f;
    ImPlot::FitNextPlotAxes(false, true);
    ImPlot::SetNextPlotLimitsX(xMin, xMax, ImGuiCond_Always);
    if (ImPlot::BeginPlot(id, nullptr, nullptr, size,
                          ImPlotFlags_CanvasOnly | ImPlotFlags_NoChild,
                          ImPlotAxisFlags_NoDecorations | ImPlotAxisFlags_Time,
                          ImPlotAxisFlags_NoDecorations)) {
        auto dots = series.GenerateDots(timeNow - timeLimit, timeNow);
        if (!dots.empty()) {
            for (auto &dot : dots) {
                dot.m_Time += std::chrono::duration_cast<std::chrono::seconds>(m_TimeZoneDiff).count();
            }
            const auto &beginDot = dots.front();
            ImPlot::PushStyleColor(ImPlotCol_Line, col);
            ImPlot::PlotLine(id, &beginDot.m_Time, &beginDot.m_Value, dots.size(), 0, sizeof(Dot));
            ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.25f);
            ImPlot::PlotShaded(id, &beginDot.m_Time, &beginDot.m_Value, dots.size(), 0, 0, sizeof(Dot));
            ImPlot::PopStyleVar();
            ImPlot::PopStyleColor();
        }
        ImPlot::EndPlot();
    }
    ImPlot::PopStyleVar();
}

auto Chart::RenderTable(double scale) -> void {
    std::lock_guard<std::mutex> guard(m_Mutex);
    const ImGuiTableFlags tableFlags = ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("##ReadtimeTable", 3, tableFlags, ImVec2(-1, 0))) {
        ImGui::TableSetupColumn("Variable", ImGuiTableColumnFlags_WidthFixed, 75.0f * scale);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, 75.0f * scale);
        ImGui::TableSetupColumn("Plot");
        ImGui::TableHeadersRow();
        ImPlot::PushColormap(ImPlotColormap_Cool);
        for (auto it = m_Series.cbegin(); it != m_Series.cend(); ++it) {
            const auto &series = it->second;
            const auto row = std::distance(m_Series.cbegin(), it);
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", series->Label().c_str());
            ImGui::TableSetColumnIndex(1);
            if (series->Data().empty()) {
                ImGui::Text(u8"NULL");
            } else {
                ImGui::Text("%.3f", series->Data().back().m_Value);
            }
            ImGui::TableSetColumnIndex(2);
            ImGui::PushID(row);
            Sparkline("##spark", *series, ImPlot::GetColormapColor(row), ImVec2(-1, 35.f * scale));
            ImGui::PopID();
        }
        ImPlot::PopColormap();
        ImGui::EndTable();
    }
}

auto Chart::TimeLimit() const noexcept -> std::chrono::microseconds { return m_TimeLimit; }

