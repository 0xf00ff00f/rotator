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
    std::shared_ptr<Mesh> mesh;
    glm::quat rotation;
    Wobble wobble = Wobble{0.125f};
};

enum class Key
{
    Space,
};

class Demo : private NonCopyable
{
public:
    Demo(int canvasWidth, int canvasHeight);
    ~Demo();

    bool initialize();
    void renderAndStep(float elapsed);
    void handleKeyPress(Key key);

private:
    enum class State
    {
        Playing,
        Success,
    };

    void render() const;
    void update(float elapsed);
    void initializeShapes();
    void setState(State state);

    int m_canvasWidth;
    int m_canvasHeight;
    std::vector<std::unique_ptr<Shape>> m_shapes;
    std::unique_ptr<ShaderProgram> m_shapeProgram;
    State m_state = State::Playing;
    int m_firstShape = 0;
    int m_secondShape = 0;
    float m_stateTime = 0.0f;
};
