#include "Shader.h"

#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>

std::string Shader::readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) throw std::runtime_error("Could not open shader: " + path);
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

GLuint Shader::compile(GLenum type, const std::string& source, const std::string& label) {
    const GLuint shader = glCreateShader(type);
    const char* code = source.c_str();
    glShaderSource(shader, 1, &code, nullptr);
    glCompileShader(shader);
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(static_cast<std::size_t>(length));
        glGetShaderInfoLog(shader, length, nullptr, log.data());
        glDeleteShader(shader);
        throw std::runtime_error("Shader compile error (" + label + "): " + log.data());
    }
    return shader;
}

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
    const GLuint vertex = compile(GL_VERTEX_SHADER, readFile(vertexPath), vertexPath);
    const GLuint fragment = compile(GL_FRAGMENT_SHADER, readFile(fragmentPath), fragmentPath);
    id_ = glCreateProgram();
    glAttachShader(id_, vertex);
    glAttachShader(id_, fragment);
    glLinkProgram(id_);
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    GLint success = 0;
    glGetProgramiv(id_, GL_LINK_STATUS, &success);
    if (!success) {
        GLint length = 0;
        glGetProgramiv(id_, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(static_cast<std::size_t>(length));
        glGetProgramInfoLog(id_, length, nullptr, log.data());
        const std::string message(log.data());
        glDeleteProgram(id_);
        id_ = 0;
        throw std::runtime_error("Shader link error: " + message);
    }
}

Shader::~Shader() { if (id_) glDeleteProgram(id_); }
void Shader::use() const { glUseProgram(id_); }
void Shader::setBool(const std::string& name, bool value) const { glUniform1i(uniformLocation(name), value); }
void Shader::setInt(const std::string& name, int value) const { glUniform1i(uniformLocation(name), value); }
void Shader::setFloat(const std::string& name, float value) const { glUniform1f(uniformLocation(name), value); }
void Shader::setVec3(const std::string& name, const glm::vec3& value) const { glUniform3fv(uniformLocation(name), 1, glm::value_ptr(value)); }
void Shader::setMat4(const std::string& name, const glm::mat4& value) const { glUniformMatrix4fv(uniformLocation(name), 1, GL_FALSE, glm::value_ptr(value)); }

GLint Shader::uniformLocation(const std::string& name) const {
    const auto found = uniformLocations_.find(name);
    if (found != uniformLocations_.end()) return found->second;
    const GLint location = glGetUniformLocation(id_, name.c_str());
    uniformLocations_.emplace(name, location);
    return location;
}
