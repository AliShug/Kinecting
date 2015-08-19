#include "stdafx.h"
#include "GLScene.h"


void GLScene::Camera::handleInput(const SDL_Event &e) {
	switch (e.type) {

	case SDL_KEYDOWN:
		//case SDL_KEYUP:
		switch (e.key.keysym.scancode) {
		case SDL_SCANCODE_W:
			eye += moveSpeed*dir;
			break;
		case SDL_SCANCODE_A:
			eye -= moveSpeed*side;
			break;
		case SDL_SCANCODE_S:
			eye -= moveSpeed*dir;
			break;
		case SDL_SCANCODE_D:
			eye += moveSpeed*side;
			break;

		case SDL_SCANCODE_Q:
			eye -= moveSpeed*up;
			break;
		case SDL_SCANCODE_E:
			eye += moveSpeed*up;
			break;
		}
		break;

	case SDL_MOUSEMOTION:
		// Check the left button is depressed
		if (e.motion.state == SDL_BUTTON(1)) {
			glm::vec2 delta;
			delta.x = e.motion.xrel;
			delta.y = e.motion.yrel;
			angle += delta*lookSpeed;
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
	side = cross(dirVec, vec3(0, 1, 0));
	up = cross(dirVec, side);

	return lookAt(eye, targ, up);
}