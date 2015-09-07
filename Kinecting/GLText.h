#pragma once
#include "GLObject.h"
class GLText : public GLObject {
public:
	static void GlobalInit() {
		if (TTF_Init() < 0) {
			throw TTF_GetError();
		}

		_LogFont = TTF_OpenFont("C:/Windows/Fonts/cour.ttf", 18);

		if (_LogFont == nullptr) {
			throw std::string("Opening font failed: ") + TTF_GetError();
		}
	}

	static void GlobalRelease() {
		TTF_CloseFont(_LogFont);
		TTF_Quit();
	}

	// Create the rendering surface
	GLText(const GLScene *parent);

	~GLText() {
		SDL_FreeSurface(_surface);
	}

	void drawText(glm::vec2 pt, std::string text);
	virtual void render(const glm::mat4 &vpMat);
    void clear() {
        SDL_FillRect(_surface, nullptr, 0);
    }

	void onResize(const Dim &dim);

protected:
	static TTF_Font *_LogFont;

	void init(const Dim &dim);

	SDL_Surface *_surface = nullptr;
	Texture _tex;
};

