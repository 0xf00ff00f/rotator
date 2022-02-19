#include "mesh.h"

#include <cassert>

namespace
{
struct VAOBinder : NonCopyable
{
    explicit VAOBinder(GLuint vao) { glBindVertexArray(vao); }
    ~VAOBinder() { glBindVertexArray(0); }
};

} // namespace

Mesh::Mesh() = default;

Mesh::~Mesh()
{
    glDeleteBuffers(1, &m_vertexBuffer);
    glDeleteVertexArrays(1, &m_vertexArray);
}

void Mesh::setVertexCount(unsigned count)
{
    m_vertexCount = count;
}

void Mesh::setVertexSize(unsigned size)
{
    m_vertexSize = size;
}

void Mesh::addVertexAttribute(unsigned componentCount, GLenum type, unsigned offset)
{
    m_attributes.push_back({.componentCount = componentCount, .type = type, .offset = offset});
}

void Mesh::initialize()
{
    assert(m_vertexCount > 0);
    assert(m_vertexSize > 0);
    assert(!m_attributes.empty());

    glGenBuffers(1, &m_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_vertexSize * m_vertexCount, nullptr, GL_STATIC_DRAW);

    glGenVertexArrays(1, &m_vertexArray);

    VAOBinder vaoBinder(m_vertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);

    int index = 0;
    for (const auto &attribute : m_attributes)
    {
        glEnableVertexAttribArray(index);
        glVertexAttribPointer(index, attribute.componentCount, attribute.type, GL_FALSE, m_vertexSize,
                              reinterpret_cast<GLvoid *>(attribute.offset));
        ++index;
    }
}

void Mesh::setVertexData(const void *data)
{
    assert(m_vertexBuffer != 0);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_vertexSize * m_vertexCount, data);
}

void Mesh::render(GLenum primitive) const
{
    VAOBinder vaoBinder(m_vertexArray);
    glDrawArrays(primitive, 0, m_vertexCount);
}
