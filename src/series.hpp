#ifndef BUSPLOT_SERIES_HPP
#define BUSPLOT_SERIES_HPP

#include <vector>
#include <mutex>
#include <chrono>

using Clock = std::chrono::system_clock;
using TimeType = std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds>;
using Duration = std::chrono::microseconds;

struct Dot {
    double m_Time{};
    double m_Value{};
};

class Series {
public:
    Series() = default;

    auto AddData(Dot data) -> void;

    auto GenerateDots(const TimeType &beginTime, const TimeType &endTime) -> std::vector<Dot>;

    [[nodiscard]] auto Data() const -> const std::vector<Dot> & { return m_Data; }

private:
    std::mutex m_Mutex;
    std::vector<Dot> m_Data;
};

#endif // BUSPLOT_SERIES_HPP