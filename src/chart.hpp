#ifndef BUSPLOT_CHART_HPP
#define BUSPLOT_CHART_HPP

#include <unordered_map>
#include <string>
#include <memory>

#include "gl.hpp"
#include "element.hpp"
#include "shader.hpp"
#include "series.hpp"
#include "axis.hpp"

class Chart : public Element {
public:
    static auto MakeChart(std::shared_ptr<Shader> shader,
                          std::shared_ptr<Shader> textShader,
                          const std::string& fontFace,
                          bool isFontFaceEmbedded = true) -> std::shared_ptr<Chart>;

    explicit Chart(std::shared_ptr<Shader> seriesShader);

    auto AddSeries(const std::string &name) -> std::shared_ptr<Series>;

    auto AddSeries(const std::string &name, const std::shared_ptr<Series> &series) -> bool;

    auto SetAxis(const std::shared_ptr<Axis> &axis) -> void;

    [[nodiscard]] auto GetAxis() -> std::shared_ptr<Axis>;

    [[nodiscard]] auto GetSeriesOrDefault(const std::string &name) const -> std::shared_ptr<Series>;

    auto Render() -> void;

private:
    std::unordered_map<std::string, std::shared_ptr<Series>> m_Series;
    std::shared_ptr<Axis> m_Axis = nullptr;

    std::shared_ptr<Shader> m_SeriesShader = nullptr;

    GLuint m_VAO = 0;
    GLuint m_VertexBuffer = 0;
    GLuint m_ColorBuffer = 0;
};

#endif // BUSPLOT_CHART_HPP