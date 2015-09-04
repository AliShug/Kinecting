#include "stdafx.h"
#include "GLScene.h"


void GLScene::Camera::handleInput(const SDL_Event &e) {
    float mod;
    if (SDL_GetModState() & KMOD_SHIFT) {
        mod = 0.1f;
    }
    else {
        mod = 1.0f;
    }

	switch (e.type) {
	case SDL_KEYDOWN:
        if (e.key.keysym.mod & KMOD_CTRL) {
            switch (e.key.keysym.scancode) {
            case SDL_SCANCODE_W:
                eye.z += moveSpeed*mod;
                break;
            case SDL_SCANCODE_A:
                eye.x += moveSpeed*mod;
                break;
            case SDL_SCANCODE_S:
                eye.z -= moveSpeed*mod;
                break;
            case SDL_SCANCODE_D:
                eye.x -= moveSpeed*mod;
                break;

            case SDL_SCANCODE_Q:
                eye.y -= moveSpeed*mod;
                break;
            case SDL_SCANCODE_E:
                eye.y += moveSpeed*mod;
                break;
            }
        }
        else {
            switch (e.key.keysym.scancode) {
            case SDL_SCANCODE_W:
                eye += moveSpeed*mod*dir;
                break;
            case SDL_SCANCODE_A:
                eye += moveSpeed*mod*side;
                break;
            case SDL_SCANCODE_S:
                eye -= moveSpeed*mod*dir;
                break;
            case SDL_SCANCODE_D:
                eye -= moveSpeed*mod*side;
                break;

            case SDL_SCANCODE_Q:
                eye -= moveSpeed*mod*up;
                break;
            case SDL_SCANCODE_E:
                eye += moveSpeed*mod*up;
                break;
            }
        }
        break;

	case SDL_MOUSEMOTION:
		// Check the left button is depressed
		if (e.motion.state == SDL_BUTTON(1)) {
			glm::vec2 delta;
			delta.x = float(e.motion.xrel);
			delta.y = float(e.motion.yrel);
			angle += delta*mod*lookSpeed;
		}
		break;
	}
}

glm::mat4 GLScene::Camera::calcView() {
	using namespace glm;

	// Camera angles
	dir.x = cosf(angle.x);
	dir.z = sinf(angle.x);
	dir *= cosf(angle.y);
	dir.y = -sinf(angle.y);

	vec3 targ = eye + dir;
	vec3 dirVec = normalize(dir);
	side = cross(dirVec, vec3(0, -1, 0));
	up = cross(dirVec, side);

	return lookAt(eye, targ, up);
}