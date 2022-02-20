#pragma once

#include "noncopyable.h"
#include "pixeltype.h"
#include "pixmap.h"
#include "util.h"

#include <glm/glm.hpp>

#include <memory>
#include <optional>
#include <vector>

class TextureAtlasPage : private NonCopyable
{
public:
    TextureAtlasPage(int width, int height, PixelType pixelType);
    ~TextureAtlasPage();

    const Pixmap *pixmap() const;

    std::optional<BoxF> insert(const Pixmap &pixmap);

private:
    Pixmap m_pixmap;
    struct Node;
    std::unique_ptr<Node> m_tree;
};
