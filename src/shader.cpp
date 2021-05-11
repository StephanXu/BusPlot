#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <string>
#include <iostream>
#include <memory>

#include "gl.hpp"
#include "shader.hpp"


auto Shader::Activate() const -> void {
    glUseProgram(m_Program);
}

auto Shader::SetFloat(const std::string &name, GLfloat value) -> void {
    glUniform1f(GetUniformLocation(name), value);
}

auto Shader::SetDouble(const std::string &name, GLdouble value) -> void {
    glUniform1d(GetUniformLocation(name), value);
}

auto Shader::SetInteger(const std::string &name, GLint value) -> void {
    glUniform1i(GetUniformLocation(name), value);
}

auto Shader::SetVector2f(const std::string &name, GLfloat x, GLfloat y) -> void {
    glUniform2f(GetUniformLocation(name), x, y);
}

auto Shader::SetVector2f(const std::string &name, const glm::vec2 &value) -> void {
    glUniform2f(GetUniformLocation(name), value.x, value.y);
}

auto Shader::SetVector2d(const std::string &name, GLdouble x, GLdouble y) -> void {
    glUniform2d(GetUniformLocation(name), x, y);
}

auto Shader::SetVector2d(const std::string &name, const glm::dvec2 &value) -> void {
    glUniform2d(GetUniformLocation(name), value.x, value.y);
}

auto Shader::SetVector3f(const std::string &name, GLfloat x, GLfloat y, GLfloat z) -> void {
    glUniform3f(GetUniformLocation(name), x, y, z);
}

auto Shader::SetVector3f(const std::string &name, const glm::vec3 &value) -> void {
    glUniform3f(GetUniformLocation(name), value.x, value.y, value.z);
}

auto Shader::SetVector3d(const std::string &name, GLdouble x, GLdouble y, GLdouble z) -> void {
    glUniform3d(GetUniformLocation(name), x, y, z);
}

auto Shader::SetVector3d(const std::string &name, const glm::dvec3 &value) -> void {
    glUniform3d(GetUniformLocation(name), value.x, value.y, value.z);
}

auto Shader::SetVector4f(const std::string &name, GLfloat x, GLfloat y, GLfloat z, GLfloat w) -> void {
    glUniform4f(GetUniformLocation(name), x, y, z, w);
}

auto Shader::SetVector4f(const std::string &name, const glm::vec4 &value) -> void {
    glUniform4f(GetUniformLocation(name), value.x, value.y, value.z, value.w);
}

auto Shader::SetVector4d(const std::string &name, GLdouble x, GLdouble y, GLdouble z, GLdouble w) -> void {
    glUniform4d(GetUniformLocation(name), x, y, z, w);
}

auto Shader::SetVector4d(const std::string &name, const glm::dvec4 &value) -> void {
    glUniform4d(GetUniformLocation(name), value.x, value.y, value.z, value.w);
}

auto Shader::SetMatrix4(const std::string &name, const glm::mat4 &matrix) -> void {
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
}

auto Shader::SetMatrix4d(const std::string &name, const glm::dmat4 &matrix) -> void {
    glUniformMatrix4dv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
}

auto Shader::GetUniformLocation(const std::string &name) -> GLint {
    auto it = m_Location.find(name);
    if (it == m_Location.end()) {
        auto location = glGetUniformLocation(m_Program, name.c_str());
        if (-1 == location) {
            throw std::runtime_error(fmt::format("Uniform {} dose not exist.", name));
        }
        it = m_Location.insert(std::make_pair(name, location)).first;
    }
    return it->second;
}

Shader::Shader(const GLuint program) : m_Program(program) {}

ShaderLoader::ShaderLoader() {
    m_Program = glCreateProgram();
}

auto ShaderLoader::AddShader(GLenum shaderType, const std::string &shaderContent) -> ShaderLoader & {
    const GLchar *content = shaderContent.c_str();
    auto shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &content, nullptr);
    glCompileShader(shader);
    glAttachShader(m_Program, shader);
    CheckCompileErrors(shader, false);
    return *this;
}

auto ShaderLoader::AddShader(GLenum shaderType, std::istream &is) -> ShaderLoader & {
    std::string str(std::istreambuf_iterator<char>{is},
                    std::istreambuf_iterator<char>{});
    return AddShader(shaderType, str);
}

auto ShaderLoader::AddShaderFromFile(GLenum shaderType, const std::string &filename) -> ShaderLoader & {
    std::ifstream ifs(filename, std::ios::in);
    if (!ifs.is_open()) {
        throw std::runtime_error(fmt::format("Open shader {} failed", filename));
    }
    return AddShader(shaderType, ifs);
}

auto ShaderLoader::Done() const -> std::shared_ptr<Shader> {
    glLinkProgram(m_Program);
    CheckCompileErrors(m_Program, true);
    return std::shared_ptr<Shader>(new Shader(m_Program));
}

auto ShaderLoader::CheckCompileErrors(GLuint object, bool isLinking) const -> void {
    GLint success;
    GLchar infoLog[1024];
    if (isLinking) {
        glGetProgramiv(object, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(object, 1024, nullptr, infoLog);
            spdlog::critical("Shader: Link-time error: {}", infoLog);
        }
    } else {
        glGetShaderiv(object, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(object, 1024, nullptr, infoLog);
            spdlog::critical("Shader: Compile-time error: {}", infoLog);
        }
    }
}