#pragma once
#include "stdafx.h"
#include "Util.h"

class Texture {
public:
    enum Format {
        DEPTH,
        DEPTH_FLOAT,
        RGB,
        BGR
    };

    Texture();
    ~Texture();

    void init(Format fmt, const Dim &size);
    void bind(GLuint prog, GLint ref, int index);
	void resize(const Dim &size);
    void setImage(const void *data);

    void activate();

    GLuint glTex;
    int width, height;
    int textureIndex = 0;

protected:
    GLint _glInternalFormat;
    GLenum _glFormat, _glDataType;
    GLuint _prog;
};

