#pragma once
#include "stdafx.h"
#include "GLWindow.h"
#include "Texture.h"

class GLDisplayWindow : public GLWindow {
public:
    GLDisplayWindow();
    ~GLDisplayWindow();

	void setDepth(const void *data, const Dim *size);

protected:
	Texture _depthTex;
};

