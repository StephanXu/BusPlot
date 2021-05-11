#ifndef BUSPLOT_ELEMENT_HPP
#define BUSPLOT_ELEMENT_HPP

#include <glm/glm.hpp>

class Element {
public:
    Element() = default;

    ~Element() = default;

    auto SetPosition(const glm::vec2 &pos) -> void;

    auto SetSize(const glm::vec2 &size) -> void;

    [[nodiscard]] auto Position() const noexcept -> glm::vec2;

    [[nodiscard]] auto RelPosition() const noexcept -> glm::vec2;

    [[nodiscard]] auto Size() const noexcept -> glm::vec2;

    auto SetParent(const Element *element) noexcept -> void;

    [[nodiscard]] auto GetModelMatrix() const noexcept -> glm::mat4;

protected:

    glm::vec2 m_Position{};
    glm::vec2 m_Size{};

    const Element *m_Parent = nullptr;
};


#endif //BUSPLOT_ELEMENT_HPP
