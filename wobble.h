#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

class Wobble
{
public:
    explicit Wobble(float amplitude);

    void update(float elapsed);
    glm::mat4 rotation() const;

private:
    struct Wave
    {
        Wave(float amplitude)
            : amplitude(amplitude)
            , dir(glm::ballRand(1.0f))
            , phase(glm::linearRand(0.0f, 2.0f * glm::pi<float>()))
            , speed(glm::linearRand(1.0f, 3.0f))
        {
        }
        glm::mat4 eval(float t) const
        {
            const auto angle = amplitude * sinf(speed * t + phase);
            return glm::rotate(glm::mat4(1.0f), angle, dir);
        }
        float amplitude;
        glm::vec3 dir;
        float phase;
        float speed;
    };
    std::vector<Wave> m_waves;
    float m_t = 0.0f;
};
