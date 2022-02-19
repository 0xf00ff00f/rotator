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
    Demo(int canvasWidth, int canvasHeight);
    ~Demo();

    bool initialize();
    void renderAndStep(float dt);

private:
    void render() const;

    int m_canvasWidth;
    int m_canvasHeight;
    float m_curTime = 0.0f;
    std::vector<std::unique_ptr<Shape>> m_shapes;
    std::unique_ptr<ShaderProgram> m_shapeProgram;
};
