#include "shake.h"

#include <glm/gtc/random.hpp>
#include <glm/gtc/constants.hpp>

Shake::Shake()
    : m_phase(glm::linearRand(0.0f, 2.0f * glm::pi<float>()))
    , m_direction(glm::normalize(glm::vec2(glm::linearRand(-0.25f, 0.25f), 1.0f)))
{
}

glm::vec2 Shake::eval(float t) const
{
    constexpr auto Amplitude = 10.0f;
    const auto damp = std::exp(-5.0f * t);
    const auto offset = Amplitude * damp * std::sin(30.0f * t + m_phase);
    const auto dir = glm::normalize(m_direction + glm::diskRand(0.25f));
    return offset * dir;
}
