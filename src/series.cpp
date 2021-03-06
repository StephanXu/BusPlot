
#include <vector>
#include <mutex>
#include <chrono>
#include <algorithm>

#include "series.hpp"

Series::Series(const std::string &label) : m_Label(label) {}

auto Series::AddData(Dot data) -> void {
    std::lock_guard<std::mutex> guard(m_Mutex);
    m_Data.push_back(data);
}

auto Series::GenerateDots(const TimeType &beginTime, const TimeType &endTime) -> std::vector<Dot> {
    std::lock_guard<std::mutex> guard(m_Mutex);
    const auto rangeBegin = std::lower_bound(
            m_Data.begin(), m_Data.end(), beginTime,
            [](const Dot &lhs, const TimeType &rhs) {
                return lhs.m_Time * 1000000 < std::chrono::duration_cast<Duration>(rhs.time_since_epoch()).count();
            });
    const auto rangeEnd = std::upper_bound(
            m_Data.begin(), m_Data.end(), endTime,
            [](const TimeType &lhs, const Dot &rhs) {
                return std::chrono::duration_cast<Duration>(lhs.time_since_epoch()).count() < rhs.m_Time * 1000000;
            });
    if (rangeBegin == m_Data.end()) {
        return std::vector<Dot>(); ///< No data;
    }
    return std::vector<Dot>(rangeBegin, rangeEnd);
}

auto Series::Data() const noexcept -> const std::vector<Dot> & {
    return m_Data;
}

auto Series::Label() const noexcept -> std::string {
    std::lock_guard<std::mutex> guard(m_LabelMutex);
    return m_Label;
}

auto Series::SetLabel(const std::string &label) -> void {
    std::lock_guard<std::mutex> guard(m_LabelMutex);
    m_Label = label;
}
