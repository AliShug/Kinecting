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

		vertex(point_t pt)
			: pos(pt) {}

		vertex(point_t p, normal_t n, color_t c, uv_t u)
			: pos(p)
			, norm(n)
			, col(c)
			, uv(u) {}
    };

    struct mesh {
        std::vector<vertex> vertices;
        std::vector<GLuint> indices;

        GLuint vbo, ibo, vao;
    };

    GLObject();
    ~GLObject() {}

	// TEMP!
	// camera/view stuff
	glm::mat4 projectionMat, viewMat;
	// /TEMP

    // Object generation functions
    void genCuboid(float length = 1, float width = 1, float height = 1);

    // Binds the object using the current context
    void bind();

    // Renders the object using the current context
    void render();

	// Get the object's transformation
	glm::mat4 getTransform() { return _transform; }

	void setPosition(glm::vec3 pos) {
		_transform = glm::translate(glm::mat4(), pos);
	}

protected:
    // Member data
    mesh _mesh;
	glm::mat4 _transform; // <- probably want to separate this out..

    // Shader manager
    ShaderManager _shaders;
};

