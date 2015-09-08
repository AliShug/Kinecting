#include "stdafx.h"
#include "GLWindow.h"
#include "Util.h"

using namespace std;

GLWindow::GLWindow() {}

GLWindow::~GLWindow() {
    closeWindow();
}

void GLWindow::showWindow(string name, Dim &size) {
    if (_window != nullptr) throw exception("Window already created!");
    cout << "Initializing window " << size.width << "x" << size.height << endl;
    _windowName = name;

    // Window
    view.dim = size;
    _window = SDL_CreateWindow(
        name.c_str(),
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        size.width, size.height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    if (!_window) {
        throw exception(SDL_GetError());
    }

    _windowID = SDL_GetWindowID(_window);
    _mouseFocus = true;
    _keyboardFocus = true;

    // GL Context (implicitly current context)
    _context = SDL_GL_CreateContext(_window);
    if (!_context) {
        throw exception(SDL_GetError());
    }

    // GLEW
    glewExperimental = true;
    GLenum glewErr = glewInit();

    if (glewErr != GLEW_OK) {
        throw exception((char *) glewGetErrorString(glewErr));
    }

    // Clear the stupid GLEW/GL Enum error
    glGetError();

	// GL settings
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	SDL_GL_SetSwapInterval(0);
	glDisable(GL_FRAMEBUFFER_SRGB);

	if (!checkGl(__FUNCTION__)) throw exception("GL init failure");
}

bool GLWindow::checkGl(const char* location) {
    GLenum err;

    if ((err = glGetError()) != GLEW_OK) {
        cerr << "Fatal OpenGL error: " << err << "@" << location << endl;

        char error[128];
        sprintf_s(error, 128, "Error @%s %i", location, err);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
            "Fatal OpenGL Error",
            error,
            NULL);

        return false;
    }
    else {
        return true;
    }
}

void GLWindow::handleEvent(const SDL_Event &e) {
    if (e.type == SDL_WINDOWEVENT && e.window.windowID == _windowID) {
        Dim size = { e.window.data1, e.window.data2 };
        switch (e.window.event) {
        case SDL_WINDOWEVENT_RESIZED:
            scene.camera.setDim(size);
            break;

        case SDL_WINDOWEVENT_SHOWN:
            _shown = true;
            break;

        case SDL_WINDOWEVENT_HIDDEN:
            _shown = false;
            break;

        case SDL_WINDOWEVENT_ENTER:
            _mouseFocus = true;
            break;

        case SDL_WINDOWEVENT_LEAVE:
            _mouseFocus = false;
            break;

        case SDL_WINDOWEVENT_FOCUS_GAINED:
            _keyboardFocus = true;
            break;

        case SDL_WINDOWEVENT_FOCUS_LOST:
            _keyboardFocus = false;
            _mouseFocus = false;
            break;

        case SDL_WINDOWEVENT_MINIMIZED:
            _keyboardFocus = false;
            _mouseFocus = false;

            if (_fullscreen) {
                // note: this breaks windows
                //SDL_RestoreWindow(_window);
            }
            else _minimized = true;
            break;

        case SDL_WINDOWEVENT_RESTORED:
        case SDL_WINDOWEVENT_MAXIMIZED:
            _minimized = false;
            break;
        }
    }

    if (_keyboardFocus && e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.scancode) {
            // Image capture
        case SDL_SCANCODE_SPACE:
            cout << "Capturing image" << endl;
            capture(_windowName + ".bmp");
            break;

            // Fullscreen support
        case SDL_SCANCODE_F11:
            toggleFullscreen();
            break;

            // Save/restore camera settings
        case SDL_SCANCODE_V:
            scene.saveCameraSettings("camera_new.cfg");
            message.str("Saved camera settings to 'camera_new.cfg'");
            break;
        case SDL_SCANCODE_B:
            scene.readCameraSettings("camera.cfg");
            message.str("Loaded camera settings from 'camera.cfg'");
            break;
        }
    }

    if (_mouseFocus && _keyboardFocus) {
        scene.camera.handleInput(e);
    }
}

void GLWindow::capture(const string &file) {
    SDL_GL_MakeCurrent(_window, _context);

    auto pixels = unique_ptr<GLubyte>(new GLubyte[3 * view.dim.area()]);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glReadPixels(0, 0, view.dim.width, view.dim.height, GL_BGR, GL_UNSIGNED_BYTE, pixels.get());

    FIBITMAP* cap = FreeImage_ConvertFromRawBits(
        pixels.get(), view.dim.width, view.dim.height,
        3 * view.dim.width, 24,
        0xFF0000, 0x00FF00, 0x0000FF);

    FreeImage_Save(FIF_BMP, cap, file.c_str());
    FreeImage_Unload(cap);
}

void GLWindow::getFrame(uint32_t ** buff, int * size) {
    SDL_GL_MakeCurrent(_window, _context);
    *size = view.dim.width * view.dim.height;
    *buff = new uint32_t[*size];
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glReadPixels(0, 0, view.dim.width, view.dim.height, GL_BGRA, GL_UNSIGNED_BYTE, *buff);
}

void GLWindow::closeWindow() {
	SDL_GL_MakeCurrent(_window, _context);
	scene.release();
    SDL_GL_DeleteContext(_context);
    SDL_DestroyWindow(_window);
    _window = nullptr;
}

void GLWindow::setFullscreen(bool fs) {
    _fullscreen = fs;

    if (_fullscreen) {
        SDL_SetWindowFullscreen(_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
    else {
        SDL_SetWindowFullscreen(_window, 0);
    }
}

void GLWindow::toggleFullscreen() {
    setFullscreen(!_fullscreen);
}
