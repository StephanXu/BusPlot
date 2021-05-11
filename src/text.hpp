#ifndef BUSPLOT_TEXT_HPP
#define BUSPLOT_TEXT_HPP

#include <glm/glm.hpp>

#include <unordered_map>
#include <mutex>
#include <vector>
#include <memory>

#include "shader.hpp"
#include "gl.hpp"

class TextRender {
public:
    explicit TextRender(std::shared_ptr<Shader> shader);

    ~TextRender();

    auto Initialize(const std::vector<unsigned char> &fontFaceFileBuffer, unsigned int fontSize) -> void;

    auto Initialize(std::istream& fontFaceFile, unsigned int fontSize) -> void;

    auto Initialize(const std::string &fontFaceFilename, unsigned int fontSize) -> void;

    auto GetTextBlockSize(const std::string &text, GLdouble scale = 1.f) -> glm::dvec2;

    auto RenderText(const std::string &text,
                    const glm::dvec2 &pos,
                    const glm::vec3 &color,
                    GLdouble scale = 1.f) -> void;

private:

    /// Holds all state information relevant to a character as loaded using FreeType
    struct Character {
        GLuint TextureID;   ///< ID handle of the glyph texture
        glm::ivec2 Size;    ///< Size of glyph
        glm::ivec2 Bearing;  ///< Offset from baseline to left/top of glyph
        GLuint Advance;    ///< Horizontal offset to advance to next glyph
    };

    auto CleanTextures() -> void;

    std::mutex m_Mutex{};
    std::unordered_map<GLchar, Character> m_Characters;
    std::shared_ptr<Shader> m_Shader = nullptr;

    GLuint m_VAO = 0;
    GLuint m_VBO = 0;
    GLuint m_TextureBuffer = 0;
};

#endif // BUSPLOT_TEXT_HPP