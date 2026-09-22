#pragma once

#include <glad/gl.h>
#include <vector>

class Mesh {
public:
    Mesh() = default;
    Mesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices = {});
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    void draw() const;
    static Mesh cube();
    static Mesh cylinder(unsigned int segments = 24);

private:
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLuint ebo_ = 0;
    GLsizei count_ = 0;
    bool indexed_ = false;
};
