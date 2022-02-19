#include "wobble.h"

#include <algorithm>

Wobble::Wobble(float amplitude)
{
    std::generate_n(std::back_inserter(m_waves), 3,
                    [amplitude] { return Wave(glm::linearRand(0.5f * amplitude, amplitude)); });
}

void Wobble::update(float elapsed)
{
    m_t += elapsed;
}

glm::mat4 Wobble::rotation() const
{
    auto r = glm::mat4(1);
    for (const auto &wave : m_waves)
        r *= wave.eval(m_t);
    return r;
}
