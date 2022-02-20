#include "demo.h"

#include "mesh.h"
#include "shaderprogram.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/string_cast.hpp>

#include <GL/glew.h>

#include <random>
#include <cstdio>
#include <iostream>

namespace
{
bool initializeProgram(ShaderProgram *program, std::string_view vertexShader, std::string_view fragmentShader)
{
    if (!program->addShader(GL_VERTEX_SHADER, vertexShader))
    {
        fprintf(stderr, "Failed to add vertex shader %s: %s\n", std::string(vertexShader).c_str(),
                program->log().c_str());
        return false;
    }

    if (!program->addShader(GL_FRAGMENT_SHADER, fragmentShader))
    {
        std::fprintf(stderr, "Failed to add fragment shader %s: %s\n", std::string(fragmentShader).c_str(),
                     program->log().c_str());
        return false;
    }

    if (!program->link())
    {
        std::fprintf(stderr, "Failed to link shader: %s\n", program->log().c_str());
    }

    return true;
}

std::unique_ptr<Shape> initializeShape(const std::vector<int> &segments)
{
    static std::default_random_engine generator;

    auto center = glm::vec3(0);
    unsigned direction = 2;
    float side = 1;

    // 001 -> 010 100
    // 010 -> 001 100
    // 100 -> 001 010

    std::uniform_int_distribution<int> zero_or_one(0, 1);

    std::vector<glm::vec3> blocks;
    for (auto length : segments)
    {
        const auto d = 2.0f * side * glm::vec3(direction >> 2, (direction >> 1) & 1, direction & 1);
        const auto l = length + zero_or_one(generator);
        for (int i = 0; i < l; ++i)
        {
            blocks.push_back(center);
            center += d;
        }
        switch (direction)
        {
        case 1:
            direction = zero_or_one(generator) ? 2 : 4;
            break;
        case 2:
            direction = zero_or_one(generator) ? 1 : 4;
            break;
        case 4:
            direction = zero_or_one(generator) ? 1 : 2;
            break;
        default:
            assert(false);
            break;
        }
        if (zero_or_one(generator))
            side = -side;
    }

    auto makeMesh = [&blocks](float blockScale) {
        struct Vertex
        {
            glm::vec3 position;
            glm::vec2 texCoord;
        };
        std::vector<Vertex> vertices;
        for (const auto &center : blocks)
        {
            auto addFace = [&vertices, &center, blockScale](const glm::vec3 &p0, const glm::vec3 &p1,
                                                            const glm::vec3 &p2, const glm::vec3 &p3) {
                vertices.push_back({p0 * blockScale + center, {0, 0}});
                vertices.push_back({p1 * blockScale + center, {0, 1}});
                vertices.push_back({p2 * blockScale + center, {1, 1}});

                vertices.push_back({p2 * blockScale + center, {1, 1}});
                vertices.push_back({p3 * blockScale + center, {1, 0}});
                vertices.push_back({p0 * blockScale + center, {0, 0}});
            };

            // top
            addFace(glm::vec3(-1, 1, -1), glm::vec3(-1, 1, 1), glm::vec3(1, 1, 1), glm::vec3(1, 1, -1));

            // bottom
            addFace(glm::vec3(1, -1, -1), glm::vec3(1, -1, 1), glm::vec3(-1, -1, 1), glm::vec3(-1, -1, -1));

            // right
            addFace(glm::vec3(-1, -1, -1), glm::vec3(-1, -1, 1), glm::vec3(-1, 1, 1), glm::vec3(-1, 1, -1));

            // left
            addFace(glm::vec3(1, 1, -1), glm::vec3(1, 1, 1), glm::vec3(1, -1, 1), glm::vec3(1, -1, -1));

            // back
            addFace(glm::vec3(-1, -1, -1), glm::vec3(-1, 1, -1), glm::vec3(1, 1, -1), glm::vec3(1, -1, -1));

            // front
            addFace(glm::vec3(1, -1, 1), glm::vec3(1, 1, 1), glm::vec3(-1, 1, 1), glm::vec3(-1, -1, 1));
        }

        auto mesh = std::make_shared<Mesh>();
        mesh->setVertexCount(vertices.size());
        mesh->setVertexSize(sizeof(Vertex));
        mesh->addVertexAttribute(3, GL_FLOAT, offsetof(Vertex, position));
        mesh->addVertexAttribute(2, GL_FLOAT, offsetof(Vertex, texCoord));
        mesh->initialize();
        mesh->setVertexData(vertices.data());

        return mesh;
    };

    auto shapeCenter = std::accumulate(blocks.begin(), blocks.end(), glm::vec3(0));
    shapeCenter *= 1.0f / blocks.size();

    const auto rotation = [&zero_or_one] {
        static const std::initializer_list<glm::vec3> dirs = {{0, 0, 1}, {0, 1, 0}, {1, 0, 0}};
        auto r = glm::mat4(1.0f);
        for (glm::vec3 dir : dirs)
        {
            auto angle = 0.25f * glm::pi<float>();
            if (zero_or_one(generator))
                angle = -angle;
            if (zero_or_one(generator))
                dir = -dir;
            r = glm::rotate(r, angle, dir);
        }
        return glm::quat_cast(r);
    }();

    auto shape = std::make_unique<Shape>();
    shape->center = shapeCenter;
    shape->mesh = makeMesh(1.0f);
    shape->outlineMesh = makeMesh(1.25f);
    shape->rotation = rotation;

    return shape;
}

constexpr auto SuccessAnimationLength = 3.0f;
}

Demo::Demo(int canvasWidth, int canvasHeight)
    : m_canvasWidth(canvasWidth)
    , m_canvasHeight(canvasHeight)
{
}

Demo::~Demo() = default;

void Demo::renderAndStep(float elapsed)
{
    render();
    update(elapsed);
}

void Demo::render() const
{
    static const auto BackgroundColor = glm::vec3(0.75);

    glClearColor(BackgroundColor.r, BackgroundColor.g, BackgroundColor.b, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    m_shapeProgram->bind();

    constexpr auto Columns = 3;

    auto viewportWidth = m_canvasWidth / Columns;
    auto viewportHeight = m_canvasHeight / ((m_shapes.size() + Columns - 1) / Columns);

    for (size_t i = 0; i < m_shapes.size(); ++i)
    {
        const auto &shape = m_shapes[i];

        const auto viewportX = (i % Columns) * viewportWidth;
        const auto viewportY = (i / Columns) * viewportHeight;
        glViewport(viewportX, viewportY, viewportWidth, viewportHeight);

        const auto projection =
            glm::perspective(glm::radians(45.0f), static_cast<float>(viewportWidth) / viewportHeight, 0.1f, 100.f);

        const auto viewPos = glm::vec3(0, 0, -25);
        const auto viewUp = glm::vec3(0, 1, 0);
        const auto view = glm::lookAt(viewPos, glm::vec3(0, 0, 0), viewUp);

        const auto t = glm::translate(glm::mat4(1.0f), -shape->center);

        const auto rotation = [this, i, &shape] {
            if (m_state == State::Success && i == m_secondShape)
            {
                const auto targetRotation = m_shapes[m_firstShape]->rotation;
                float t = std::min(1.0f, m_stateTime / (0.5f * SuccessAnimationLength));
                return glm::mix(shape->rotation, targetRotation, t);
            }
            return shape->rotation;
        }();

        const auto r = glm::mat4_cast(rotation) * shape->wobble.rotation();
        const auto model = r * t;

        const auto mvp = projection * view * model;

        m_shapeProgram->setUniform(m_shapeProgram->uniformLocation("mvp"), mvp);

        if (shape->selected)
        {
            m_shapeProgram->setUniform(m_shapeProgram->uniformLocation("mixColor"), glm::vec4(1, 0, 0, 1));
            glDisable(GL_DEPTH_TEST);
            shape->outlineMesh->render(GL_TRIANGLES);
        }

        const auto bgAlpha = [this, i, &shape] {
            if (m_state == State::Success && i != m_firstShape && i != m_secondShape)
            {
                return std::min(1.0f, m_stateTime / (0.5f * SuccessAnimationLength));
            }
            return 0.0f;
        }();
        m_shapeProgram->setUniform(m_shapeProgram->uniformLocation("mixColor"), glm::vec4(BackgroundColor, bgAlpha));
        glEnable(GL_DEPTH_TEST);
        shape->mesh->render(GL_TRIANGLES);
    }
}

void Demo::update(float elapsed)
{
    for (auto &shape : m_shapes)
        shape->wobble.update(elapsed);
    m_stateTime += elapsed;
    if (m_state == State::Success && m_stateTime > SuccessAnimationLength)
    {
        setState(State::Playing);
        initializeShapes();
    }
}

bool Demo::initialize()
{
    m_shapeProgram.reset(new ShaderProgram);
    if (!initializeProgram(m_shapeProgram.get(), "shaders/shape.vert", "shaders/shape.frag"))
        return false;

    initializeShapes();

    return true;
}

void Demo::initializeShapes()
{
    constexpr auto ShapeCount = 6;

    const std::vector segments = {3, 3, 2, 3};

    m_shapes.clear();
    for (int i = 0; i < ShapeCount; ++i)
        m_shapes.push_back(initializeShape(segments));

    static std::default_random_engine generator;

    m_firstShape = std::uniform_int_distribution<int>(0, ShapeCount - 2)(generator);
    m_secondShape = std::uniform_int_distribution<int>(m_firstShape + 1, ShapeCount - 1)(generator);

    m_shapes[m_secondShape]->mesh = m_shapes[m_firstShape]->mesh;
    m_shapes[m_secondShape]->outlineMesh = m_shapes[m_firstShape]->outlineMesh;
    m_shapes[m_secondShape]->center = m_shapes[m_firstShape]->center;
}

void Demo::handleKeyPress(Key)
{
    switch (m_state)
    {
    case State::Playing: {
        if (!m_shapes[m_firstShape]->selected)
        {
            m_shapes[m_firstShape]->selected = true;
        }
        else
        {
            m_shapes[m_secondShape]->selected = true;
            setState(State::Success);
        }
        break;
    }
    default:
        break;
    }
}

void Demo::setState(State state)
{
    m_state = state;
    m_stateTime = 0.0f;
}
