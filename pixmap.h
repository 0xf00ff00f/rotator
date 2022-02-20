#pragma once

#include "pixeltype.h"

#include <string>
#include <vector>

struct Pixmap
{
    int width;
    int height;
    PixelType pixelType;
    std::vector<unsigned char> pixels;

    Pixmap()
        : width(-1)
        , height(-1)
        , pixelType(PixelType::Invalid)
    {
    }

    Pixmap(int width, int height, PixelType pixelType)
        : width(width)
        , height(height)
        , pixelType(pixelType)
        , pixels(width * height * pixelSizeInBytes(pixelType))
    {
    }

    operator bool() const { return pixelType != PixelType::Invalid; }
};

Pixmap loadPixmap(const std::string &path);
