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
    const auto viewPos = glm::vec3(0, 5, 5);
    const auto viewUp = glm::vec3(0, 1, 0);
    const auto view = glm::lookAt(viewPos, glm::vec3(0, 0, 0), viewUp);

    const auto angle = -0.2f * 0.75f * m_curTime + 1.5f;
    const auto model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0));
    const auto mvp = projection * view * model;

    glViewport(0, 0, m_width, m_height);
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    m_shapeProgram->bind();
    m_shapeProgram->setUniform(m_shapeProgram->uniformLocation("mvp"), mvp);

    m_mesh->render(GL_TRIANGLES);
}

bool Demo::initialize()
{
    struct Vertex
    {
        glm::vec3 position;
        glm::vec2 texCoord;
    };
    static const auto cubeVertices = [] {
        std::vector<Vertex> vertices;
        auto addFace = [&vertices](const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3) {
            vertices.push_back({p0, {0, 0}});
            vertices.push_back({p1, {0, 1}});
            vertices.push_back({p2, {1, 1}});

            vertices.push_back({p2, {1, 1}});
            vertices.push_back({p3, {1, 0}});
            vertices.push_back({p0, {0, 0}});
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

        assert(vertices.size() == 6 * 6);

        return vertices;
    }();
    m_mesh.reset(new Mesh);
    m_mesh->setVertexCount(6 * 6);
    m_mesh->setVertexSize(sizeof(Vertex));
    m_mesh->addVertexAttribute(3, GL_FLOAT, offsetof(Vertex, position));
    m_mesh->addVertexAttribute(2, GL_FLOAT, offsetof(Vertex, texCoord));
    m_mesh->initialize();
    m_mesh->setVertexData(cubeVertices.data());

    m_shapeProgram.reset(new ShaderProgram);
    initializeProgram(m_shapeProgram.get(), "shaders/shape.vert", "shaders/shape.frag");

    return true;
}
