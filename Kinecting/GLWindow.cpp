#include "stdafx.h"
#include "GLWindow.h"
#include "Util.h"


GLWindow::GLWindow() {}

GLWindow::~GLWindow() {
    closeWindow();
}


void GLWindow::initRenderer() {
    // Load and compile the shaders
    shaders.compileShaders();

    // Allocate & setup the textures
    _inputTex.init(_inFmt, view.dim.width, view.dim.height);
    shaders.bindTexture(_inputTex, "UInputImg");

    // GL settings
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glDisable(GL_CULL_FACE);

    // Depth mesh
    _mesh.init(shaders.namedParam("LVertexPos3D").ref);

    // Set frame properties
    auto p1 = shaders.namedParam("camXDim");
    auto p2 = shaders.namedParam("camYDim");
    if (p1.valid()) p1.bindFloat(view.dim.width);
    if (p2.valid()) p2.bindFloat(view.dim.height);

    auto xz = shaders.namedParam("camXZFactor");
    auto yz = shaders.namedParam("camYZFactor");
    if (xz.valid()) xz.bindFloat(tanf((70.6f * M_PI / 180.0f) / 2.0f));
    if (yz.valid()) yz.bindFloat(tanf((60.0f * M_PI / 180.0f) / 2.0f));

    SDL_GL_SetSwapInterval(0);

    // Disable sRGB 'corrections' - we want true linear colour space for calculations
    glDisable(GL_FRAMEBUFFER_SRGB);

    if (!checkGl(__FUNCTION__)) throw std::exception("GL init failure");
}

void GLWindow::showWindow(std::string name, int w, int h) {
    if (_window != nullptr) throw std::exception("Window already created!");
    std::cout << "Initializing window " << w << "x" << h << std::endl;

    _windowName = name;

    // Work out the GL texture formats
    switch (_inFmt) {
    case Texture::Format::DEPTH:
        _glSurfInternalFormat = GL_R16UI;
        _glSurfFormat = GL_RED_INTEGER;
        _glSurfDataType = GL_UNSIGNED_SHORT;
        break;

    case Texture::Format::RGB:
        _glSurfInternalFormat = GL_RGB8;
        _glSurfFormat = GL_RGB;
        _glSurfDataType = GL_UNSIGNED_BYTE;
        break;
    }

    // Window
    view.dim.width = w; view.dim.height = h;
    _window = SDL_CreateWindow(
        name.c_str(),
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        w, h,
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

    // VSync
    if (SDL_GL_SetSwapInterval(1) < 0) {
        std::cout << "Warning - Could not enable vsync: " << SDL_GetError() << std::endl;
    }

    // OpenGL
    initRenderer();
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
    shaders.free();

    SDL_GL_DeleteContext(_context);
    SDL_DestroyWindow(_window);
    _window = nullptr;
}

void GLWindow::render() {
    SDL_GL_MakeCurrent(_window, _context);

    // Render our quad
    shaders.use();
    _mesh.render();

    // Unbind
    glUseProgram(NULL);

    // Swap buffers
    SDL_GL_SwapWindow(_window);
}

void GLWindow::setInputImage(const void* data, const Dim *size) {
    SDL_GL_MakeCurrent(_window, _context);

    if (size && (_inputTex.width != size->width || _inputTex.height != size->height)) {
        // Re-initialize texture with new input size
        _inputTex = Texture();
        _inputTex.init(_inFmt, size->width, size->height);
        shaders.bindTexture(_inputTex, "UInputImg");
    }

    shaders.use();
    _inputTex.setImage(data);
}