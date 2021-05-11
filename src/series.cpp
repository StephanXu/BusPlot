
#include <glm/glm.hpp>

#include <vector>
#include <mutex>
#include <chrono>

#include "series.hpp"

auto Series::AddData(Dot data) -> void {
    std::lock_guard<std::mutex> guard(m_Mutex);
    m_Data.push_back(data);
}

auto Series::SetColor(const glm::vec3 &color) -> void { m_Color = color; }

auto Series::Color() const noexcept -> glm::vec3 { return m_Color; }

auto Series::GenerateVertices(size_t sourceBegin, size_t sourceEnd,
                              std::vector<glm::dvec2>::iterator destBegin,
                              std::vector<glm::dvec2>::iterator destEnd,
                              std::vector<glm::vec3>::iterator destColorBegin,
                              std::vector<glm::vec3>::iterator destColorEnd, double *minYOfData,
                              double *maxYOfData) -> void {
    if (0 == sourceEnd - sourceBegin) {
        return;
    }
    double minY = 0.f, maxY = 0.f;
    minY = maxY = m_Data[sourceBegin].m_Value;

    auto sourceIdx = sourceBegin;
    auto vertexIt = destBegin;
    auto colorIt = destColorBegin;
    while (sourceIdx != sourceEnd && vertexIt != destEnd && colorIt != destColorEnd) {
        const auto &dot = m_Data[sourceIdx];
        vertexIt->x = static_cast<double>(dot.m_Time);
        vertexIt->y = dot.m_Value;
        *colorIt = Color();
        minY = std::min(minY, dot.m_Value);
        maxY = std::max(maxY, dot.m_Value);

        ++vertexIt;
        ++colorIt;
        ++sourceIdx;
    }
    if (minYOfData) {
        *minYOfData = minY;
    }
    if (maxYOfData) {
        *maxYOfData = maxY;
    }
}

auto Series::GetVerticesRange(const TimeType &beginTime, const TimeType &endTime) -> std::pair<size_t, size_t> {
    std::lock_guard<std::mutex> guard(m_Mutex);
    const auto rangeBegin = std::lower_bound(
            m_Data.begin(), m_Data.end(), beginTime,
            [](const Dot &lhs, const TimeType &rhs) {
                return lhs.m_Time < std::chrono::duration_cast<Duration>(rhs.time_since_epoch()).count();
            });
    const auto rangeEnd = std::upper_bound(
            m_Data.begin(), m_Data.end(), endTime,
            [](const TimeType &lhs, const Dot &rhs) {
                return std::chrono::duration_cast<Duration>(lhs.time_since_epoch()).count() < rhs.m_Time;
            });
    if (rangeBegin == m_Data.end()) {
        return {0, 0}; ///< No data;
    }
    return {rangeBegin - m_Data.begin(), rangeEnd - m_Data.begin()};
}
