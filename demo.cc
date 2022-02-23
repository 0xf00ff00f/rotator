#include "demo.h"

#include "mesh.h"
#include "shadermanager.h"
#include "uipainter.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/string_cast.hpp>

#include <GL/glew.h>

#include <random>
#include <algorithm>
#include <sstream>
#include <iomanip>

using namespace std::string_literals;

namespace
{
constexpr const auto ShapeCount = 6;
constexpr const auto ShapeSegments = 4;

constexpr const auto Columns = 3;
constexpr const auto TopMargin = 40;

constexpr const auto BackgroundColor = glm::vec3(0.75);

constexpr const auto TotalPlayTime = 120.0f;

constexpr const auto FadeOutTime = 2.0f;
constexpr const auto SuccessStateTime = 2.0f;
constexpr const auto FailStateTime = 1.0f;

constexpr const std::array<glm::imat4x4, 24> Rotations = {
    glm::imat4x4{{0, 0, -1, 0}, {0, -1, 0, 0}, {-1, 0, 0, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{0, 0, -1, 0}, {0, 1, 0, 0}, {1, 0, 0, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{0, 0, 1, 0}, {0, -1, 0, 0}, {1, 0, 0, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{0, 0, 1, 0}, {0, 1, 0, 0}, {-1, 0, 0, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{0, 0, -1, 0}, {-1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{0, 0, -1, 0}, {1, 0, 0, 0}, {0, -1, 0, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{0, 0, 1, 0}, {-1, 0, 0, 0}, {0, -1, 0, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{0, 0, 1, 0}, {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{0, -1, 0, 0}, {0, 0, -1, 0}, {1, 0, 0, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{0, -1, 0, 0}, {0, 0, 1, 0}, {-1, 0, 0, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{0, 1, 0, 0}, {0, 0, -1, 0}, {-1, 0, 0, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{0, 1, 0, 0}, {0, 0, 1, 0}, {1, 0, 0, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{0, -1, 0, 0}, {-1, 0, 0, 0}, {0, 0, -1, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{0, -1, 0, 0}, {1, 0, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{0, 1, 0, 0}, {-1, 0, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{0, 1, 0, 0}, {1, 0, 0, 0}, {0, 0, -1, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{-1, 0, 0, 0}, {0, 0, -1, 0}, {0, -1, 0, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{-1, 0, 0, 0}, {0, 0, 1, 0}, {0, 1, 0, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{1, 0, 0, 0}, {0, 0, -1, 0}, {0, 1, 0, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{1, 0, 0, 0}, {0, 0, 1, 0}, {0, -1, 0, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{-1, 0, 0, 0}, {0, -1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{-1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, -1, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{1, 0, 0, 0}, {0, -1, 0, 0}, {0, 0, -1, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}};

constexpr const char *FontName = "OpenSans_Regular.ttf";

Blocks rotated(const Blocks &shape, const glm::imat4x4 &rotation)
{
    Blocks rotated(shape.size());
    std::transform(shape.begin(), shape.end(), rotated.begin(),
                   [&rotation](const glm::ivec3 &p) { return rotation * glm::ivec4(p, 1); });
    return rotated;
}

Blocks canonicalized(const Blocks &shape)
{
    const auto origin = std::accumulate(shape.begin(), shape.end(), glm::ivec3(std::numeric_limits<int>::max()),
                                        [](const glm::ivec3 &a, const glm::ivec3 &b) { return glm::min(a, b); });
    Blocks result(shape.size());
    std::transform(shape.begin(), shape.end(), result.begin(), [&origin](const glm::ivec3 &p) { return p - origin; });
    std::sort(result.begin(), result.end(), [](const glm::ivec3 &lhs, const glm::ivec3 &rhs) {
        return std::tie(lhs.x, lhs.y, lhs.z) < std::tie(rhs.x, rhs.y, rhs.z);
    });
    return result;
}

bool sameShape(const Blocks &lhs, const Blocks &rhs)
{
    if (lhs.size() != rhs.size())
        return false;
    const Blocks base = canonicalized(lhs);
    for (const auto &rotation : Rotations)
    {
        if (base == canonicalized(rotated(rhs, rotation)))
            return true;
    }
    return false;
}

std::optional<Blocks> generateShape(std::mt19937 &generator)
{
    auto randomBit = [&generator] { return std::uniform_int_distribution<int>(0, 1)(generator); };

    auto center = glm::ivec3(0);
    unsigned direction = 2;
    int side = 1;

    Blocks blocks;
    for (size_t i = 0; i < ShapeSegments; ++i)
    {
        const auto d = 2 * side * glm::ivec3(direction >> 2, (direction >> 1) & 1, direction & 1);
        const auto l = 2 + (i & 1) + randomBit();
        for (int i = 0; i < l; ++i)
        {
            // reject self-intersecting shapes
            if (auto it = std::find(blocks.begin(), blocks.end(), center); it != blocks.end())
                return {};
            blocks.push_back(center);
            center += d;
        }
        switch (direction)
        {
        case 1:
            // 001 -> 010 100
            direction = randomBit() ? 2 : 4;
            break;
        case 2:
            // 010 -> 001 100
            direction = randomBit() ? 1 : 4;
            break;
        case 4:
            // 100 -> 001 010
            direction = randomBit() ? 1 : 2;
            break;
        default:
            assert(false);
            break;
        }
        if (randomBit())
            side = -side;
    }
    return blocks;
}

std::unique_ptr<Shape> initializeShape(const Blocks &blocks, const glm::imat4x4 &baseRotation)
{
    auto makeMesh = [&blocks](float blockScale) {
        struct Vertex
        {
            glm::vec3 position;
            glm::vec2 texCoord;
        };
        std::vector<Vertex> vertices;
        for (const auto &center : blocks)
        {
            auto addFace = [&vertices, blockScale, center = glm::vec3(center)](
                               const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3) {
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

        auto mesh = std::make_unique<Mesh>();
        mesh->setVertexCount(vertices.size());
        mesh->setVertexSize(sizeof(Vertex));
        mesh->addVertexAttribute(3, GL_FLOAT, offsetof(Vertex, position));
        mesh->addVertexAttribute(2, GL_FLOAT, offsetof(Vertex, texCoord));
        mesh->initialize();
        mesh->setVertexData(vertices.data());

        return mesh;
    };

    glm::vec3 center = std::accumulate(blocks.begin(), blocks.end(), glm::ivec3(0));
    center *= 1.0f / blocks.size();

    const auto rx = glm::rotate(glm::mat4(1), 0.25f * glm::pi<float>(), glm::vec3(1, 0, 0));
    const auto rz = glm::rotate(glm::mat4(1), 0.25f * glm::pi<float>(), glm::vec3(0, 0, 1));
    const auto rotation = glm::quat_cast(rx * rz * glm::mat4(baseRotation));

    auto shape = std::make_unique<Shape>();
    shape->blocks = blocks;
    shape->center = center;
    shape->mesh = makeMesh(1.0f);
    shape->outlineMesh = makeMesh(1.25f);
    shape->baseRotation = baseRotation;
    shape->rotation = rotation;

    return shape;
}
}

Demo::Demo(int canvasWidth, int canvasHeight)
    : m_canvasWidth(canvasWidth)
    , m_canvasHeight(canvasHeight)
    , m_shaderManager(new ShaderManager)
    , m_uiPainter(new UIPainter(m_shaderManager.get()))
    , m_shakes(ShapeCount)
{
    m_uiPainter->resize(canvasWidth, canvasHeight);
    initialize();
}

Demo::~Demo() = default;

void Demo::renderAndStep(float elapsed)
{
    render();
    update(elapsed);
}

void Demo::render() const
{
    glClearColor(BackgroundColor.r, BackgroundColor.g, BackgroundColor.b, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_state != State::Intro)
        renderShapes();

    renderUI();
}

void Demo::renderShapes() const
{
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDisable(GL_BLEND);

    m_shaderManager->useProgram(ShaderManager::Shape);

    const auto viewportWidth = m_canvasWidth / Columns;
    const auto viewportHeight = (m_canvasHeight - TopMargin) / ((m_shapes.size() + Columns - 1) / Columns);

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
        const auto viewCenter = [this, i, &shape] {
            if (m_state == State::Fail && shape->selected)
            {
                const auto &shake = m_shakes[i];
                return glm::vec3(shake.eval(m_stateTime / FailStateTime), 0);
            }
            return glm::vec3(0, 0, 0);
        }();
        const auto view = glm::lookAt(viewPos, viewCenter, viewUp);

        const auto t = glm::translate(glm::mat4(1.0f), -shape->center);

        const auto rotation = [this, i, &shape] {
            if ((m_state == State::Success || m_state == State::Result) && i == m_secondShape)
            {
                const auto targetRotation = m_shapes[m_firstShape]->rotation;
                const auto l = m_state == State::Result ? FadeOutTime : 0.5f * SuccessStateTime;
                const auto t = std::min(1.0f, m_stateTime / l);
                return glm::mix(shape->rotation, targetRotation, t);
            }
            return shape->rotation;
        }();

        const auto r = glm::mat4_cast(rotation) * shape->wobble.rotation();
        const auto model = r * t;

        const auto mvp = projection * view * model;

        m_shaderManager->setUniform(ShaderManager::ModelViewProjection, mvp);

        if (shape->selected)
        {
            auto color = [this] {
                switch (m_state)
                {
                case State::Fail:
                    return glm::vec3(1, 0, 0);
                case State::Success:
                    return glm::vec3(0, 1, 0);
                default:
                    return glm::vec3(1, 1, 0);
                }
            }();
            if (m_state == State::Result)
                color = glm::mix(color, BackgroundColor, std::min(1.0f, m_stateTime / FadeOutTime));
            m_shaderManager->setUniform(ShaderManager::MixColor, glm::vec4(color, 1));
            glDisable(GL_DEPTH_TEST);
            shape->outlineMesh->render(GL_TRIANGLES);
        }

        const auto bgAlpha = [this, i] {
            switch (m_state)
            {
            case State::Result: {
                auto alpha = std::min(1.0f, m_stateTime / FadeOutTime);
                if (i == m_firstShape || i == m_secondShape)
                    alpha *= 0.5f;
                return alpha;
            }
            case State::Success: {
                if (i != m_firstShape && i != m_secondShape)
                    return std::min(1.0f, m_stateTime / (0.5f * SuccessStateTime));
                break;
            }
            default:
                break;
            }
            return 0.0f;
        }();
        m_shaderManager->setUniform(ShaderManager::MixColor, glm::vec4(BackgroundColor, bgAlpha));
        glEnable(GL_DEPTH_TEST);
        shape->mesh->render(GL_TRIANGLES);
    }
}

void Demo::renderUI() const
{
    glViewport(0, 0, m_canvasWidth, m_canvasHeight);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_uiPainter->startPainting();

    switch (m_state)
    {
    case State::Intro:
        renderIntro();
        break;
    case State::Success:
    case State::Fail:
    case State::Playing:
        renderTimer();
        break;
    case State::Result:
        renderTimer();
        renderScore();
        break;
    default:
        break;
    }

    m_uiPainter->donePainting();
}

void Demo::renderTimer() const
{
    static const UIPainter::Font FontBig{FontName, 80};
    static const UIPainter::Font FontSmall{FontName, 40};

    const auto remaining = std::max(0, static_cast<int>((TotalPlayTime - m_playTime) * 1000));
    const auto bigText = [remaining] {
        std::stringstream ss;
        ss.fill('0');
        ss.width(2);
        ss << remaining / 1000 / 60;
        ss << ':';
        ss.fill('0');
        ss.width(2);
        ss << (remaining / 1000) % 60;
        return ss.str();
    }();
    const auto smallText = [remaining] {
        std::stringstream ss;
        ss << '.';
        ss.fill('0');
        ss.width(3);
        ss << remaining % 1000;
        return ss.str();
    }();

    const auto alpha = [this] {
        if (m_state == State::Result)
            return std::max(0.0f, 1.0f - m_stateTime / FadeOutTime);
        return 1.0f;
    }();

    m_uiPainter->setFont(FontBig);
    const auto bigAdvance = m_uiPainter->horizontalAdvance(bigText);

    m_uiPainter->setFont(FontSmall);
    const auto smallAdvance = m_uiPainter->horizontalAdvance(smallText);

    const auto totalAdvance = bigAdvance + smallAdvance;

    const auto textPos = glm::vec2(-0.5 * totalAdvance, -0.5 * m_canvasHeight + 50);

    m_uiPainter->setFont(FontBig);
    m_uiPainter->drawText(textPos, glm::vec4(0, 0, 0, alpha), 0, bigText);

    m_uiPainter->setFont(FontSmall);
    m_uiPainter->drawText(textPos + glm::vec2(bigAdvance, 0), glm::vec4(0, 0, 0, alpha), 0, smallText);
}

void Demo::renderIntro() const
{
    static const UIPainter::Font FontBig{FontName, 60};
    static const UIPainter::Font FontSmall{FontName, 40};

    const auto color = glm::vec4(0, 0, 0, 1);

    m_uiPainter->setFont(FontBig);
    drawCenteredText(glm::vec2(0, -40), color, "SELECT THE MATCHING PAIR"s);

    m_uiPainter->setFont(FontSmall);
    drawCenteredText(glm::vec2(0, 200), color, "TAP TO START"s);
}

void Demo::renderScore() const
{
    static const UIPainter::Font FontBig{FontName, 60};
    static const UIPainter::Font FontSmall{FontName, 40};

    const auto alpha = [this] {
        constexpr auto StartTime = 2.0f;
        constexpr auto FadeInTime = 1.0f;
        if (m_stateTime < StartTime)
            return 0.0f;
        return std::min(1.0f, (m_stateTime - StartTime) / FadeInTime);
    }();
    const auto color = glm::vec4(0, 0, 0, alpha);

    m_uiPainter->setFont(FontBig);

    const auto scoreText = [this] {
        std::stringstream ss;
        ss << m_score << " SHAPES ROTATED";
        return ss.str();
    }();
    drawCenteredText(glm::vec2(0, -40), color, scoreText);

    if (m_score > 0)
    {
        const auto accuracyText = [this] {
            std::stringstream ss;
            ss << "ACCURACY: ";
            ss << std::fixed;
            ss << std::setprecision(2);
            ss << m_score * 100.0f / m_attempts;
            ss << "%";
            return ss.str();
        }();
        drawCenteredText(glm::vec2(0, 20), color, accuracyText);
    }

    m_uiPainter->setFont(FontSmall);
    drawCenteredText(glm::vec2(0, 200), color, "TAP TO RETRY"s);
}

void Demo::drawCenteredText(const glm::vec2 &pos, const glm::vec4 &color, const std::string &text) const
{
    const auto advance = m_uiPainter->horizontalAdvance(text);
    m_uiPainter->drawText(pos - glm::vec2(0.5f * advance, 0.0f), color, 0, text);
}

void Demo::update(float elapsed)
{
    for (auto &shape : m_shapes)
        shape->wobble.update(elapsed);
    m_stateTime += elapsed;
    switch (m_state)
    {
    case State::Intro:
        break;
    case State::Success:
        if (m_stateTime > SuccessStateTime)
        {
            setState(State::Playing);
            initializeShapes();
        }
        break;
    case State::Fail:
        if (m_stateTime > FailStateTime)
        {
            for (auto &shape : m_shapes)
                shape->selected = false;
            m_selectedCount = 0;
            setState(State::Playing);
        }
        [[fallthrough]];
    case State::Playing:
        m_playTime += elapsed;
        if (m_playTime > TotalPlayTime)
            setState(State::Result);
        break;
    case State::Result:
        break;
    }
}

void Demo::initialize()
{
    m_score = 0;
    m_attempts = 0;
    m_playTime = 0.0f;
    initializeShapes();
}

void Demo::initializeShapes()
{
    static auto generator = [] {
        std::random_device rd;
        return std::mt19937(rd());
    }();
    m_firstShape = std::uniform_int_distribution<int>(0, ShapeCount - 2)(generator);
    m_secondShape = std::uniform_int_distribution<int>(m_firstShape + 1, ShapeCount - 1)(generator);

    m_shapes.clear();
    for (int i = 0; i < ShapeCount; ++i)
    {
        auto blocks = [this, i] {
            if (i == m_secondShape)
                return m_shapes[m_firstShape]->blocks;
            std::uniform_int_distribution<size_t> distribution(0, (1 << (3 * ShapeSegments)) - 1);
            std::optional<Blocks> blocks;
            for (;;)
            {
                blocks = generateShape(generator);
                const auto valid = [this, i, &blocks] {
                    if (!blocks)
                        return false;
                    auto it = std::find_if(m_shapes.begin(), m_shapes.end(),
                                           [&blocks](const auto &shape) { return sameShape(*blocks, shape->blocks); });
                    return it == m_shapes.end();
                }();
                if (valid)
                    break;
            }
            return *blocks;
        }();
        auto rotation = [this, i, &blocks] {
            glm::imat4x4 rotation;
            for (;;)
            {
                const auto index = std::uniform_int_distribution<int>(0, Rotations.size() - 1)(generator);
                rotation = Rotations[index];
                const auto valid = [this, i, &blocks, &rotation] {
                    if (i != m_secondShape)
                        return true;
                    const auto &firstShape = m_shapes[m_firstShape];
                    const Blocks base = canonicalized(rotated(firstShape->blocks, firstShape->baseRotation));
                    return base != canonicalized(rotated(blocks, rotation));
                }();
                if (valid)
                    break;
            }
            return rotation;
        }();
        m_shapes.push_back(initializeShape(blocks, rotation));
    }

    assert(sameShape(m_shapes[m_firstShape]->blocks, m_shapes[m_secondShape]->blocks));

    m_selectedCount = 0;
}

void Demo::handleKeyPress(Key)
{
    switch (m_state)
    {
    case State::Intro:
        setState(State::Playing);
        break;
#ifdef CHEAT
    case State::Playing: {
        if (!m_shapes[m_firstShape]->selected)
            toggleShapeSelection(m_firstShape);
        else
            toggleShapeSelection(m_secondShape);
        break;
    }
#endif
    case State::Result: {
        if (m_stateTime > FadeOutTime)
        {
            setState(State::Playing);
            initialize();
        }
        break;
    }
    default:
        break;
    }
}

void Demo::handleMouseButton(int x, int y)
{
    switch (m_state)
    {
    case State::Intro:
        setState(State::Playing);
        break;
    case State::Playing: {
        const auto shapeIndex = [this, x, y = m_canvasHeight - y] {
            const auto viewportWidth = m_canvasWidth / Columns;
            const auto viewportHeight = (m_canvasHeight - TopMargin) / ((m_shapes.size() + Columns - 1) / Columns);
            const auto row = y / viewportHeight;
            const auto col = x / viewportWidth;
            return row * Columns + col;
        }();
        toggleShapeSelection(shapeIndex);
        break;
    }
    case State::Result: {
        if (m_stateTime > FadeOutTime)
        {
            setState(State::Playing);
            initialize();
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

void Demo::toggleShapeSelection(int index)
{
    if (index >= m_shapes.size())
        return;
    auto &shape = m_shapes[index];
    if (shape->selected)
    {
        shape->selected = false;
        --m_selectedCount;
    }
    else
    {
        shape->selected = true;
        ++m_selectedCount;
        if (m_selectedCount == 2)
        {
            ++m_attempts;
            if (m_shapes[m_firstShape]->selected && m_shapes[m_secondShape]->selected)
            {
                ++m_score;
                setState(State::Success);
            }
            else
            {
                setState(State::Fail);
            }
        }
    }
}
