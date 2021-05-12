#include <fmt/format.h>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>

#include "gl.hpp"
#include "series.hpp"
#include "shader.hpp"
#include "element.hpp"
#include "text.hpp"
#include "axis.hpp"

Axis::Axis(std::shared_ptr<Shader> shader, std::shared_ptr<TextRender> textRender)
        : m_Shader(std::move(shader)), m_TextRender(std::move(textRender)) {
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    glGenBuffers(1, &m_VertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
    glVertexAttribLPointer(0, 2, GL_DOUBLE, sizeof(glm::dvec2), nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &m_ColorBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_ColorBuffer);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
    glEnableVertexAttribArray(1);

    const std::time_t epochPlus11h = 60 * 60 * 11;
    const int local_time = localtime(&epochPlus11h)->tm_hour;
    const int gm_time = gmtime(&epochPlus11h)->tm_hour;
    m_TimeZoneHourDiff = local_time - gm_time;
}

auto Axis::Render() -> void {
    const auto gridColor = Property<glm::vec3>("gridColor");
    const auto axisColor = Property<glm::vec3>("axisColor");
    const auto textColor = Property<glm::vec3>("textColor");
    const auto[xBegin, xEnd] = m_AxisXRange;
    const auto[yBegin, yEnd] = m_AxisYRange;
    const std::vector<glm::dvec2> borderVertices = {
            {xBegin, yBegin},
            {xEnd,   yBegin},
            {xEnd,   yEnd},
            {xBegin, yEnd},
            {xBegin, yBegin}
    };
    glm::dmat4 scale = glm::dmat4(1.f);
    scale = glm::scale(scale, glm::dvec3(1.f / static_cast<double>(xEnd - xBegin), 1.f / (yEnd - yBegin), 1.0f));
    scale = glm::translate(scale, glm::dvec3(static_cast<double>(-xBegin), -yBegin, 0.0f));
    glm::dmat4 model = GetModelMatrix();

    const size_t xIntervalCount = (xEnd - xBegin) / m_AxisXInterval;
    const size_t yIntervalCount = static_cast<size_t>((yEnd - yBegin) / m_AxisYInterval);
    std::vector<glm::dvec2> vertices = borderVertices;
    vertices.reserve(borderVertices.size() + xIntervalCount * 2 + yIntervalCount * 2);
    std::vector<glm::vec3> colors(vertices.size());
    std::fill(colors.begin(), colors.begin() + borderVertices.size(), axisColor);
    std::fill(colors.begin() + borderVertices.size(), colors.end(), gridColor);
    for (long long x = xBegin + m_AxisXInterval - xBegin % m_AxisXInterval; x < xEnd; x += m_AxisXInterval) {
        vertices.emplace_back(x, yBegin);
        vertices.emplace_back(x, yEnd);
        colors.push_back(gridColor);
        colors.push_back(gridColor);
        const long long MILLISECOND = std::chrono::duration_cast<Duration>(std::chrono::milliseconds(1)).count();
        const long long SECOND = std::chrono::duration_cast<Duration>(std::chrono::seconds(1)).count();
        const long long MINUTE = std::chrono::duration_cast<Duration>(std::chrono::minutes(1)).count();;
        const long long HOUR = std::chrono::duration_cast<Duration>(std::chrono::hours(1)).count();
        const long long DAY = std::chrono::duration_cast<Duration>(std::chrono::hours(24)).count();
        glm::dvec4 textPos = model * scale * glm::dvec4(x, yBegin, 0.f, 1.f);
        const std::string text = fmt::format("{:02}:{:02}:{:02}.{:03}",
                                             x % DAY / HOUR + m_TimeZoneHourDiff,
                                             x % HOUR / MINUTE,
                                             x % MINUTE / SECOND,
                                             x % SECOND / MILLISECOND);
        auto textBlockSize = m_TextRender->GetTextBlockSize(text);
        textPos.x -= textBlockSize.x / 2.f;
        textPos.y -= textBlockSize.y + 10.f;
        m_TextRender->RenderText(text, glm::dvec2(textPos.x, textPos.y), textColor);
    }

    for (double y = std::floor(yBegin); y < yEnd; y += m_AxisYInterval) {
        vertices.emplace_back(xBegin, y);
        vertices.emplace_back(xEnd, y);
        colors.push_back(gridColor);
        colors.push_back(gridColor);
        glm::dvec4 textPos = model * scale * glm::dvec4(xBegin, y, 0.f, 1.f);
        const std::string text = fmt::format("{}", y);
        auto textBlockSize = m_TextRender->GetTextBlockSize(text);
        textPos.x -= textBlockSize.x + 10.f;
        textPos.y -= textBlockSize.y / 2.f;
        m_TextRender->RenderText(text, glm::dvec2(textPos.x, textPos.y), textColor);
    }

    m_Shader->Activate();
    m_Shader->SetMatrix4d("model", model);
    m_Shader->SetMatrix4d("scale", scale);

    glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::dvec2) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, m_ColorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * colors.size(), colors.data(), GL_STATIC_DRAW);

    glBindVertexArray(m_VAO);
    glDrawArrays(GL_LINES, borderVertices.size(), vertices.size() - 4);
    glDrawArrays(GL_LINE_STRIP, 0, borderVertices.size());
}

auto Axis::SetAxisXRange(const long long int start, const long long int end) -> void { m_AxisXRange = {start, end}; }

auto Axis::SetAxisXInterval(const long long int interval) -> void { m_AxisXInterval = interval; }

auto Axis::SetAxisYRange(const double start, const double end) -> void { m_AxisYRange = {start, end}; }

auto Axis::SetAxisYInterval(const float interval) -> void { m_AxisYInterval = interval; }
