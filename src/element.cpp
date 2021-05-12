
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "element.hpp"

auto Element::SetPosition(const glm::vec2 &pos) -> void { m_Position = pos; }

auto Element::Position() const noexcept -> glm::vec2 { return m_Position; }

auto Element::SetSize(const glm::vec2 &size) -> void { m_Size = size; }

auto Element::Size() const noexcept -> glm::vec2 { return m_Size; }

auto Element::SetParent(const Element *element) noexcept -> void { m_Parent = element; }

auto Element::RelPosition() const noexcept -> glm::vec2 {
    return m_Parent ? m_Position + m_Parent->m_Position : m_Position;
}

auto Element::GetModelMatrix() const noexcept -> glm::mat4 {
    glm::mat4 model = glm::mat4(1.f);
    model = glm::translate(model, glm::vec3(RelPosition(), 0.0f));
    model = glm::scale(model, glm::vec3(Size(), 1.0f));
    return model;
}

auto Element::SetProperty(const std::string &name, std::any value) -> void {
    m_Properties.insert(std::make_pair(name, value));
}
