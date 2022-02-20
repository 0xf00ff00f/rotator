#pragma once

#include "noncopyable.h"
#include "wobble.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <memory>
#include <vector>

class Mesh;
class ShaderManager;
class UIPainter;

struct Shape
{
    size_t dna;
    glm::vec3 center;
    std::unique_ptr<Mesh> mesh;
    std::unique_ptr<Mesh> outlineMesh;
    glm::quat rotation;
    bool selected = false;
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
    std::unique_ptr<ShaderManager> m_shaderManager;
    std::unique_ptr<UIPainter> m_uiPainter;
    std::vector<std::unique_ptr<Shape>> m_shapes;
    State m_state = State::Playing;
    int m_firstShape = 0;
    int m_secondShape = 0;
    float m_stateTime = 0.0f;
};
