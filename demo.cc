#include "demo.h"

#include "mesh.h"
#include "shaderprogram.h"

#include <glm/gtc/matrix_transform.hpp>

#include <GL/glew.h>

#include <cstdio>

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

std::unique_ptr<Shape> initializeShape(const std::vector<glm::vec3> &blocks)
{
    struct Vertex
    {
        glm::vec3 position;
        glm::vec2 texCoord;
    };
    std::vector<Vertex> vertices;
    for (const auto &center : blocks)
    {
        auto addFace = [&vertices, &center](const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2,
                                            const glm::vec3 &p3) {
            vertices.push_back({p0 + center, {0, 0}});
            vertices.push_back({p1 + center, {0, 1}});
            vertices.push_back({p2 + center, {1, 1}});

            vertices.push_back({p2 + center, {1, 1}});
            vertices.push_back({p3 + center, {1, 0}});
            vertices.push_back({p0 + center, {0, 0}});
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

    auto mesh = std::make_unique<Mesh>();
    mesh->setVertexCount(vertices.size());
    mesh->setVertexSize(sizeof(Vertex));
    mesh->addVertexAttribute(3, GL_FLOAT, offsetof(Vertex, position));
    mesh->addVertexAttribute(2, GL_FLOAT, offsetof(Vertex, texCoord));
    mesh->initialize();
    mesh->setVertexData(vertices.data());

    BoundingBox bb;
    bb.min = glm::vec3(std::numeric_limits<float>::max());
    bb.max = glm::vec3(std::numeric_limits<float>::min());
    for (auto &vertex : vertices)
    {
        const auto &p = vertex.position;
        bb.min = glm::min(bb.min, p);
        bb.max = glm::max(bb.max, p);
    }

    auto shape = std::make_unique<Shape>();
    shape->boundingBox = bb;
    shape->mesh = std::move(mesh);

    return shape;
}
}

Demo::Demo(int width, int height)
    : m_width(width)
    , m_height(height)
{
}

Demo::~Demo() = default;

void Demo::renderAndStep(float dt)
{
    render();
    m_curTime += dt;
}

void Demo::render() const
{
    const auto projection = glm::perspective(glm::radians(45.0f), static_cast<float>(m_width) / m_height, 0.1f, 100.f);
    const auto viewPos = glm::vec3(0, 15, 15);
    const auto viewUp = glm::vec3(0, 1, 0);
    const auto view = glm::lookAt(viewPos, glm::vec3(0, 0, 0), viewUp);

    const auto center = 0.5f * (m_shape->boundingBox.min + m_shape->boundingBox.max);
    const auto t = glm::translate(glm::mat4(1.0f), -center);
    const auto angle = -1.5f * m_curTime;
    const auto r = glm::rotate(glm::mat4(1.0f), angle, glm::normalize(glm::vec3(1, 1, 1)));
    const auto model = r * t;
    const auto mvp = projection * view * model;

    glViewport(0, 0, m_width, m_height);
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    m_shapeProgram->bind();
    m_shapeProgram->setUniform(m_shapeProgram->uniformLocation("mvp"), mvp);

    m_shape->mesh->render(GL_TRIANGLES);
}

bool Demo::initialize()
{
    m_shapeProgram.reset(new ShaderProgram);
    if (!initializeProgram(m_shapeProgram.get(), "shaders/shape.vert", "shaders/shape.frag"))
        return false;

    m_shape = initializeShape({{0, 0, 0}, {0, 2, 0}, {0, 4, 0}, {0, 6, 0}, {2, 6, 0}});

    return true;
}
