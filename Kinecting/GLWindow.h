#pragma once
#include "stdafx.h"
#include "DepthMesh.h"
#include "ShaderManager.h"
#include "Texture.h"
#include "Util.h"
#include "GLObject.h"

class GLWindow {
public:
    struct cameraInfo {
        float xFov, yFov;
        float xzFactor, yzFactor;
        Dim dim;
    };

    GLWindow();
    virtual ~GLWindow();

    // Window size
    cameraInfo view;

    // Global init (SDL, OpenGL etc)
    static void InitGUI() {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            throw std::exception(SDL_GetError());
        }

        // OpenGL 3.3 Core
        if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3) < 0) {
            throw std::exception(SDL_GetError());
        }
        if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3) < 0) {
            throw std::exception(SDL_GetError());
        }
        if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE) < 0) {
            throw std::exception(SDL_GetError());
        }
    }

    // Global exit (cleanup)
    static void ReleaseGUI() {
        SDL_Quit();
    }

    // Local init
    void showWindow(std::string name, int w, int h);

    virtual void update() {}
    virtual void render();
    virtual void handleEvent(const SDL_Event &e);
    virtual void setInputImage(const void * data, const Dim *size = nullptr);

    void closeWindow();
    void capture(const std::string &file);
    void getFrame(uint32_t **buff, int *size);

	// Creates, stores, binds and returns a new gl-object
	auto createObject() {
		SDL_GL_MakeCurrent(_window, _context);
		auto obj = std::make_shared<GLObject>();
		obj->genCuboid(1, 1, 1);
		obj->bind();
		_objects.push_back(obj);
		return obj;
	}

    ShaderManager shaders;

protected:
	// TEMP (probably)
	std::vector<std::shared_ptr<GLObject>> _objects;
	// /TEMP

    // Various init & utility functions
    virtual void initRenderer();
    bool checkGl(const char* location);

    // Variables
    SDL_Window *_window = nullptr;
    SDL_GLContext _context = nullptr;

    bool _fullscreen;

    DepthMesh _mesh;
    Texture _inputTex;
    std::string _windowName;

    // Format
    Texture::Format _inFmt, _outFmt;

    GLint _glSurfInternalFormat;
    GLenum _glSurfFormat, _glSurfDataType;
};

