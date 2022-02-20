#pragma once

#include "noncopyable.h"

class AbstractTexture : private NonCopyable
{
public:
    virtual ~AbstractTexture() = default;

    virtual void bind() const = 0;
};
