#pragma once
#include "stdafx.h"
#include "GLWindow.h"

class GLDisplayWindow : public GLWindow {
public:
    GLDisplayWindow();
    ~GLDisplayWindow();

    void renderObject(glm::vec3 pos);
};

