#include "stdafx.h"
#include "GLDisplayWindow.h"



GLDisplayWindow::GLDisplayWindow() {
    _inFmt = Texture::Format::BGR;
    shaders.vertexShader = "normal_vertex.glsl";
    shaders.fragmentShader = "rgb_frag.glsl";
}


GLDisplayWindow::~GLDisplayWindow() {}


void GLDisplayWindow::renderObject(glm::vec3 pos) {

}