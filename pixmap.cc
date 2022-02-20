#include "pixmap.h"

#include <stb_image.h>

#include <cassert>

Pixmap loadPixmap(const std::string &path)
{
    stbi_set_flip_vertically_on_load(1);

    int width, height, channels;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &channels, 4);
    if (!data)
        return {};
    assert(channels == 4);

    Pixmap pm;
    pm.width = width;
    pm.height = height;
    pm.pixelType = PixelType::RGBA;
    pm.pixels.assign(data, data + width * height * channels);

    stbi_image_free(data);

    return pm;
}
