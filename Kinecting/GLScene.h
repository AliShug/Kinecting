#pragma once
#include "stdafx.h"
#include "Util.h"
#include "GLObject.h"
#include "GLText.h"

// Manages a 3D scene within a GLWindow
class GLScene
{
public:
	// Simple camera class
	struct Camera {
		Dim dim;
		glm::vec2 fov;
		float aspect;

		glm::vec3 eye, dir, side, up;
		glm::vec2 angle = { M_PI / 2, 0.0f };
		float moveSpeed = 0.1f;
		float lookSpeed = 0.02f;

		glm::mat4 calcProjection() {
			return glm::perspectiveFov(fov.y * float(M_PI/180), float(dim.width), float(dim.height), 0.01f, 1000.0f);
		}

		glm::mat4 calcView();
		void handleInput(const SDL_Event &e);

	};

	GLScene() {}
	~GLScene() {
		release();
	}

	// Setup camera using specified dimensions
	void setCamera(Dim &viewPort, float yFov) {
		camera.fov.y = yFov;
		camera.eye = { 0, 0, 0 };

        setCameraDim(viewPort);
	}

    void setCameraDim(Dim &viewPort) {
        camera.dim = viewPort;
        camera.aspect = float(viewPort.width) / float(viewPort.height);
        camera.fov.x = camera.fov.y * camera.aspect;
    }

    void setCameraFov(float yFov) {
        camera.fov.y = yFov;
        camera.fov.x = camera.fov.y * camera.aspect;
    }

    // Camera settings save/load
    void readCameraSettings(std::string file);
    void saveCameraSettings(std::string file);

	// Generate a new object
	auto newObject(const std::string &fragShader, const std::string &vertShader) {
		auto obj = std::make_shared<GLObject>(this, fragShader, vertShader);
		objects.push_back(obj);
		return obj;
	}

	auto newTextOverlay() {
		auto text = std::make_shared<GLText>(this);
		objects.push_back(text);
		return text;
	}

	// Render the scene's objects
	void render() {
        glm::mat4 vp = camera.calcProjection() * camera.calcView();

		for (auto obj : objects) {
			obj->render(vp);
		}
	}

	// Free resources
	void release() {
		for (auto obj : objects) {
			obj->shaders.release();
		}
	}

	Camera camera;
	std::vector<std::shared_ptr<GLObject>> objects;
};

