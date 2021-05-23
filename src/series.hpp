#ifndef BUSPLOT_SERIES_HPP
#define BUSPLOT_SERIES_HPP

#include <vector>
#include <mutex>
#include <chrono>
#include <string>

using Clock = std::chrono::system_clock;
using TimeType = std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds>;
using Duration = std::chrono::microseconds;

struct Dot {
    double m_Time{};
    double m_Value{};
};

class Series {
public:
    explicit Series(const std::string &label);

    auto AddData(Dot data) -> void;

    auto GenerateDots(const TimeType &beginTime, const TimeType &endTime) -> std::vector<Dot>;

    [[nodiscard]] auto Data() const noexcept -> const std::vector<Dot> &;

    [[nodiscard]] auto Label() const noexcept -> std::string;

    auto SetLabel(const std::string &label) -> void;

private:
    mutable std::mutex m_Mutex;
    std::vector<Dot> m_Data;
    mutable std::mutex m_LabelMutex;
    std::string m_Label;
};

#endif // BUSPLOT_SERIES_HPP