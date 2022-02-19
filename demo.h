#pragma once

#include "noncopyable.h"

#include <glm/glm.hpp>

#include <memory>
#include <vector>

class Mesh;
class ShaderProgram;

class Demo : private NonCopyable
{
public:
    Demo(int width, int height);
    ~Demo();

    bool initialize();
    void renderAndStep(float dt);

private:
    void render() const;

    int m_width;
    int m_height;
    float m_curTime = 0.0f;
    std::unique_ptr<Mesh> m_mesh;
    std::unique_ptr<ShaderProgram> m_shapeProgram;
};
