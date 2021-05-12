#ifndef BUSPLOT_ELEMENT_HPP
#define BUSPLOT_ELEMENT_HPP

#include <fmt/format.h>
#include <glm/glm.hpp>

#include <unordered_map>
#include <string>
#include <any>

class Element {
public:
    Element() = default;

    ~Element() = default;

    auto SetPosition(const glm::vec2 &pos) -> void;

    auto SetSize(const glm::vec2 &size) -> void;

    [[nodiscard]] auto Position() const noexcept -> glm::vec2;

    [[nodiscard]] auto RelPosition() const noexcept -> glm::vec2;

    [[nodiscard]] auto Size() const noexcept -> glm::vec2;

    [[nodiscard]] auto GetModelMatrix() const noexcept -> glm::mat4;

    auto SetParent(const Element *element) noexcept -> void;

    auto SetProperty(const std::string &name, std::any value) -> void;

    template<typename T>
    auto Property(const std::string &name) const -> T {
        auto it = m_Properties.find(name);
        if (it == m_Properties.end()) {
            throw std::runtime_error(fmt::format("Can't find element property: {}", name));
        }
        return std::any_cast<T>(it->second);
    }

protected:
    std::unordered_map<std::string, std::any> m_Properties;
    glm::vec2 m_Position{};
    glm::vec2 m_Size{};

    const Element *m_Parent = nullptr;
};


#endif //BUSPLOT_ELEMENT_HPP
