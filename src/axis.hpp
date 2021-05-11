#ifndef BUSPLOT_AXIS_HPP
#define BUSPLOT_AXIS_HPP

#include <memory>

#include "gl.hpp"
#include "shader.hpp"
#include "element.hpp"
#include "text.hpp"

class Axis : public Element {
public:
    explicit Axis(std::shared_ptr<Shader> shader,
                  std::shared_ptr<TextRender> textRender);

    auto Render() -> void;

    auto SetAxisXRange(const long long start, const long long end) -> void;

    auto SetAxisXInterval(const long long interval) -> void;

    auto SetAxisYRange(const double start, const double end) -> void;

    auto SetAxisYInterval(const float interval) -> void;

private:

    GLuint m_VAO = 0;
    GLuint m_VertexBuffer = 0;
    GLuint m_ColorBuffer = 0;

    std::shared_ptr<Shader> m_Shader;
    std::shared_ptr<TextRender> m_TextRender;

    std::pair<long long, long long> m_AxisXRange;
    std::pair<double, double> m_AxisYRange;
    long long m_AxisXInterval = 1000 * 1000 * 1;
    double m_AxisYInterval = 0.0;

    int m_TimeZoneHourDiff = 0;
};

#endif // BUSPLOT_AXIS_HPP