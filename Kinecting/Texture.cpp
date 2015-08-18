#include "stdafx.h"
#include "Texture.h"



Texture::Texture() {}


Texture::~Texture() {}


void Texture::init(Format fmt, int w, int h) {
    width = w;
    height = h;
    
    switch (fmt) {
    case Format::DEPTH:
        _glInternalFormat = GL_R16UI;
        _glFormat = GL_RED_INTEGER;
        _glDataType = GL_UNSIGNED_SHORT;
        break;
    case Format::DEPTH_FLOAT:
        _glInternalFormat = GL_R32F;
        _glFormat = GL_RED;
        _glDataType = GL_FLOAT;
        break;

    case Format::RGB:
        _glInternalFormat = GL_RGBA8;
        _glFormat = GL_RGBA;
        _glDataType = GL_UNSIGNED_BYTE;
        break;
    case Format::BGR:
        _glInternalFormat = GL_RGBA8;
        _glFormat = GL_BGRA;
        _glDataType = GL_UNSIGNED_BYTE;
        break;
    }

    glGenTextures(1, &glTex);
    glBindTexture(GL_TEXTURE_2D, glTex);

    // Allocate memory
    glTexImage2D(GL_TEXTURE_2D, 0, _glInternalFormat, w, h, 0, _glFormat, _glDataType, nullptr);
}

void Texture::bind(GLuint prog, GLint ref, int index) {
    _prog = prog;
    textureIndex = index;

    glUseProgram(_prog);
    glUniform1i(ref, index);
    activate();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) throw std::exception("Bad texture binding");
}

void Texture::setImage(const void * data) {
    activate();

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, _glFormat, _glDataType, data);
    GLenum err = glGetError();

	if (err != GL_NO_ERROR) {
		std::cerr << "Bad texture write to " << glTex << ", " << width << "x" << height << std::endl;
		throw "Bad texture write to " + std::to_string(glTex) + ", error " + std::to_string(err);
	}
}

void Texture::activate() {
    glActiveTexture(GL_TEXTURE0 + textureIndex);
    glBindTexture(GL_TEXTURE_2D, glTex);
}