#include "stdafx.h"
#include "GLNormalWindow.h"



GLNormalWindow::GLNormalWindow() {
    _inFmt = Texture::Format::DEPTH_FLOAT;
    shaders.vertexShader = "normal_vertex.glsl";
    shaders.fragmentShader = "normal_frag.glsl";
}


GLNormalWindow::~GLNormalWindow() {}

void GLNormalWindow::setPreviousDepthFrame(const void * data) {
    SDL_GL_MakeCurrent(_window, _context);
    shaders.use();
    _prevFrameTex.setImage(data);
}

void GLNormalWindow::initRenderer() {
    GLWindow::initRenderer();

    _prevFrameTex.init(Texture::Format::DEPTH_FLOAT, view.dim.width, view.dim.height);
    shaders.bindTexture(_prevFrameTex, "UPrevDepth");
}
