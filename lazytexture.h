#pragma once

#include "abstracttexture.h"
#include "texture.h"

struct Pixmap;

class LazyTexture : public AbstractTexture
{
public:
    explicit LazyTexture(const Pixmap *pixmap);

    void markDirty();

    void bind() const;

    const Pixmap *pixmap() const;

private:
    const Pixmap *m_pixmap;
    Texture m_texture;
    mutable bool m_dirty;
};
