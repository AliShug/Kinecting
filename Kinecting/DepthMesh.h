#pragma once
#include "stdafx.h"

class DepthMesh {

public:
	DepthMesh(void);
	~DepthMesh(void);

	bool init(GLint vertexAttribute);
    void render();

protected:
	GLuint _VBO, _IBO, _VAO;
	int _inds;
};

