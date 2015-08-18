#include "stdafx.h"
#include "GLObject.h"


GLObject::GLObject() {
    // Just compile the shader program
    _shaders.fragmentShader = "object_frag.glsl";
    _shaders.vertexShader = "object_vert.glsl";

    _shaders.compileShaders();
}

void GLObject::genCuboid(float length, float width, float height) {
    // Points
    point_t p0(-length * 0.5f, -width * 0.5f,  height * 0.5f);
    point_t p1( length * 0.5f, -width * 0.5f,  height * 0.5f);
    point_t p2( length * 0.5f, -width * 0.5f, -height * 0.5f);
    point_t p3(-length * 0.5f, -width * 0.5f, -height * 0.5f);

    point_t p4(-length * 0.5f, width * 0.5f,  height * 0.5f);
    point_t p5( length * 0.5f, width * 0.5f,  height * 0.5f);
    point_t p6( length * 0.5f, width * 0.5f, -height * 0.5f);
    point_t p7(-length * 0.5f, width * 0.5f, -height * 0.5f);

    // Normals
    normal_t up     (0.0f, 1.0f, 0.0f);
    normal_t down   (-up);
    normal_t front  (0.0f, 0.0f, 1.0f);
    normal_t back   (-front);
    normal_t left   (1.0f, 0.0f, 0.0f);
    normal_t right  (-left);

    // Color (white)
    color_t col(1);

    _mesh.vertices = decltype(_mesh.vertices) {
        // Bottom
        { p0, down, col, {1, 1} },
        { p1, down, col, {0, 1} },
        { p2, down, col, {0, 0} },
        { p3, down, col, {1, 0} },

        // Left
        { p7, left, col, {1, 1} },
        { p4, left, col, {0, 1} },
        { p0, left, col, {0, 0} },
        { p3, left, col, {1, 0} },

        // Front
        { p4, front, col, {1, 1} },
        { p5, front, col, {0, 1} },
        { p1, front, col, {0, 0} },
        { p0, front, col, {1, 0} },

        // Back
        { p6, back, col, {1, 1} },
        { p7, back, col, {0, 1} },
        { p3, back, col, {0, 0} },
        { p2, back, col, {1, 0} },

        // Right
        { p5, right, col, {1, 1} },
        { p6, right, col, {0, 1} },
        { p2, right, col, {0, 0} },
        { p1, right, col, {1, 0} },

        // Top
        { p7, up, col, {1, 1} },
        { p6, up, col, {0, 1} },
        { p5, up, col, {0, 0} },
        { p4, up, col, {1, 0} }
    };

    _mesh.indices = decltype(_mesh.indices) {
        // Bottom
        3 + 4 * 0, 1 + 4 * 0, 0 + 4 * 0,
        3 + 4 * 0, 2 + 4 * 0, 1 + 4 * 0,

        // Left
        3 + 4 * 1, 1 + 4 * 1, 0 + 4 * 1,
        3 + 4 * 1, 2 + 4 * 1, 1 + 4 * 1,

        // Front
        3 + 4 * 2, 1 + 4 * 2, 0 + 4 * 2,
        3 + 4 * 2, 2 + 4 * 2, 1 + 4 * 2,

        // Back
        3 + 4 * 3, 1 + 4 * 3, 0 + 4 * 3,
        3 + 4 * 3, 2 + 4 * 3, 1 + 4 * 3,

        // Right
        3 + 4 * 4, 1 + 4 * 4, 0 + 4 * 4,
        3 + 4 * 4, 2 + 4 * 4, 1 + 4 * 4,

        // Top
        3 + 4 * 5, 1 + 4 * 5, 0 + 4 * 5,
        3 + 4 * 5, 2 + 4 * 5, 1 + 4 * 5,
    };
}

void GLObject::bind() {
    // Switch to our shaders
    _shaders.use();

    if (!_shaders.namedParam("vertex").valid()) {
        throw "Vertex attribute missing from object shader!";
    }

    // Grab the vertex input reference (reference to the input structure)
    GLint vertexRef = _shaders.namedParam("vertex").ref;

    // Create & bind VAO (this encapsulates the other states)
    glGenVertexArrays(1, &_mesh.vao);
    glBindVertexArray(_mesh.vao);

    // Create the VBO
    glGenBuffers(1, &_mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, _mesh.vertices.size(), _mesh.vertices.data(), GL_STATIC_DRAW);

    // Create IBO
    glGenBuffers(1, &_mesh.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _mesh.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, _mesh.indices.size(), _mesh.indices.data(), GL_STATIC_DRAW);

    // Vertex attributes - defines the input structure
    glEnableVertexAttribArray(vertexRef);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<void*>(offsetof(vertex, pos)));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<void*>(offsetof(vertex, norm)));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<void*>(offsetof(vertex, col)));
}

void GLObject::render() {
    // Switch to our shaders
    _shaders.use();

    // TODO .. render
}