#pragma once

#include "shaderprogram.h"

#include <array>
#include <memory>

class ShaderProgram;

class ShaderManager
{
public:
    ~ShaderManager();

    enum Program
    {
        Text,
        Shape,
        Circle,
        ThickLine,
        NumPrograms
    };
    void useProgram(Program program);

    enum Uniform
    {
        ModelViewProjection,
        BaseColorTexture,
        MixColor,
        NumUniforms
    };

    template<typename T>
    void setUniform(Uniform uniform, T &&value)
    {
        if (!m_currentProgram)
            return;
        const auto location = uniformLocation(uniform);
        if (location == -1)
            return;
        m_currentProgram->program->setUniform(location, std::forward<T>(value));
    }

private:
    int uniformLocation(Uniform uniform);

    struct CachedProgram
    {
        std::unique_ptr<ShaderProgram> program;
        std::array<int, Uniform::NumUniforms> uniformLocations;
    };
    std::array<std::unique_ptr<CachedProgram>, Program::NumPrograms> m_cachedPrograms;
    CachedProgram *m_currentProgram = nullptr;
};
