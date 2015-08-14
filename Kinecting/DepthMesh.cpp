// Application includes
#include "stdafx.h"
#include "DepthMesh.h"


DepthMesh::DepthMesh(void) {}

DepthMesh::~DepthMesh(void) {}


bool DepthMesh::init(GLint vertexAttribute) {
	static_assert(sizeof(glm::vec3) == 3 * sizeof(GLfloat), "glm::vec3 unsuitably padded");

    int xdim = 1, ydim = 1;
	int xverts = xdim + 1;
	int yverts = ydim + 1;
	size_t vbSize = xverts * yverts * sizeof(glm::vec3);
	glm::vec3* vData = (glm::vec3*) malloc(vbSize);

	// Generate grid vertices
	for (int y = 0; y < yverts; y++) {
		for (int x = 0; x < xverts; x++) {
			// Initialize vertex
			glm::vec3* vp = &vData[y * xverts + x];
			*vp = glm::vec3(0.0f);

			// Grid
			vp->x = (float)x * 2 - 1;
			vp->y = (float)y * 2 - 1;
			//(*vp) *= scale;
		}
	}

	// Index buffer data
	_inds = 6 * xdim * ydim;
	size_t ibSize = _inds * sizeof(GLuint);
	GLuint* iData = (GLuint*) malloc(ibSize);

	for (int y = 0; y < ydim; y++) {
		for (int x = 0; x < xdim; x++) {
			// First triangle
			iData[(y*xdim + x)*6 + 0] = (y + 0)*xverts + (x + 0);
			iData[(y*xdim + x)*6 + 1] = (y + 0)*xverts + (x + 1);
			iData[(y*xdim + x)*6 + 2] = (y + 1)*xverts + (x + 1);

			// Second triangle
			iData[(y*xdim + x)*6 + 3] = (y + 0)*xverts + (x + 0);
			iData[(y*xdim + x)*6 + 4] = (y + 1)*xverts + (x + 1);
			iData[(y*xdim + x)*6 + 5] = (y + 1)*xverts + (x + 0);
		}
	}


	// Create & bind VAO (this encapsulates the other states)
	glGenVertexArrays(1, &_VAO);
	glBindVertexArray(_VAO);

	// Create the VBO
	glGenBuffers(1, &_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, _VBO);
	glBufferData(GL_ARRAY_BUFFER, vbSize, vData, GL_STATIC_DRAW);

	// Create IBO
	glGenBuffers(1, &_IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ibSize, iData, GL_STATIC_DRAW);

	// Bind vertex attribute (within VAO)
	glEnableVertexAttribArray(vertexAttribute);
	glVertexAttribPointer(vertexAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);

	return true;
}

void DepthMesh::render() {
	// Ensure the VAO is bound (encapsulates all relevant states)
	glBindVertexArray(_VAO);

	// Render!
	glDrawElements(GL_TRIANGLES, _inds, GL_UNSIGNED_INT, 0);
}