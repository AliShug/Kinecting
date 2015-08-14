#include "stdafx.h"
#include "GLDepthWindow.h"



GLDepthWindow::GLDepthWindow() {
    _inFmt = Texture::Format::DEPTH_FLOAT;
    shaders.vertexShader = "normal_vertex.glsl";
    shaders.fragmentShader = "depth_frag.glsl";
}

GLDepthWindow::~GLDepthWindow() {}
