#pragma once

#include <glm/glm.hpp>

class Shake
{
public:
    Shake();

    glm::vec2 eval(float t) const;

private:
    float m_phase;
    glm::vec2 m_direction;
};
