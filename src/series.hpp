#ifndef BUSPLOT_SERIES_HPP
#define BUSPLOT_SERIES_HPP

#include <glm/glm.hpp>

#include <vector>
#include <mutex>
#include <chrono>

using Clock = std::chrono::system_clock;
using TimeType = std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds>;
using Duration = std::chrono::microseconds;

struct Dot {
    long long m_Time{};
    double m_Value{};
};

class Series {
public:
    Series() = default;

    auto AddData(Dot data) -> void;

    auto GenerateVertices(size_t sourceBegin,
                          size_t sourceEnd,
                          std::vector<glm::dvec2>::iterator destBegin,
                          std::vector<glm::dvec2>::iterator destEnd,
                          std::vector<glm::vec3>::iterator destColorBegin,
                          std::vector<glm::vec3>::iterator destColorEnd,
                          double *minYOfData, double *maxYOfData) -> void;

    auto GetVerticesRange(const TimeType &beginTime, const TimeType &endTime) -> std::pair<size_t, size_t>;

    [[nodiscard]] auto Data() const -> const std::vector<Dot> & { return m_Data; }

    auto SetColor(const glm::vec3 &color) -> void;

    [[nodiscard]] auto Color() const noexcept -> glm::vec3;

private:
    std::mutex m_Mutex;
    std::vector<Dot> m_Data;
    glm::vec3 m_Color{};
};

#endif // BUSPLOT_SERIES_HPP