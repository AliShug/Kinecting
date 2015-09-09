#include "stdafx.h"
#include "GLObject.h"
#include "GLScene.h"

using namespace glm;

mat4 GLObject::projectorVP;

GLObject::GLObject(const GLScene *parent, const std::string &fragShader, const std::string &vertShader)
	: _scene(parent) {

    // Just compile the shader program
    shaders.fragmentShader = fragShader.c_str();
    shaders.vertexShader = vertShader.c_str();

    shaders.compileShaders();
}


void GLObject::genQuad(const Dim &size) {
	int xdim = size.width;
	int ydim = size.height;
	int xverts = xdim + 1;
	int yverts = ydim + 1;

	_mesh.vertices.reserve(xverts * yverts);

	// Uniform normals + color
	normal_t front(0.0f, 0.0f, 1.0f);

	// Color (white)
	color_t col(1);

	for (int y = 0; y < yverts; y++) {
		for (int x = 0; x < xverts; x++) {
			// Generate 2D point on XY plane
			point_t pt;
			pt.x = -1.0f + (2.0f / xdim) * x;
			pt.y = -1.0f + (2.0f / ydim) * y;
			pt.z =  0.0f;

			// Generate 2D UV coord (flipped Y)
			uv_t uv;
			uv.x = (1.0f / xdim) * x;
			uv.y = 1.0f - (1.0f / ydim) * y;

			_mesh.vertices.push_back({ pt, front, col, uv });
		}
	}

	for (int y = 0; y < ydim; y++) {
		for (int x = 0; x < xdim; x++) {
			// First triangle
			_mesh.indices.push_back((y + 0) * xverts + (x + 0));
			_mesh.indices.push_back((y + 0) * xverts + (x + 1));
			_mesh.indices.push_back((y + 1) * xverts + (x + 1));

			// Second triangle
			_mesh.indices.push_back((y + 0) * xverts + (x + 0));
			_mesh.indices.push_back((y + 1) * xverts + (x + 1));
			_mesh.indices.push_back((y + 1) * xverts + (x + 0));
		}
	}

    _bound = false;
}

void GLObject::genCuboid(float length, float width, float height) {
    // Points
    point_t p0(-width * 0.5f, -height * 0.5f,  length * 0.5f);
    point_t p1( width * 0.5f, -height * 0.5f,  length * 0.5f);
    point_t p2( width * 0.5f, -height * 0.5f, -length * 0.5f);
    point_t p3(-width * 0.5f, -height * 0.5f, -length * 0.5f);

    point_t p4(-width * 0.5f,  height * 0.5f,  length * 0.5f);
    point_t p5( width * 0.5f,  height * 0.5f,  length * 0.5f);
    point_t p6( width * 0.5f,  height * 0.5f, -length * 0.5f);
    point_t p7(-width * 0.5f,  height * 0.5f, -length * 0.5f);

    // Normals
    normal_t up     (0.0f, 1.0f, 0.0f);
    normal_t down   (-up);
    normal_t front  (0.0f, 0.0f, 1.0f);
    normal_t back   (-front);
    normal_t left   (1.0f, 0.0f, 0.0f);
    normal_t right  (-left);

    // Color (white)
    color_t col(1);

    _mesh.vertices = {
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

    _mesh.indices = {
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
        3 + 4 * 5, 2 + 4 * 5, 1 + 4 * 5
    };

    _bound = false;
}

void GLObject::genLine(const vec3 &start, const vec3 &end, const color_t &col) {
    _mesh.vertices = {
        { start, { 0, 0, 0 }, col, { 0, 0 } },
        { end, {0, 0, 0}, col, {0, 0} }
    };
    _mesh.indices = { 0, 1 };

    _bound = false;
}

void GLObject::genPointCloud(const PointCloud &pc) {
	_mesh.vertices.clear();
	_mesh.indices.clear();

	for (int i = 0; i < pc.cloud.size(); i++) {
		auto pt = pc.cloud[i];
		_mesh.vertices.push_back({ pt.pos, {0, 0, 0}, pt.col, {0, 0} });
		_mesh.indices.push_back(i);
	}

	_bound = false;
}

void GLObject::genPointCloud(const color_t &col, const PointCloud &pc) {
    _mesh.vertices.clear();
    _mesh.indices.clear();

    for (int i = 0; i < pc.cloud.size(); i++) {
        auto pt = pc.cloud[i];
        _mesh.vertices.push_back({ pt.pos, { 0, 0, 0 }, col * pt.stress, { 0, 0 } });
        _mesh.indices.push_back(i);
    }

    _bound = false;
}

void GLObject::genFrustrum(float nearDist, float farDist, vec2 fov, const color_t &col, bool half) {
    // NOTE: this is intended to be used purely for wireframe display
    renderMode = RenderMode::LINES;
    _mesh.vertices.clear();
    _mesh.indices.clear();

    // Points
    fov *= M_PI / 180.0f;
    vec2 uv(0.0f);
    vec3 normal(0.0f);
    vec2 nearDim = { nearDist*tan(0.5f*fov.x), nearDist*tan(0.5f*fov.y) };
    vec2 farDim = { farDist*tan(0.5f*fov.x), farDist*tan(0.5f*fov.y) };

    float nearYBase = 0.0f;
    float farYBase = 0.0f;
    if (half) {
        nearYBase = nearDim.y;
        farYBase = farDim.y;
    }

   _mesh.vertices.push_back({{-nearDim.x, -nearDim.y + nearYBase, nearDist}, normal, col, uv});
   _mesh.vertices.push_back({{ nearDim.x, -nearDim.y + nearYBase, nearDist}, normal, col, uv});
   _mesh.vertices.push_back({{ farDim.x, -farDim.y + farYBase, farDist}, normal, col, uv});
   _mesh.vertices.push_back({{-farDim.x, -farDim.y + farYBase, farDist}, normal, col, uv});
   _mesh.vertices.push_back({{-nearDim.x, nearDim.y + nearYBase, nearDist}, normal, col, uv});
   _mesh.vertices.push_back({{ nearDim.x, nearDim.y + nearYBase, nearDist}, normal, col, uv});
   _mesh.vertices.push_back({{ farDim.x, farDim.y + farYBase, farDist}, normal, col, uv});
   _mesh.vertices.push_back({{-farDim.x, farDim.y + farYBase, farDist}, normal, col, uv});

   // Indices
   _mesh.indices = {
       // Bottom
       0, 1, 1, 2, 2, 3, 3, 0,
       // Top
       4, 5, 5, 6, 6, 7, 7, 4,
       // Verticals
       0, 4, 1, 5, 2, 6, 3, 7
   };

    _bound = false;
}

void GLObject::bind() {
    // Switch to our shaders
    shaders.use();

    if (!shaders.namedParam("pos").valid()) {
        throw "Vertex position attribute missing from object shader!";
    }

	/*if (!shaders.namedParam("norm").valid()) {
		throw "Vertex normal attribute missing from object shader!";
	}

	if (!shaders.namedParam("col").valid()) {
		throw "Vertex color attribute missing from object shader!";
	}*/

    // Grab the vertex input reference (reference to the input structure)
	auto vertexRef = shaders.namedParam("pos");
	auto normalRef = shaders.namedParam("norm");
	auto colorRef = shaders.namedParam("col");
	auto uvRef = shaders.namedParam("uv");

    // Create & bind VAO (this encapsulates the other states)
	if (!_ownsVAO) {
		glGenVertexArrays(1, &_mesh.vao);
		glBindVertexArray(_mesh.vao);

		glGenBuffers(1, &_mesh.vbo);
		glGenBuffers(1, &_mesh.ibo);
		_ownsVAO = true;
	}
	else {
		glBindVertexArray(_mesh.vao);
	}

    // Bind the VBO
    glBindBuffer(GL_ARRAY_BUFFER, _mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, _mesh.vertices.size() * sizeof(vertex), _mesh.vertices.data(), GL_STATIC_DRAW);

    // Bind the IBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _mesh.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, _mesh.indices.size() * sizeof(GLuint), _mesh.indices.data(), GL_STATIC_DRAW);

    // Vertex attributes - defines the input structure
    glEnableVertexAttribArray(vertexRef.ref);
    glVertexAttribPointer(vertexRef.ref, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<void*>(offsetof(vertex, pos)));
	
	if (normalRef.valid()) {
		glEnableVertexAttribArray(normalRef.ref);
		glVertexAttribPointer(normalRef.ref, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<void*>(offsetof(vertex, norm)));
	}

	if (colorRef.valid()) {
		glEnableVertexAttribArray(colorRef.ref);
		glVertexAttribPointer(colorRef.ref, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<void*>(offsetof(vertex, col)));
	}

	if (uvRef.valid()) {
		glEnableVertexAttribArray(uvRef.ref);
		glVertexAttribPointer(uvRef.ref, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<void*>(offsetof(vertex, uv)));
	}

    _bound = true;
}

void GLObject::render(const mat4 &vpMat) {
    if (_hidden) return;
	if (!_bound) bind();

    // Switch to our shaders & mesh VAO
    shaders.use();
	glBindVertexArray(_mesh.vao);

	// General shader uniforms
	auto p1 = shaders.namedParam("camXDim");
	auto p2 = shaders.namedParam("camYDim");
	if (p1.valid()) p1.bindFloat(_scene->camera.dim.width);
	if (p2.valid()) p2.bindFloat(_scene->camera.dim.height);

	auto xz = shaders.namedParam("camXZFactor");
	auto yz = shaders.namedParam("camYZFactor");
	if (xz.valid()) xz.bindFloat(tanf((_scene->camera.fov.x * M_PI / 180.0f) / 2.0f));
	if (yz.valid()) yz.bindFloat(tanf((_scene->camera.fov.y * M_PI / 180.0f) / 2.0f));

    xz = shaders.namedParam("kinectXZFactor");
    yz = shaders.namedParam("kinectYZFactor");
    if (xz.valid()) xz.bindFloat(kinectXZ);
    if (yz.valid()) yz.bindFloat(kinectYZ);

	// Pump in the transformation matrix
	auto mvpRef = shaders.namedParam("MatMVP");
	if (mvpRef.valid()) {
		mat4 mvp = vpMat * getTransform();
		mvpRef.bindMat4(mvp);
	}

    auto mvpProjectorRef = shaders.namedParam("MatMVP_projector");
    if (mvpProjectorRef.valid()) {
        mat4 mvp = GLObject::projectorVP * getTransform();
        mvpProjectorRef.bindMat4(mvp);
    }

	// Points
	if (renderMode == POINTS) {
		glPointSize(pointSize);
	}

	// Render the object
	glDrawElements(renderMode, _mesh.indices.size(), GL_UNSIGNED_INT, NULL);
}

void GLObject::applyTransform(const mat4 &mat) {
	for (auto &vert : _mesh.vertices) {
		vec4 pos4(vert.pos, 1.0f);
		pos4 = mat * pos4;
		vert.pos = pos4.xyz;
	}

	bind();
}

void GLObject::applyTransform(const mat3 &mat) {
	for (auto &vert : _mesh.vertices) {
		vert.pos = mat * vert.pos;
	}

	bind();
}