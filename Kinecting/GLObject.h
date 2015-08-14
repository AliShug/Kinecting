#pragma once
#include "stdafx.h"
#include "ShaderManager.h"

class GLObject {
public:
    // Internal types
    typedef glm::vec3 point_t;
    typedef glm::vec3 normal_t;
    typedef glm::vec3 color_t;
    typedef glm::vec2 uv_t;

    struct vertex {
        point_t pos;
        normal_t norm;
        color_t col;
        uv_t uv;

        vertex()
            : pos(0)
            , norm(0)
            , col(1)
            , uv(0) {}
    };

    struct mesh {
        std::vector<vertex> vertices;
        std::vector<GLuint> indices;

        GLuint vbo, ibo, vao;
    };

    GLObject();
    ~GLObject() {}

    // Object generation functions
    void genCuboid(float length = 1, float width = 1, float height = 1);

    // Binds the object using the current context
    void bind();

    // Renders the object using the current context
    void render();

protected:
    // Member data
    mesh _mesh;

    // Shader manager
    ShaderManager _shaders;
};

