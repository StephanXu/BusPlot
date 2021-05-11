#include <spdlog/spdlog.h>
#include <glm/glm.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <unordered_map>
#include <mutex>
#include <vector>
#include <memory>
#include <fstream>

#include "gl.hpp"
#include "shader.hpp"
#include "text.hpp"

TextRender::TextRender(std::shared_ptr<Shader> shader)
        : m_Shader(std::move(shader)) {
    std::vector<glm::dvec2> textures = {
            {0.0, 0.0},
            {0.0, 1.0},
            {1.0, 1.0},

            {0.0, 0.0},
            {1.0, 1.0},
            {1.0, 0.0}
    };

    // Configure VAO/VBO for texture quads
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glEnableVertexAttribArray(0);
    glVertexAttribLPointer(0, 2, GL_DOUBLE, sizeof(glm::dvec2), nullptr);

    glGenBuffers(1, &m_TextureBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_TextureBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::dvec2) * textures.size(), textures.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribLPointer(1, 2, GL_DOUBLE, sizeof(glm::dvec2), nullptr);
}

TextRender::~TextRender() {
    std::lock_guard<std::mutex> guard(m_Mutex);
    CleanTextures();
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_TextureBuffer);
    glDeleteVertexArrays(1, &m_VAO);
}

auto TextRender::Initialize(const std::vector<unsigned char> &fontFaceFileBuffer, unsigned int fontSize) -> void {
    std::lock_guard<std::mutex> guard(m_Mutex);
    CleanTextures();
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        throw std::runtime_error("Initialize freetype failed.");
    }
    FT_Face face;
    if (FT_New_Memory_Face(ft, fontFaceFileBuffer.data(), fontFaceFileBuffer.size(), 0, &face)) {
        throw std::runtime_error("Load font failed.");
    }
    FT_Set_Pixel_Sizes(face, 0, fontSize);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (GLubyte c = 0; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            spdlog::warn("Failed to load Glyph: {:02x}", c);
            continue;
        }
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
        );
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Now store character for later use
        const Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x)
        };
        m_Characters.insert(std::make_pair(c, character));
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

auto TextRender::Initialize(std::istream &fontFaceFile, unsigned int fontSize) -> void {
    std::vector<unsigned char> fontFaceBuffer(std::istreambuf_iterator<char>{fontFaceFile},
                                              std::istreambuf_iterator<char>());
    Initialize(fontFaceBuffer, fontSize);
}

auto TextRender::Initialize(const std::string &fontFaceFilename, unsigned int fontSize) -> void {
    std::ifstream ifs(fontFaceFilename, std::ios::in | std::ios::binary);
    if (!ifs.is_open()) {
        throw std::runtime_error("Load fontface file error.");
    }
    Initialize(ifs, fontSize);
}

auto TextRender::GetTextBlockSize(const std::string &text, GLdouble scale) -> glm::dvec2 {
    std::lock_guard<std::mutex> guard(m_Mutex);
    GLdouble x = 0.f;
    glm::dvec2 res(std::numeric_limits<double>::min(), std::numeric_limits<double>::min());
    for (const auto &c: text) {
        auto it = m_Characters.find(c);
        if (it == m_Characters.end()) {
            throw std::runtime_error("Character dose not exist");
        }
        const Character &ch = it->second;
        const GLdouble xpos = x + ch.Bearing.x * scale;
        const GLdouble ypos = (ch.Size.y - ch.Bearing.y) * scale;
        const GLdouble w = ch.Size.x;
        const GLdouble h = ch.Size.y;
        res.x = std::max(res.x, xpos + w);
        res.y = std::max(res.y, ypos + h);
        x += (ch.Advance >> 6) * scale;
    }
    return res;
}

auto TextRender::RenderText(const std::string &text,
                            const glm::dvec2 &pos,
                            const glm::vec3 &color,
                            GLdouble scale) -> void {
    std::lock_guard<std::mutex> guard(m_Mutex);
    m_Shader->Activate();
    m_Shader->SetVector3f("textColor", color);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(m_VAO);
    GLdouble x = pos.x;
    GLdouble y = pos.y;
    for (const auto &c : text) {
        auto it = m_Characters.find(c);
        if (it == m_Characters.end()) {
            throw std::runtime_error("Character dose not exist");
        }
        const Character &ch = it->second;
        const GLdouble xpos = x + ch.Bearing.x * scale;
        const GLdouble ypos = y + (ch.Size.y - ch.Bearing.y) * scale;
        const GLdouble w = ch.Size.x;
        const GLdouble h = ch.Size.y;
        std::vector<glm::dvec2> vertices = {
                {xpos,     ypos + h},
                {xpos,     ypos,},
                {xpos + w, ypos,},

                {xpos,     ypos + h},
                {xpos + w, ypos,},
                {xpos + w, ypos + h}
        };
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::dvec2) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW);
        glBindVertexArray(m_VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertices.size());
        // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
        x += (ch.Advance >> 6) * scale;
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

auto TextRender::CleanTextures() -> void {
    while (!m_Characters.empty()) {
        auto &character = m_Characters.begin()->second;
        glDeleteTextures(1, &character.TextureID);
        m_Characters.erase(m_Characters.begin());
    }
}
