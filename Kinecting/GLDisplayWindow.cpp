#include "stdafx.h"
#include "GLDisplayWindow.h"



GLDisplayWindow::GLDisplayWindow() {
    _inFmt = Texture::Format::BGR;
    shaders.vertexShader = "normal_vertex.glsl";
    shaders.fragmentShader = "rgb_frag.glsl";
}


GLDisplayWindow::~GLDisplayWindow() {}

void GLDisplayWindow::setDepth(const void *data, const Dim *size) {
	SDL_GL_MakeCurrent(_window, _context);
	shaders.use();

	if (size && (_depthTex.width != size->width || _depthTex.height != size->height)) {
		// Re-initialize texture with new input size
		_depthTex = Texture();
		_depthTex.init(_inFmt, size->width, size->height);
		shaders.bindTexture(_depthTex, "UInputImg");
	}

	_depthTex.setImage(data);
}