#pragma once

#include "abstracttexture.h"
#include "pixeltype.h"

#include <GL/glew.h>
#include <string>

struct Pixmap;

class Texture : public AbstractTexture
{
public:
    explicit Texture(const Pixmap &pixmap);
    Texture(int width, int height, PixelType pixelType, const unsigned char *data = nullptr);
    ~Texture() override;

    void setData(const unsigned char *data) const;

    int width() const { return m_width; }
    int height() const { return m_height; }

    void bind() const override;

private:
    int m_width;
    int m_height;
    GLuint m_id;
    GLint m_format;
};
