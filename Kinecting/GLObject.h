#pragma once
#include "stdafx.h"
#include "ShaderManager.h"
#include "PointCloud.h"

class GLScene;

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

    enum RenderMode {
        TRIANGLES = GL_TRIANGLES,
        LINE_STRIP = GL_LINE_STRIP,
		POINTS = GL_POINTS
    };

	GLObject(const GLScene *parent, const std::string &fragShader, const std::string &vertShader);
    ~GLObject() {}

    // Object generation functions
	void genQuad(const Dim &size);
    void genCuboid(float length = 1, float width = 1, float height = 1);
    void genLine(const glm::vec3 &start, const glm::vec3 &end);
	void genPointCloud(const PointCloud &pc);

    // Binds the object using the current context
    void bind();

    // Renders the object using the current context
    void render(const glm::mat4 &vpMat);

	// Get the object's transformation
	glm::mat4 getTransform() {
		_transform = _position * _rotation;
		return _transform;
	}

	void setPosition(glm::vec3 pos) {
		_position = glm::translate(glm::mat4(), pos);
	}

	void setRotation(glm::vec3 rotate) {
		_rotation = glm::orientate4(rotate);
	}

	// Applies a transformation to the object's vertices (direct modification)
	void applyTransform(const glm::mat4 &mat);
	void applyTransform(const glm::mat3 &mat);

    // Shader manager
    ShaderManager shaders;

    // Rendering mode
    RenderMode renderMode = TRIANGLES;
	float pointSize = 1.0f;

	void hide() { _hidden = true; }
	void unhide() { _hidden = false; }

protected:
    // Member data
	bool _hidden = false;
    bool _bound = false;
	bool _ownsVAO = false;
    mesh _mesh;
	glm::mat4 _transform;
	glm::mat4 _position, _rotation;
	const GLScene *_scene;
};

