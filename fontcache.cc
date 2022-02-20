#include "fontcache.h"

#include "ioutil.h"
#include "pixmap.h"
#include "log.h"

FontCache::FontCache(TextureAtlas *textureAtlas)
    : m_textureAtlas(textureAtlas)
{
}

FontCache::~FontCache() = default;

bool FontCache::load(const std::string &ttfPath, int pixelHeight)
{
    auto buffer = Util::readFile(ttfPath);
    if (!buffer)
        return false;

    m_ttfBuffer = std::move(*buffer);

    int result = stbtt_InitFont(&m_font, m_ttfBuffer.data(), stbtt_GetFontOffsetForIndex(m_ttfBuffer.data(), 0));
    if (result == 0)
    {
        return false;
    }

    m_scale = stbtt_ScaleForPixelHeight(&m_font, pixelHeight);

    int ascent;
    int descent;
    int lineGap;
    stbtt_GetFontVMetrics(&m_font, &ascent, &descent, &lineGap);
    m_ascent = m_scale * ascent;
    m_descent = m_scale * descent;
    m_lineGap = m_scale * lineGap;

    m_pixelHeight = pixelHeight;

    return true;
}

const FontCache::Glyph *FontCache::getGlyph(int codepoint)
{
    auto it = m_glyphs.find(codepoint);
    if (it == m_glyphs.end())
        it = m_glyphs.insert(it, {codepoint, initializeGlyph(codepoint)});
    return it->second.get();
}

std::unique_ptr<FontCache::Glyph> FontCache::initializeGlyph(int codepoint)
{
    auto pm = m_textureAtlas->addPixmap(getCodepointPixmap(codepoint));
    if (!pm)
    {
        log("Couldn't fit glyph %d in texture atlas\n", codepoint);
        return {};
    }

    int advanceWidth, leftSideBearing;
    stbtt_GetCodepointHMetrics(&m_font, codepoint, &advanceWidth, &leftSideBearing);

    int ix0, iy0, ix1, iy1;
    stbtt_GetCodepointBitmapBox(&m_font, codepoint, m_scale, m_scale, &ix0, &iy0, &ix1, &iy1);

    assert(pm->width == ix1 - ix0);
    assert(pm->height == iy1 - iy0);

    auto glyph = std::make_unique<Glyph>();
    glyph->boundingBox = BoxI{{ix0, iy0}, {ix1, iy1}};
    glyph->advanceWidth = m_scale * advanceWidth;
    glyph->pixmap = *pm;
    return glyph;
}

Pixmap FontCache::getCodepointPixmap(int codepoint) const
{
    int ix0, iy0, ix1, iy1;
    stbtt_GetCodepointBitmapBox(&m_font, codepoint, m_scale, m_scale, &ix0, &iy0, &ix1, &iy1);

    const auto width = ix1 - ix0;
    const auto height = iy1 - iy0;

    Pixmap pm;
    pm.width = width;
    pm.height = height;
    pm.pixelType = PixelType::Grayscale;
    pm.pixels.resize(width * height);
    stbtt_MakeCodepointBitmap(&m_font, pm.pixels.data(), width, height, width, m_scale, m_scale, codepoint);

    return pm;
}
