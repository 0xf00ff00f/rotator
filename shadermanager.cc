#include "shadermanager.h"

#include "log.h"

#include <type_traits>

namespace
{
std::string shaderPath(std::string_view basename)
{
    return std::string("assets/shaders/") + std::string(basename);
}

std::unique_ptr<ShaderProgram> loadProgram(const char *vertexShader, const char *fragmentShader)
{
    auto program = std::make_unique<ShaderProgram>();
    if (!program->addShader(GL_VERTEX_SHADER, shaderPath(vertexShader)))
    {
        log("Failed to add vertex shader for program %s: %s\n", vertexShader, program->log().c_str());
        return {};
    }
    if (!program->addShader(GL_FRAGMENT_SHADER, shaderPath(fragmentShader)))
    {
        log("Failed to add fragment shader for program %s: %s\n", fragmentShader, program->log().c_str());
        return {};
    }
    if (!program->link())
    {
        log("Failed to link program: %s\n", program->log().c_str());
        return {};
    }
    return program;
}

std::unique_ptr<ShaderProgram> loadProgram(ShaderManager::Program id)
{
    struct ProgramSource
    {
        const char *vertexShader;
        const char *fragmentShader;
    };
    static const ProgramSource programSources[] = {
        {"text.vert", "text.frag"},           // Text
        {"shape.vert", "shape.frag"},         // Shape
        {"circle.vert", "circle.frag"},       // Circle
        {"thickline.vert", "thickline.frag"}, // ThickLine
    };
    static_assert(std::extent_v<decltype(programSources)> == ShaderManager::NumPrograms,
                  "expected number of programs to match");

    const auto &sources = programSources[id];
    return loadProgram(sources.vertexShader, sources.fragmentShader);
}

} // namespace

ShaderManager::~ShaderManager() = default;

void ShaderManager::useProgram(Program id)
{
    auto &cachedProgram = m_cachedPrograms[id];
    if (!cachedProgram)
    {
        cachedProgram.reset(new CachedProgram);
        cachedProgram->program = loadProgram(id);
        auto &uniforms = cachedProgram->uniformLocations;
        std::fill(uniforms.begin(), uniforms.end(), -1);
    }
    if (cachedProgram.get() == m_currentProgram)
    {
        return;
    }
    if (cachedProgram->program)
    {
        cachedProgram->program->bind();
    }
    m_currentProgram = cachedProgram.get();
}

int ShaderManager::uniformLocation(Uniform id)
{
    if (!m_currentProgram || !m_currentProgram->program)
    {
        return -1;
    }
    auto location = m_currentProgram->uniformLocations[id];
    if (location == -1)
    {
        static constexpr const char *uniformNames[] = {
            // clang-format off
            "mvp",
            "baseColorTexture",
            "mixColor",
            // clang-format on
        };
        static_assert(std::extent_v<decltype(uniformNames)> == NumUniforms, "expected number of uniforms to match");

        location = m_currentProgram->program->uniformLocation(uniformNames[id]);
        m_currentProgram->uniformLocations[id] = location;
    }
    return location;
}
