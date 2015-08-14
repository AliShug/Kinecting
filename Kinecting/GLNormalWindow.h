#pragma once
#include "GLWindow.h"
class GLNormalWindow :
    public GLWindow {
public:
    GLNormalWindow();
    ~GLNormalWindow();

    void setPreviousDepthFrame(const void *data);

protected:
    void initRenderer();
    Texture _prevFrameTex;
};

