#pragma once

#include "noncopyable.h"

#include <glm/glm.hpp>

#include <memory>
#include <vector>

class Mesh;
class ShaderProgram;

struct BoundingBox
{
    glm::vec3 min;
    glm::vec3 max;
};

struct Shape
{
    BoundingBox boundingBox;
    std::unique_ptr<Mesh> mesh;
};

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
    std::unique_ptr<Shape> m_shape;
    std::unique_ptr<ShaderProgram> m_shapeProgram;
};
