#include "shaderprogram.h"

#include "ioutil.h"

#include <fstream>
#include <sstream>
#include <optional>
#include <memory>

#include <glm/gtc/type_ptr.hpp>

ShaderProgram::ShaderProgram()
    : m_id{glCreateProgram()}
{
}

bool ShaderProgram::addShader(GLenum type, std::string_view filename)
{
    auto source = Util::readFile(std::string(filename));
    if (!source)
    {
        std::stringstream ss;
        ss << "failed to load " << filename;
        m_log = ss.str();
        return false;
    }
    source->push_back('\0');
    return addShaderSource(type, reinterpret_cast<const GLchar *>(source->data()));
}

bool ShaderProgram::addShaderSource(GLenum type, const GLchar *source)
{
    const auto shader = glCreateShader(type);

    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        m_log.clear();
        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 1)
        {
            auto buffer = std::make_unique<char[]>(logLength);
            GLsizei dummy;
            glGetShaderInfoLog(shader, logLength, &dummy, buffer.get());
            m_log = buffer.get();
        }
        return false;
    }

    glAttachShader(m_id, shader);

    return true;
}

bool ShaderProgram::link()
{
    glLinkProgram(m_id);

    GLint status;
    glGetProgramiv(m_id, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        m_log.clear();
        GLint logLength;
        glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 1)
        {
            auto buffer = std::make_unique<char[]>(logLength);
            GLsizei dummy;
            glGetProgramInfoLog(m_id, logLength, &dummy, buffer.get());
            m_log = buffer.get();
        }
        return false;
    }

    return true;
}

void ShaderProgram::bind() const
{
    glUseProgram(m_id);
}

int ShaderProgram::uniformLocation(std::string_view name) const
{
    return glGetUniformLocation(m_id, name.data());
}

void ShaderProgram::setUniform(int location, int value) const
{
    glUniform1i(location, value);
}

void ShaderProgram::setUniform(int location, float value) const
{
    glUniform1f(location, value);
}

void ShaderProgram::setUniform(int location, const glm::vec2 &value) const
{
    glUniform2fv(location, 1, glm::value_ptr(value));
}

void ShaderProgram::setUniform(int location, const glm::vec3 &value) const
{
    glUniform3fv(location, 1, glm::value_ptr(value));
}

void ShaderProgram::setUniform(int location, const glm::vec4 &value) const
{
    glUniform4fv(location, 1, glm::value_ptr(value));
}

void ShaderProgram::setUniform(int location, const std::vector<float> &value) const
{
    glUniform1fv(location, value.size(), value.data());
}

void ShaderProgram::setUniform(int location, const std::vector<glm::vec2> &value) const
{
    glUniform2fv(location, value.size(), reinterpret_cast<const float *>(value.data()));
}

void ShaderProgram::setUniform(int location, const std::vector<glm::vec3> &value) const
{
    glUniform3fv(location, value.size(), reinterpret_cast<const float *>(value.data()));
}

void ShaderProgram::setUniform(int location, const std::vector<glm::vec4> &value) const
{
    glUniform4fv(location, value.size(), reinterpret_cast<const float *>(value.data()));
}

void ShaderProgram::setUniform(int location, const glm::mat3 &value) const
{
    glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void ShaderProgram::setUniform(int location, const glm::mat4 &value) const
{
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}
