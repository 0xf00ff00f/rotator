#pragma once

#include "noncopyable.h"
#include "shaderprogram.h"
#include "util.h"

#include <memory>
#include <string_view>
#include <unordered_map>
#include <vector>

class TextureAtlas;
class FontCache;
class SpriteBatcher;
class ShaderManager;
class AbstractTexture;

class UIPainter : private NonCopyable
{
public:
    explicit UIPainter(ShaderManager *shaderManager);
    ~UIPainter();

    void resize(int width, int height);

    void startPainting();
    void donePainting();

    struct Font
    {
        std::string name;
        int pixelHeight;
        bool operator==(const Font &other) const { return name == other.name && pixelHeight == other.pixelHeight; }
    };
    void setFont(const Font &font);

    template<typename StringT>
    void drawText(const glm::vec2 &pos, const glm::vec4 &color, int depth, const StringT &text);

    void drawTextBox(const BoxF &box, const glm::vec4 &color, int depth, const std::string &text);

    template<typename StringT>
    float horizontalAdvance(const StringT &text);

    void drawCircle(const glm::vec2 &center, float radius, const glm::vec4 &color, int depth);
    void drawRoundedRect(const BoxF &box, float radius, const glm::vec4 &color, int depth);
    void drawThickLine(const glm::vec2 &from, const glm::vec2 &to, float thickness, const glm::vec4 &color, int depth);

    void resetTransform();
    void scale(const glm::vec2 &s);
    void scale(float sx, float sy);
    void scale(float s);
    void translate(float dx, float dy);
    void translate(const glm::vec2 &p);
    void rotate(float angle);
    void saveTransform();
    void restoreTransform();

    SpriteBatcher *spriteBatcher() const { return m_spriteBatcher.get(); }
    const FontCache *font() const { return m_font; }
    BoxF sceneBox() const { return m_sceneBox; }

    enum class VerticalAlign
    {
        Top,
        Middle,
        Bottom
    };
    void setVerticalAlign(VerticalAlign align);

    enum class HorizontalAlign
    {
        Left,
        Center,
        Right
    };
    void setHorizontalAlign(HorizontalAlign align);

private:
    struct Vertex
    {
        glm::vec2 position;
        glm::vec2 textureCoords;
    };
    void addQuad(const AbstractTexture *texture, const Vertex &v0, const Vertex &v1, const Vertex &v2, const Vertex &v3,
                 const glm::vec4 &fgColor, const glm::vec4 &bgColor, int depth);

    void updateSceneBox(int width, int height);

    struct TextRow
    {
        std::string_view text;
        float width;
    };
    std::vector<TextRow> breakTextLines(const std::string &text, float lineWidth);

    struct FontHasher
    {
        std::size_t operator()(const Font &font) const;
    };
    std::unordered_map<Font, std::unique_ptr<FontCache>, FontHasher> m_fonts;
    std::unique_ptr<SpriteBatcher> m_spriteBatcher;
    std::unique_ptr<TextureAtlas> m_textureAtlas;
    BoxF m_sceneBox = {};
    FontCache *m_font = nullptr;
    glm::mat4 m_transform;
    std::vector<glm::mat4> m_transformStack;
    VerticalAlign m_verticalAlign = VerticalAlign::Top;
    HorizontalAlign m_horizontalAlign = HorizontalAlign::Left;
};
