#ifndef BUSPLOT_CHART_HPP
#define BUSPLOT_CHART_HPP

#include <unordered_map>
#include <string>
#include <memory>

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

    auto AddSeries(const std::string &name) -> std::shared_ptr<Series>;

    auto AddSeries(const std::string &name, const std::shared_ptr<Series> &series) -> bool;

    template<typename T>
    auto SetTimeLimit(T timeLimit) -> void {
        m_TimeLimit = std::chrono::duration_cast<std::chrono::microseconds>(timeLimit);
    }

    [[nodiscard]] auto TimeLimit() const noexcept -> std::chrono::microseconds;

    [[nodiscard]] auto GetSeriesOrDefault(const std::string &name) const -> std::shared_ptr<Series>;

    auto RenderPlot() -> void;

    auto RenderTable() -> void;

private:

    auto Sparkline(const char *id, const std::vector<Dot> &dots, const ImVec4 &col, const ImVec2 &size) -> void;

    std::unordered_map<std::string, std::shared_ptr<Series>> m_Series;
    std::chrono::hours m_TimeZoneDiff{};
    std::chrono::microseconds m_TimeLimit{5000000};
};

#endif // BUSPLOT_CHART_HPP