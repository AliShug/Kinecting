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
        bool shortThrow;

		glm::vec3 eye, dir, side, up;
		glm::vec2 angle = { M_PI / 2, 0.0f };
		float moveSpeed = 0.1f;
		float lookSpeed = 0.02f;

        std::shared_ptr<GLText> overlay = nullptr;

		glm::mat4 calcProjection() {
            float internalFov = fov.y;
            Dim internalDim = dim;
            if (shortThrow) {
                internalFov *= 2;
                internalDim.height *= 2;
            }

			return glm::perspectiveFov(internalFov * float(M_PI/180), float(internalDim.width), float(internalDim.height), 0.01f, 1000.0f);
		}

		glm::mat4 calcView();
		void handleInput(const SDL_Event &e);

        void set(bool isProjector, Dim newDim, float yFov, glm::vec3 newEye = { 0, 0, 0 }, glm::vec2 newAngle = { M_PI / 2, 0.0f }) {
            angle = newAngle;
            eye = newEye;
            shortThrow = isProjector;
            setDim(newDim);
            setFov(yFov);
        }

        void setDim(const Dim &newDim) {
            dim = newDim;
            aspect = float(newDim.width) / float(newDim.height);
            fov.x = fov.y * aspect;

            Dim internalDim = dim;
            if (shortThrow) {
                internalDim.height *= 2;
                glViewport(0, -dim.height, internalDim.width, internalDim.height);
            }

            if (overlay) {
                overlay->onResize(internalDim);
            }
        }

        void setFov(float yFov) {
            fov.y = yFov;
            fov.x = fov.y * aspect;
        }
	};

	GLScene() {}
	~GLScene() {
		release();
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

	auto getTextOverlay() {
        if (!camera.overlay) {
            camera.overlay = std::make_shared<GLText>(this);
            camera.setDim(camera.dim); // make sure the overlay's size is updated
            return camera.overlay;
        }
        else {
            return camera.overlay;
        }
	}

	// Render the scene's objects
	void render() {
        glm::mat4 vp = camera.calcProjection() * camera.calcView();

		for (auto obj : objects) {
			obj->render(vp);
		}

        if (camera.overlay) {
            camera.overlay->render(vp);
            camera.overlay->clear();
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

