#pragma once

#include <glm/glm.hpp>

#include <cstdint>
#include <string>

namespace Component {
    struct Movement {
        glm::vec2 velocity;
    };

    struct Sprite {
        glm::vec4 color;
    };

    struct Transform {
        glm::vec2 position;
        glm::vec2 rotation;
        glm::vec2 scale;
    };

    struct Tag {
        std::string name;
    };
}// namespace Component
