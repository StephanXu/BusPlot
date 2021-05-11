#ifndef BUSPLOT_SHADER_HPP
#define BUSPLOT_SHADER_HPP

#include <glm/glm.hpp>

#include <string>
#include <unordered_map>

#include "gl.hpp"

class ShaderLoader;

class Shader {
public:
    auto Activate() const -> void;

    auto SetFloat(const std::string &name, GLfloat value) -> void;

    auto SetDouble(const std::string &name, GLdouble value) -> void;

    auto SetInteger(const std::string &name, GLint value) -> void;

    auto SetVector2f(const std::string &name, GLfloat x, GLfloat y) -> void;

    auto SetVector2f(const std::string &name, const glm::vec2 &value) -> void;

    auto SetVector2d(const std::string &name, GLdouble x, GLdouble y) -> void;

    auto SetVector2d(const std::string &name, const glm::dvec2 &value) -> void;

    auto SetVector3f(const std::string &name, GLfloat x, GLfloat y, GLfloat z) -> void;

    auto SetVector3f(const std::string &name, const glm::vec3 &value) -> void;

    auto SetVector3d(const std::string &name, GLdouble x, GLdouble y, GLdouble z) -> void;

    auto SetVector3d(const std::string &name, const glm::dvec3 &value) -> void;

    auto SetVector4f(const std::string &name, GLfloat x, GLfloat y, GLfloat z, GLfloat w) -> void;

    auto SetVector4f(const std::string &name, const glm::vec4 &value) -> void;

    auto SetVector4d(const std::string &name, GLdouble x, GLdouble y, GLdouble z, GLdouble w) -> void;

    auto SetVector4d(const std::string &name, const glm::dvec4 &value) -> void;

    auto SetMatrix4(const std::string &name, const glm::mat4 &matrix) -> void;

    auto SetMatrix4d(const std::string &name, const glm::dmat4 &matrix) -> void;

    auto GetUniformLocation(const std::string &name) -> GLint;

private:
    friend class ShaderLoader;

    explicit Shader(GLuint program);

    std::unordered_map<std::string, GLint> m_Location;
    GLuint m_Program;
};

class ShaderLoader {
public:
    explicit ShaderLoader();

    auto AddShader(GLenum shaderType, const std::string &shaderContent) -> ShaderLoader &;

    auto AddShader(GLenum shaderType, std::istream &is) -> ShaderLoader &;

    auto AddShaderFromFile(GLenum shaderType, const std::string &filename) -> ShaderLoader &;

    [[nodiscard]] auto Done() const -> std::shared_ptr<Shader>;

private:
    GLuint m_Program;

    auto CheckCompileErrors(GLuint object, bool isLinking) const -> void;
};


#endif // BUSPLOT_SHADER_HPP