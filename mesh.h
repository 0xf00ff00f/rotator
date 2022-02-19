#pragma once

#include "noncopyable.h"

#include <GL/glew.h>

#include <vector>

class Mesh : private NonCopyable
{
public:
    using IndexType = unsigned;

    Mesh();
    ~Mesh();

    void setVertexCount(unsigned count);
    void setVertexSize(unsigned size);
    void addVertexAttribute(unsigned componentCount, GLenum type, unsigned offset);

    void initialize();
    void setVertexData(const void *data); // is this polymorphism?

    void render(GLenum primitive = GL_TRIANGLES) const;

private:
    struct VertexAttribute
    {
        unsigned componentCount;
        GLenum type;
        unsigned offset;
    };

    unsigned m_vertexCount = 0;
    unsigned m_vertexSize = 0;
    std::vector<VertexAttribute> m_attributes;
    GLuint m_vertexBuffer = 0;
    GLuint m_vertexArray = 0;
};
