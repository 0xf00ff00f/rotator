#include "textureatlaspage.h"

#include "pixmap.h"

#include <cassert>

namespace
{
struct Rect
{
    int x, y;
    int width, height;
};

} // namespace

struct TextureAtlasPage::Node
{
    Rect rect;
    std::unique_ptr<Node> left, right;
    bool used;
    std::optional<Rect> insert(int width, int height);
};

std::optional<Rect> TextureAtlasPage::Node::insert(int width, int height)
{
    if (used)
    {
        return std::nullopt;
    }

    if (width > rect.width || height > rect.height)
    {
        return std::nullopt;
    }

    // is this an internal node?

    if (left)
    {
        auto result = left->insert(width, height);
        if (!result)
        {
            assert(right);
            result = right->insert(width, height);
        }
        return result;
    }

    // image fits perfectly in this node?

    if (width == rect.width && height == rect.height)
    {
        used = true;
        return rect;
    }

    // else split this node

    const int splitX = rect.width - width;
    const int splitY = rect.height - height;
    if (splitX > splitY)
    {
        // split horizontally

        left = std::make_unique<Node>(Node{{rect.x, rect.y, width, rect.height}});
        right = std::make_unique<Node>(Node{{rect.x + width, rect.y, splitX, rect.height}});
        return left->insert(width, height);
    }
    else
    {
        // split vertically

        left = std::make_unique<Node>(Node{{rect.x, rect.y, rect.width, height}});
        right = std::make_unique<Node>(Node{{rect.x, rect.y + height, rect.width, splitY}});
        return left->insert(width, height);
    }
}

TextureAtlasPage::TextureAtlasPage(int width, int height, PixelType pixelType)
    : m_pixmap(width, height, pixelType)
    , m_tree(std::make_unique<Node>(Node{{0, 0, width, height}}))
{
}

TextureAtlasPage::~TextureAtlasPage() = default;

const Pixmap *TextureAtlasPage::pixmap() const
{
    return &m_pixmap;
}

std::optional<BoxF> TextureAtlasPage::insert(const Pixmap &pixmap)
{
    constexpr auto Margin = 1;

    if (pixmap.pixelType != m_pixmap.pixelType)
    {
        return std::nullopt;
    }

    auto rect = m_tree->insert(pixmap.width + 2 * Margin, pixmap.height + 2 * Margin);
    if (!rect)
    {
        return std::nullopt;
    }

    const auto pixelSize = pixelSizeInBytes(m_pixmap.pixelType);

    const unsigned char *src = pixmap.pixels.data();
    const auto srcSpan = pixmap.width * pixelSize;

    unsigned char *dest = m_pixmap.pixels.data() + ((rect->y + Margin) * m_pixmap.width + rect->x + Margin) * pixelSize;
    const auto destSpan = m_pixmap.width * pixelSize;

    for (int i = 0; i < pixmap.height; ++i)
    {
        std::copy(src, src + srcSpan, dest);
        src += srcSpan;
        dest += destSpan;
    }

    const auto textureSize = glm::vec2(m_pixmap.width, m_pixmap.height);
    const auto uvMin = glm::vec2(rect->x + Margin, rect->y + Margin) / textureSize;
    const auto duv = glm::vec2(rect->width - 2 * Margin, rect->height - 2 * Margin) / textureSize;
    const auto uvMax = uvMin + duv;

    return BoxF{uvMin, uvMax};
}
