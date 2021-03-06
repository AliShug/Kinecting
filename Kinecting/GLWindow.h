#pragma once
#include "stdafx.h"
#include "ShaderManager.h"
#include "Texture.h"
#include "Util.h"
#include "GLObject.h"
#include "GLScene.h"
#include "GLText.h"

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

    // Multiple displays
    static int numDisplays;
    static std::vector<SDL_Rect> displays;

    // Global init (SDL, OpenGL etc)
    static void InitGUI() {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            throw std::exception(SDL_GetError());
        }

		GLText::GlobalInit();

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

        // SDL hints - prevent the fullscreen window from hiding itself
        SDL_SetHintWithPriority(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0", SDL_HintPriority::SDL_HINT_OVERRIDE);

        // Multiple monitors
        numDisplays = SDL_GetNumVideoDisplays();
        for (int i = 0; i < numDisplays; i++) {
            SDL_Rect screen;
            SDL_GetDisplayBounds(i, &screen);
            displays.push_back(screen);
        }
    }

    // Global exit (cleanup)
    static void ReleaseGUI() {
		GLText::GlobalRelease();
        SDL_Quit();
    }

    // Local init
    void showWindow(std::string name, Dim &size);

    virtual void update() {}

	// Render the 3D view
    virtual void render();

    virtual void handleEvent(const SDL_Event &e);

    void closeWindow();
    void capture(const std::string &file);
    void getFrame(uint32_t **buff, int *size);

    void setFullscreen(bool fs);
    void toggleFullscreen();

    // Moves to the next monitor
    void nextMonitor() {
        _displayIndex = (_displayIndex + 1) % numDisplays;
        SDL_Rect display = displays[_displayIndex];
        SDL_SetWindowPosition(_window, display.x, display.y);
    }

	// Activates the window's rendering context
	void activate() {
		SDL_GL_MakeCurrent(_window, _context);
	}

	GLScene scene;
    std::stringstream message;
    std::shared_ptr<GLText> overlay = nullptr;

protected:
    // Error check util
    bool checkGl(const char* location);

    // Members
    int _windowID = -1;
    int _displayIndex = 0;
    //Window focus
    bool _mouseFocus;
    bool _keyboardFocus;
    bool _minimized;
    bool _shown;
    SDL_Window *_window = nullptr;
    SDL_GLContext _context = nullptr;

    bool _fullscreen = false;
    std::string _windowName;
};

