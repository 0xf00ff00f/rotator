#pragma once

#include "noncopyable.h"

#include <GL/glew.h>

#include <string>
#include <vector>
#include <string_view>

#include <glm/glm.hpp>

class ShaderProgram : private NonCopyable
{
public:
    ShaderProgram();

    bool addShader(GLenum type, std::string_view path);
    bool addShaderSource(GLenum type, const GLchar *source);
    bool link();
    const std::string &log() const { return m_log; }

    void bind() const;

    int uniformLocation(std::string_view name) const;

    void setUniform(int location, int v) const;
    void setUniform(int location, float v) const;
    void setUniform(int location, const glm::vec2 &v) const;
    void setUniform(int location, const glm::vec3 &v) const;
    void setUniform(int location, const glm::vec4 &v) const;

    void setUniform(int location, const std::vector<float> &v) const;
    void setUniform(int location, const std::vector<glm::vec2> &v) const;
    void setUniform(int location, const std::vector<glm::vec3> &v) const;
    void setUniform(int location, const std::vector<glm::vec4> &v) const;

    void setUniform(int location, const glm::mat3 &mat) const;
    void setUniform(int location, const glm::mat4 &mat) const;

private:
    GLuint m_id;
    std::string m_log;
};
