#pragma once
#include "stdafx.h"
#include "Util.h"
#include "GLObject.h"

// Manages a 3D scene within a GLWindow
class GLScene
{
public:
	// Simple camera class
	struct camera_t {
		Dim dim;
		glm::vec2 fov;
		float aspect;

		glm::vec3 eye, dir;

		glm::mat4 calcProjection() {
			return glm::perspective(fov.y, aspect, 0.1f, 1000.0f);
		}

		glm::mat4 calcView() {
			return glm::lookAt(eye, eye + dir, glm::vec3(0, 1, 0));
		}
	};

	GLScene();
	~GLScene();

	// Setup camera using specified dimensions
	void setCamera(Dim &viewPort, float yFov) {
		camera.dim = viewPort;
		camera.fov.y = yFov;
		camera.aspect = float(viewPort.width) / float(viewPort.height);
		camera.fov.x = camera.fov.y * camera.aspect;

		camera.eye = { 0, 1, 2 };
		camera.dir = { 0, -1, -2 };
	}

	// Generate a new object
	auto newObject(const std::string &fragShader, const std::string &vertShader) {
		auto obj = std::make_shared<GLObject>(this, fragShader, vertShader);
		objects.push_back(obj);
		return obj;
	}

	// Render the scene's objects
	void render() {
		glm::mat4 vp;

		for (auto obj : objects) {
			vp = camera.calcProjection() * camera.calcView();
			obj->render(vp);
		}
	}

	// Free resources
	void free() {
		for (auto obj : objects) {
			obj->shaders.free();
		}
	}

	camera_t camera;
	std::vector<std::shared_ptr<GLObject>> objects;
};

