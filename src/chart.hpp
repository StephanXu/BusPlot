#ifndef BUSPLOT_CHART_HPP
#define BUSPLOT_CHART_HPP

#include <unordered_map>
#include <string>
#include <memory>
#include <atomic>

#include "gl.hpp"
#include "series.hpp"

class Chart {
public:
    explicit Chart() {
        const std::time_t epochPlus11h = 60 * 60 * 11;
        const int local_time = localtime(&epochPlus11h)->tm_hour;
        const int gm_time = gmtime(&epochPlus11h)->tm_hour;
        m_TimeZoneDiff = std::chrono::hours(local_time - gm_time);
    };

    auto AddSeries(uint16_t seriesId) -> std::shared_ptr<Series>;

    auto AddSeries(uint16_t seriesId, const std::shared_ptr<Series> &series) -> bool;

    template<typename T>
    auto SetTimeLimit(T timeLimit) -> void {
        m_TimeLimit = std::chrono::duration_cast<std::chrono::microseconds>(timeLimit);
    }

    [[nodiscard]] auto TimeLimit() const noexcept -> std::chrono::microseconds;

    [[nodiscard]] auto GetSeriesOrDefault(uint16_t seriesId) const noexcept -> std::shared_ptr<Series>;

    auto GetOrAddSeries(uint16_t seriesId) -> std::shared_ptr<Series>;

    auto RemoveSeries(uint16_t seriesId) -> bool;

    auto RenderPlot() -> void;

    auto RenderTable(double scale) -> void;

private:

    auto Sparkline(const char *id, Series &series, const ImVec4 &col, const ImVec2 &size) -> void;

    mutable std::mutex m_Mutex;
    std::unordered_map<int, std::shared_ptr<Series>> m_Series;
    std::chrono::hours m_TimeZoneDiff{};
    std::atomic<std::chrono::microseconds> m_TimeLimit{std::chrono::microseconds(5000000)};
};

#endif // BUSPLOT_CHART_HPP