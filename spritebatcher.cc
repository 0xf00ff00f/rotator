#include "spritebatcher.h"
#include "abstracttexture.h"
#include "textureatlas.h"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>

SpriteBatcher::SpriteBatcher(ShaderManager *shaderManager)
    : m_shaderManager(shaderManager)
{
    initializeResources();
}

SpriteBatcher::~SpriteBatcher()
{
    releaseResources();
}

void SpriteBatcher::setTransformMatrix(const glm::mat4 &matrix)
{
    m_transformMatrix = matrix;
}

glm::mat4 SpriteBatcher::transformMatrix() const
{
    return m_transformMatrix;
}

void SpriteBatcher::setBatchProgram(ShaderManager::Program program)
{
    m_batchProgram = program;
}

ShaderManager::Program SpriteBatcher::batchProgram() const
{
    return m_batchProgram;
}

void SpriteBatcher::startBatch()
{
    m_quadCount = 0;
}

void SpriteBatcher::addSprite(const PackedPixmap &pixmap, const glm::vec2 &topLeft, const glm::vec2 &bottomRight,
                              const glm::vec4 &fgColor, const glm::vec4 &bgColor, int depth)
{
    const auto &p0 = topLeft;
    const auto &p1 = bottomRight;

    const auto &textureCoords = pixmap.textureCoords;
    const auto &t0 = textureCoords.min;
    const auto &t1 = textureCoords.max;

    const auto verts = QuadVerts{{{{p0.x, p0.y}, {t0.x, t0.y}, fgColor, bgColor},
                                  {{p1.x, p0.y}, {t1.x, t0.y}, fgColor, bgColor},
                                  {{p1.x, p1.y}, {t1.x, t1.y}, fgColor, bgColor},
                                  {{p0.x, p1.y}, {t0.x, t1.y}, fgColor, bgColor}}};

    addSprite(pixmap.texture, verts, depth);
}

void SpriteBatcher::addSprite(const PackedPixmap &pixmap, const glm::vec2 &topLeft, const glm::vec2 &bottomRight,
                              const glm::vec4 &color, int depth)
{
    const auto &p0 = topLeft;
    const auto &p1 = bottomRight;

    const auto &textureCoords = pixmap.textureCoords;
    const auto &t0 = textureCoords.min;
    const auto &t1 = textureCoords.max;

    const auto verts = QuadVerts{{{{p0.x, p0.y}, {t0.x, t0.y}, color, {0, 0, 0, 0}},
                                  {{p1.x, p0.y}, {t1.x, t0.y}, color, {0, 0, 0, 0}},
                                  {{p1.x, p1.y}, {t1.x, t1.y}, color, {0, 0, 0, 0}},
                                  {{p0.x, p1.y}, {t0.x, t1.y}, color, {0, 0, 0, 0}}}};

    addSprite(pixmap.texture, verts, depth);
}

void SpriteBatcher::addSprite(const AbstractTexture *texture, const QuadVerts &verts, int depth)
{
    if (m_quadCount == MaxQuadsPerBatch)
    {
        renderBatch();
        startBatch();
    }

    auto &quad = m_quads[m_quadCount++];
    quad.texture = texture;
    quad.program = m_batchProgram;
    quad.verts = verts;
    quad.depth = depth;
}

void SpriteBatcher::renderBatch() const
{
    if (m_quadCount == 0)
        return;

    static std::array<const Quad *, MaxQuadsPerBatch> sortedQuads;
    const auto quadsEnd = m_quads.begin() + m_quadCount;
    std::transform(m_quads.begin(), quadsEnd, sortedQuads.begin(), [](const Quad &quad) { return &quad; });
    const auto sortedQuadsEnd = sortedQuads.begin() + m_quadCount;
    std::stable_sort(sortedQuads.begin(), sortedQuadsEnd, [](const Quad *a, const Quad *b) {
        return std::tie(a->depth, a->texture, a->program) < std::tie(b->depth, b->texture, b->program);
    });

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBindVertexArray(m_vao);

    const AbstractTexture *currentTexture = nullptr;
    std::optional<ShaderManager::Program> currentProgram = std::nullopt;

    auto batchStart = sortedQuads.begin();
    while (batchStart != sortedQuadsEnd)
    {
        const auto *batchTexture = (*batchStart)->texture;
        const auto batchProgram = (*batchStart)->program;
        const auto batchEnd =
            std::find_if(batchStart + 1, sortedQuadsEnd, [batchTexture, batchProgram](const Quad *quad) {
                return quad->texture != batchTexture || quad->program != batchProgram;
            });

        const auto quadCount = batchEnd - batchStart;
        const auto bufferRangeSize = quadCount * GLQuadSize;

        if (!m_bufferAllocated || (m_bufferOffset + bufferRangeSize > BufferCapacity))
        {
            // orphan the old buffer and grab a new memory block
            glBufferData(GL_ARRAY_BUFFER, BufferCapacity * sizeof(GLfloat), nullptr, GL_STREAM_DRAW);
            m_bufferOffset = 0;
            m_bufferAllocated = true;
        }

        auto *data = reinterpret_cast<GLfloat *>(glMapBufferRange(GL_ARRAY_BUFFER, m_bufferOffset * sizeof(GLfloat),
                                                                  bufferRangeSize * sizeof(GLfloat),
                                                                  GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
        for (auto it = batchStart; it != batchEnd; ++it)
        {
            auto *quadPtr = *it;

            const auto &verts = quadPtr->verts;

            const auto emitVertex = [&data, &verts](int index) {
                const auto &v = verts[index];
                *data++ = v.position.x;
                *data++ = v.position.y;

                *data++ = v.textureCoords.x;
                *data++ = v.textureCoords.y;

                *data++ = v.fgColor.x;
                *data++ = v.fgColor.y;
                *data++ = v.fgColor.z;
                *data++ = v.fgColor.w;

                *data++ = v.bgColor.x;
                *data++ = v.bgColor.y;
                *data++ = v.bgColor.z;
                *data++ = v.bgColor.w;
            };

            emitVertex(0);
            emitVertex(1);
            emitVertex(2);

            emitVertex(2);
            emitVertex(3);
            emitVertex(0);
        }
        glUnmapBuffer(GL_ARRAY_BUFFER);

        if (currentTexture != batchTexture)
        {
            currentTexture = batchTexture;
            if (currentTexture)
                currentTexture->bind();
        }

        if (currentProgram != batchProgram)
        {
            currentProgram = batchProgram;
            m_shaderManager->useProgram(batchProgram);
            m_shaderManager->setUniform(ShaderManager::Uniform::ModelViewProjection, m_transformMatrix);
            if (currentTexture)
                m_shaderManager->setUniform(ShaderManager::Uniform::BaseColorTexture, 0);
        }

        glDrawArrays(GL_TRIANGLES, m_bufferOffset / GLVertexSize, quadCount * 6);

        m_bufferOffset += bufferRangeSize;
        batchStart = batchEnd;
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void SpriteBatcher::initializeResources()
{
    glGenBuffers(1, &m_vbo);
    glGenVertexArrays(1, &m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBindVertexArray(m_vao);

    // position

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<GLvoid *>(0));

    // textureCoords

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<GLvoid *>(2 * sizeof(GLfloat)));

    // fgColor

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<GLvoid *>(4 * sizeof(GLfloat)));

    // bgColor

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<GLvoid *>(8 * sizeof(GLfloat)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void SpriteBatcher::releaseResources()
{
    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);
}
