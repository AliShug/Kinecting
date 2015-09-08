#include "stdafx.h"
#include "GLScene.h"

using namespace std;
using namespace glm;

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

            case SDL_SCANCODE_KP_PLUS:
                setFov(fov.y - 0.02f);
                break;
            case SDL_SCANCODE_KP_MINUS:
                setFov(fov.y + 0.02f);
                break;
            }
        }
        cout << "Eye " << eye << endl;
        break;

	case SDL_MOUSEMOTION:
		// Check the left button is depressed
		if (e.motion.state == SDL_BUTTON(1)) {
			vec2 delta;
			delta.x = float(e.motion.xrel);
			delta.y = float(e.motion.yrel);
			angle += delta*mod*lookSpeed;
		}
		break;
	}
}

mat4 GLScene::Camera::calcView() {
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

void GLScene::readCameraSettings(string file) {
    // Read in camera data
    ifstream in(file);

    if (in.is_open()) {
        string line;
        size_t sz;

        // Angle
        getline(in, line);
        camera.angle.x = stof(line, &sz);
        camera.angle.y = stof(line.substr(sz + 1));

        // Eye position
        getline(in, line);
        camera.eye.x = stof(line, &sz);
        line = line.substr(sz + 1);
        camera.eye.y = stof(line, &sz);
        camera.eye.z = stof(line.substr(sz + 1));

        // Fov
        getline(in, line);
        camera.fov.x = stof(line, &sz);
        camera.fov.y = stof(line.substr(sz + 1));

        in.close();
    }
}

void GLScene::saveCameraSettings(string file) {
    // Write out necessary camera data
    ofstream out(file);

    if (out.is_open()) {
        out << camera.angle.x << "," << camera.angle.y << endl;
        out << camera.eye.x << "," << camera.eye.y << "," << camera.eye.z << endl;
        out << camera.fov.x << "," << camera.fov.y << endl;

        out.close();
    }
}