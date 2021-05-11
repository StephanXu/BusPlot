#include <glm/gtc/matrix_transform.hpp>
#include <cmrc/cmrc.hpp>

CMRC_DECLARE(resources);

#include <memory>
#include <sstream>

#include "gl.hpp"
#include "chart.hpp"
#include "text.hpp"
#include "shader.hpp"


auto Chart::MakeChart(std::shared_ptr<Shader> shader,
                      std::shared_ptr<Shader> textShader,
                      const std::string &fontFace,
                      bool isFontFaceEmbedded) -> std::shared_ptr<Chart> {
    auto chart = std::make_shared<Chart>(shader);
    auto textRender = std::make_shared<TextRender>(std::move(textShader));
    const unsigned int fontSize = 36;
    if (isFontFaceEmbedded) {
        auto fs = cmrc::resources::get_filesystem();
        auto file = fs.open(fontFace);
        textRender->Initialize(std::vector<unsigned char>(file.begin(), file.end()), fontSize);
    } else {
        textRender->Initialize(fontFace, fontSize);
    }
    auto axis = std::make_shared<Axis>(shader, textRender);
    chart->SetAxis(axis);
    return chart;
}

Chart::Chart(std::shared_ptr<Shader> seriesShader)
        : m_SeriesShader(std::move(seriesShader)) {
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
}

auto Chart::AddSeries(const std::string &name) -> std::shared_ptr<Series> {
    auto series = std::make_shared<Series>();
    return AddSeries(name, series)
           ? series
           : nullptr;
}

auto Chart::AddSeries(const std::string &name, const std::shared_ptr<Series> &series) -> bool {
    return m_Series.insert(std::make_pair(name, series)).second;
}

auto Chart::SetAxis(const std::shared_ptr<Axis> &axis) -> void {
    if (m_Axis) {
        m_Axis->SetParent(nullptr);
    }
    m_Axis = axis;
    m_Axis->SetParent(this);
}

auto Chart::GetAxis() -> std::shared_ptr<Axis> { return m_Axis; }

auto Chart::GetSeriesOrDefault(const std::string &name) const -> std::shared_ptr<Series> {
    auto it = m_Series.find(name);
    return it == m_Series.end() ? nullptr : it->second;
}

auto Chart::Render() -> void {
    const Duration timeLimit = std::chrono::duration_cast<Duration>(std::chrono::seconds(5));
    const TimeType timeNow = std::chrono::time_point_cast<Duration>(Clock::now());

    std::vector<std::pair<size_t, size_t>> ranges;
    ranges.reserve(m_Series.size());
    size_t verticesCount = 0;
    for (const auto &item : m_Series) {
        auto &series = item.second;
        auto range = series->GetVerticesRange(timeNow - timeLimit, timeNow);
        ranges.push_back(range);
        verticesCount += range.second - range.first;
    }
    std::vector<glm::dvec2> vertices(verticesCount);
    std::vector<glm::vec3> colors(verticesCount);
    double minY = 0.f;
    double maxY = 0.f;
    const long long minX = std::chrono::duration_cast<Duration>((timeNow - timeLimit).time_since_epoch()).count();
    const long long maxX = std::chrono::duration_cast<Duration>(timeNow.time_since_epoch()).count();
    {
        auto verticesIterator = vertices.begin();
        auto colorsIterator = colors.begin();
        auto rangeIterator = ranges.cbegin();
        for (const auto &item : m_Series) {
            auto &series = item.second;
            auto[beginIdx, endIdx] = *(rangeIterator++);
            double minYOfData = 0.f, maxYOfData = 0.f;
            series->GenerateVertices(beginIdx, endIdx,
                                     verticesIterator, vertices.end(),
                                     colorsIterator, colors.end(),
                                     &minYOfData, &maxYOfData);
            minY = std::min(minY, minYOfData);
            maxY = std::max(maxY, maxYOfData);
            verticesIterator += static_cast<long>(endIdx - beginIdx);
            colorsIterator += static_cast<long>(endIdx - beginIdx);
        }
    }

    glm::dmat4 scale = glm::dmat4(1.f);
    scale = glm::scale(scale, glm::dvec3(1.f / static_cast<double>(maxX - minX), 1.f / (maxY - minY), 1.0f));
    scale = glm::translate(scale, glm::dvec3(static_cast<double>(-minX), -minY, 0.0f));

    m_Axis->SetAxisXInterval(1000 * 700);
    m_Axis->SetAxisXRange(minX, maxX);
    m_Axis->SetAxisYInterval(8.f);
    m_Axis->SetAxisYRange(minY, maxY);
    m_Axis->SetPosition({0, 0});
    m_Axis->SetSize(m_Size);
    m_Axis->Render();

    m_SeriesShader->Activate();
    m_SeriesShader->SetMatrix4d("model", GetModelMatrix());
    m_SeriesShader->SetMatrix4d("scale", scale);

    glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::dvec2) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, m_ColorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * colors.size(), colors.data(), GL_DYNAMIC_DRAW);

    glBindVertexArray(m_VAO);
    int startPos = 0;
    for (const auto &range : ranges) {
        int count = static_cast<int>(range.second - range.first);
        glDrawArrays(GL_LINE_STRIP, startPos, count);
        startPos += count;
    }
}
