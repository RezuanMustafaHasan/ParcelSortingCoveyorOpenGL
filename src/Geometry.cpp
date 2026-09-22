#include "Geometry.h"

#include <cmath>
#include <utility>

Mesh::Mesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices) {
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(float)), vertices.data(), GL_STATIC_DRAW);

    if (!indices.empty()) {
        indexed_ = true;
        count_ = static_cast<GLsizei>(indices.size());
        glGenBuffers(1, &ebo_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)), indices.data(), GL_STATIC_DRAW);
    } else {
        count_ = static_cast<GLsizei>(vertices.size() / 6);
    }

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

Mesh::~Mesh() {
    if (ebo_) glDeleteBuffers(1, &ebo_);
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (vao_) glDeleteVertexArrays(1, &vao_);
}

Mesh::Mesh(Mesh&& other) noexcept { *this = std::move(other); }
Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this != &other) {
        vao_ = other.vao_; vbo_ = other.vbo_; ebo_ = other.ebo_;
        count_ = other.count_; indexed_ = other.indexed_;
        other.vao_ = other.vbo_ = other.ebo_ = 0;
    }
    return *this;
}

void Mesh::draw() const {
    glBindVertexArray(vao_);
    if (indexed_) glDrawElements(GL_TRIANGLES, count_, GL_UNSIGNED_INT, nullptr);
    else glDrawArrays(GL_TRIANGLES, 0, count_);
    glBindVertexArray(0);
}

Mesh Mesh::cube() {
    const std::vector<float> v = {
        -0.5f,-0.5f, 0.5f, 0,0,1,  0.5f,-0.5f, 0.5f, 0,0,1,  0.5f, 0.5f, 0.5f, 0,0,1,
        -0.5f,-0.5f, 0.5f, 0,0,1,  0.5f, 0.5f, 0.5f, 0,0,1, -0.5f, 0.5f, 0.5f, 0,0,1,
         0.5f,-0.5f,-0.5f, 0,0,-1, -0.5f,-0.5f,-0.5f, 0,0,-1, -0.5f, 0.5f,-0.5f, 0,0,-1,
         0.5f,-0.5f,-0.5f, 0,0,-1, -0.5f, 0.5f,-0.5f, 0,0,-1,  0.5f, 0.5f,-0.5f, 0,0,-1,
        -0.5f,-0.5f,-0.5f,-1,0,0, -0.5f,-0.5f, 0.5f,-1,0,0, -0.5f, 0.5f, 0.5f,-1,0,0,
        -0.5f,-0.5f,-0.5f,-1,0,0, -0.5f, 0.5f, 0.5f,-1,0,0, -0.5f, 0.5f,-0.5f,-1,0,0,
         0.5f,-0.5f, 0.5f, 1,0,0,  0.5f,-0.5f,-0.5f, 1,0,0,  0.5f, 0.5f,-0.5f, 1,0,0,
         0.5f,-0.5f, 0.5f, 1,0,0,  0.5f, 0.5f,-0.5f, 1,0,0,  0.5f, 0.5f, 0.5f, 1,0,0,
        -0.5f, 0.5f, 0.5f, 0,1,0,  0.5f, 0.5f, 0.5f, 0,1,0,  0.5f, 0.5f,-0.5f, 0,1,0,
        -0.5f, 0.5f, 0.5f, 0,1,0,  0.5f, 0.5f,-0.5f, 0,1,0, -0.5f, 0.5f,-0.5f, 0,1,0,
        -0.5f,-0.5f,-0.5f, 0,-1,0, 0.5f,-0.5f,-0.5f, 0,-1,0, 0.5f,-0.5f, 0.5f, 0,-1,0,
        -0.5f,-0.5f,-0.5f, 0,-1,0, 0.5f,-0.5f, 0.5f, 0,-1,0,-0.5f,-0.5f, 0.5f, 0,-1,0
    };
    return Mesh(v);
}

Mesh Mesh::cylinder(unsigned int segments) {
    constexpr float pi = 3.14159265358979323846f;
    std::vector<float> v;
    std::vector<unsigned int> idx;
    for (unsigned int i = 0; i <= segments; ++i) {
        const float a = 2.0f * pi * static_cast<float>(i) / static_cast<float>(segments);
        const float x = std::cos(a) * 0.5f;
        const float z = std::sin(a) * 0.5f;
        v.insert(v.end(), {x,-0.5f,z, x*2.0f,0,z*2.0f, x,0.5f,z, x*2.0f,0,z*2.0f});
    }
    for (unsigned int i = 0; i < segments; ++i) {
        const unsigned int b = i * 2;
        idx.insert(idx.end(), {b,b+1,b+2, b+1,b+3,b+2});
    }
    const unsigned int bottomCenter = static_cast<unsigned int>(v.size()/6);
    v.insert(v.end(), {0,-0.5f,0, 0,-1,0});
    const unsigned int topCenter = static_cast<unsigned int>(v.size()/6);
    v.insert(v.end(), {0,0.5f,0, 0,1,0});
    for (unsigned int i = 0; i <= segments; ++i) {
        const float a = 2.0f * pi * static_cast<float>(i) / static_cast<float>(segments);
        const float x = std::cos(a)*0.5f, z = std::sin(a)*0.5f;
        v.insert(v.end(), {x,-0.5f,z, 0,-1,0, x,0.5f,z, 0,1,0});
    }
    const unsigned int capStart = bottomCenter + 2;
    for (unsigned int i = 0; i < segments; ++i) {
        idx.insert(idx.end(), {bottomCenter, capStart+i*2+2, capStart+i*2});
        idx.insert(idx.end(), {topCenter, capStart+i*2+1, capStart+i*2+3});
    }
    return Mesh(v, idx);
}
