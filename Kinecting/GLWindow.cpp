#include "stdafx.h"
#include "GLWindow.h"
#include "Util.h"


GLWindow::GLWindow() {}

GLWindow::~GLWindow() {
    closeWindow();
}

void GLWindow::showWindow(std::string name, Dim &size) {
    if (_window != nullptr) throw std::exception("Window already created!");
    std::cout << "Initializing window " << size.width << "x" << size.height << std::endl;
    _windowName = name;

    // Window
    view.dim = size;
    _window = SDL_CreateWindow(
        name.c_str(),
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        size.width, size.height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    if (!_window) {
        throw std::exception(SDL_GetError());
    }

    // GL Context (implicitly current context)
    _context = SDL_GL_CreateContext(_window);
    if (!_context) {
        throw std::exception(SDL_GetError());
    }

    // GLEW
    glewExperimental = true;
    GLenum glewErr = glewInit();

    if (glewErr != GLEW_OK) {
        throw std::exception((char *) glewGetErrorString(glewErr));
    }

    // Clear the stupid GLEW/GL Enum error
    glGetError();

	// GL settings
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	SDL_GL_SetSwapInterval(0);
	glDisable(GL_FRAMEBUFFER_SRGB);

	if (!checkGl(__FUNCTION__)) throw std::exception("GL init failure");
}

bool GLWindow::checkGl(const char* location) {
    GLenum err;

    if ((err = glGetError()) != GLEW_OK) {
        std::cerr << "Fatal OpenGL error: " << err << "@" << location << std::endl;

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
    if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.scancode == SDL_SCANCODE_SPACE) {
            // Capture image
            std::cout << "Capturing image" << std::endl;
            capture(_windowName + ".bmp");
        }
    }

	scene.camera.handleInput(e);
}

void GLWindow::capture(const std::string &file) {
    SDL_GL_MakeCurrent(_window, _context);

    auto pixels = std::unique_ptr<GLubyte>(new GLubyte[3 * view.dim.area()]);
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
