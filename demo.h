#pragma once

#include "noncopyable.h"
#include "wobble.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

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
    glm::quat rotation;
    Wobble wobble = Wobble{0.125f};
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
    void update(float dt);

    int m_canvasWidth;
    int m_canvasHeight;
    float m_curTime = 0.0f;
    std::vector<std::unique_ptr<Shape>> m_shapes;
    std::unique_ptr<ShaderProgram> m_shapeProgram;
};
