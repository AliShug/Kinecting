#include "stdafx.h"
#include "GLScene.h"
#include "GLText.h"

using namespace glm;
using namespace std;

TTF_Font *GLText::_LogFont = nullptr;

void GLText::init(const Dim &dim) {
	// OpenGL-compatible SDL rendering surface
	uint32_t rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif

	_surface = SDL_CreateRGBSurface(0, dim.width, dim.height, 32,
		rmask, gmask, bmask, amask);
	if (_surface == nullptr) {
		throw string("CreateRGBSurface failed: ") + SDL_GetError();
	}

    _tex.init(Texture::RGB, dim);
    shaders.bindTexture(_tex, "UInputImg");
}

GLText::GLText(const GLScene *parent) : GLObject(parent, "text_frag.glsl", "text_vert.glsl") {
	init(parent->camera.dim);
	genQuad({ 1, 1 });
}

void GLText::drawText(vec2 pt, string text) {
    if (text.empty()) return;

	SDL_Surface *newText;
	newText = TTF_RenderText_Blended(_LogFont, text.c_str(), { 255, 255, 255, 255 });

	if (newText == nullptr) {
		throw string("Text rendering failed: ") + TTF_GetError();
	}

	SDL_Rect targetRect;
	targetRect.x = int(pt.x);
	targetRect.y = int(pt.y);
	targetRect.w = newText->w;
	targetRect.h = newText->h;

	int res = SDL_BlitSurface(newText, nullptr, _surface, &targetRect);
    if (res < 0) {
        throw string("Failed to blit text surface: ") + SDL_GetError();
    }
}

void GLText::render(const glm::mat4 &vpMat) {
	if (_hidden) return;
	if (!_bound) bind();

	// Switch to our shaders & mesh VAO
	shaders.use();
	glBindVertexArray(_mesh.vao);

	// Write the image data
	_tex.setImage(_surface->pixels);

	// General shader uniforms
	auto p1 = shaders.namedParam("camXDim");
	auto p2 = shaders.namedParam("camYDim");
	if (p1.valid()) p1.bindFloat(_scene->camera.dim.width);
	if (p2.valid()) p2.bindFloat(_scene->camera.dim.height);

	// Render the object
	glDrawElements(renderMode, _mesh.indices.size(), GL_UNSIGNED_INT, NULL);
}

void GLText::onResize(const Dim &dim) {
    if (dim.width == _tex.width && dim.height == _tex.height) return;

	SDL_FreeSurface(_surface);
	_tex.resize(dim);

	// OpenGL-compatible SDL rendering surface
	uint32_t rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif

	_surface = SDL_CreateRGBSurface(0, dim.width, dim.height, 32,
		rmask, gmask, bmask, amask);
	if (_surface == nullptr) {
		throw string("CreateRGBSurface failed: ") + SDL_GetError();
	}
}