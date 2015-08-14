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
        { p0, down, {1, 1}, col },
        { p1, down, {0, 1}, col },
        { p2, down, {0, 0}, col },
        { p3, down, {1, 0}, col },

        // Left
        { p7, left, {1, 1}, col },
        { p4, left, {0, 1}, col },
        { p0, left, {0, 0}, col },
        { p3, left, {1, 0}, col },

        // Front
        { p4, front, {1, 1}, col },
        { p5, front, {0, 1}, col },
        { p1, front, {0, 0}, col },
        { p0, front, {1, 0}, col },

        // Back
        { p6, back, {1, 1}, col },
        { p7, back, {0, 1}, col },
        { p3, back, {0, 0}, col },
        { p2, back, {1, 0}, col },

        // Right
        { p5, right, {1, 1}, col },
        { p6, right, {0, 1}, col },
        { p2, right, {0, 0}, col },
        { p1, right, {1, 0}, col },

        // Top
        { p7, up, {1, 1}, col },
        { p6, up, {0, 1}, col },
        { p5, up, {0, 0}, col },
        { p4, up, {1, 0}, col }
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