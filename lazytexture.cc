#include "lazytexture.h"

#include "pixmap.h"

LazyTexture::LazyTexture(const Pixmap *pixmap)
    : m_pixmap(pixmap)
    , m_texture(pixmap->width, pixmap->height, pixmap->pixelType)
    , m_dirty(true)
{
}

void LazyTexture::markDirty()
{
    m_dirty = true;
}

void LazyTexture::bind() const
{
    if (m_dirty)
    {
        m_texture.setData(m_pixmap->pixels.data());
        m_dirty = false;
    }
    m_texture.bind();
}

const Pixmap *LazyTexture::pixmap() const
{
    return m_pixmap;
}
