// Kinecting.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "KinectDevice.h"

#include "GLWindow.h"
#include "NormDepthImage.h"
#include "PointCloud.h"
#include "GLObject.h"

auto getTestPng(std::string file) {
    // Load a test image
    FIBITMAP *temp = FreeImage_Load(FIF_PNG, (file+".png").c_str(), PNG_DEFAULT);
    FIBITMAP *testBmp = FreeImage_ConvertTo32Bits(temp);
    FreeImage_Unload(temp);

    int imW, imH;
    imW = FreeImage_GetWidth(testBmp);
    imH = FreeImage_GetHeight(testBmp);

    auto testData = std::shared_ptr<uint32_t>(new uint32_t[imW * imH]);
    memcpy(testData.get(), FreeImage_GetBits(testBmp), 4 * imW * imH);
    FreeImage_Unload(testBmp);

    return testData;
}

int main(int argc, char *args[]) {
    KinectDevice kinect;
    GLWindow dispWindow;

    // Data for testing (load from png)
    //auto testData = getTestPng("normalTestImg");

    try {
        // Initialize the gui and kinect
        GLWindow::InitGUI();
        kinect.connect(kinect.DEPTH_STREAM);

        int frame_w = kinect.depthFrameInfo.w;
        int frame_h = kinect.depthFrameInfo.h;

        Dim fullSize = { frame_w, frame_h };
        dispWindow.showWindow("Kinecting", fullSize);
		auto &scene = dispWindow.scene;
		scene.setCamera(fullSize, kinect.depthFrameInfo.yFov);

		// Storage for raw camera output
		auto rawData = std::unique_ptr<uint16_t>(new uint16_t[fullSize.area()]);
		auto floatData = std::unique_ptr<float>(new float[fullSize.area()]);

        // Image processing platform
        NormDepthImage img(fullSize);

		// Display objects
		//auto obj = dispWindow.createObject();

		// Camera output surface
		auto surf = scene.newObject("rgb_frag.glsl", "normal_vertex.glsl");
		surf->genQuad();
		surf->bind();
		
		Texture normalTex, depthTex;
		normalTex.init(Texture::BGR, fullSize);
		depthTex.init(Texture::DEPTH_FLOAT, fullSize);

		surf->shaders.bindTexture(normalTex, "UInputImg");
		surf->shaders.bindTexture(depthTex, "UInputDepth");

		// Cube
		auto obj = scene.newObject("object_frag.glsl", "object_vert.glsl");
		obj->genCuboid();
		obj->bind();

        // Begin reading in frames...
        kinect.asyncListen();
        // ... and run the main loop
        SDL_Event e;
        bool quit = false;
        while (!quit) {
            while (SDL_PollEvent(&e) != 0) {
                // Handle all waiting events
                dispWindow.handleEvent(e);

                if (e.type == SDL_QUIT || (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE)) {
                    quit = true;
                }
            }


            {
                // Exception-proof lock for depth read
				std::lock_guard<decltype(kinect.frameLock)> lck(kinect.frameLock);
                memcpy(rawData.get(), kinect.depthData.get(), fullSize.area()*sizeof(uint16_t));
            }
                
            // Convert depth to float-32
            const int count = frame_w * frame_h;
            auto fd = floatData.get();
            auto rd = rawData.get();

            for (int i = 0; i < count; i++) {
                fd[i] = (float) rd[i];
            }

            // Pass to image processor and gui
            img.setDepth(floatData.get());

			dispWindow.activate();
			surf->shaders.use();
			depthTex.setImage(floatData.get());
            
            // **************************************************
            // *** Image Processing

            // Smooth depth
            //img.threshold_meanDepthBlur(4, 0.001f);

            // Calculate normals
            const float camXZ = tanf((70.6f * M_PI / 180.0f) / 2.0f);
            const float camYZ = tanf((60.0f * M_PI / 180.0f) / 2.0f);
            img.calcNormals(camXZ, camYZ);

            // Blur preprocess
            img.threshold_gaussNormalBlur(12, 0.8f, 0.01f);
            //img.threshold_meanNormalBlur(4, 0.28f);

            // Flood-fill
            //img.threshold_normalFlood({ frame_w / 2, frame_h / 2 }, 0.1f, 0.01f);


            // Now we can generate a point-cloud and get the mean position
            PointCloud pc(img, camXZ, camYZ);
            auto pos = pc.meanPosition();
            //std::cout << pos.z << std::endl;
			//obj->setPosition(pos);
			obj->setPosition({ 0.0f, 0.0f, 0.0f });

            // Pull out a formatted uint32 image
            auto pixels = img.getFormattedImg();

            // Display
			dispWindow.activate();
			surf->shaders.use();
			normalTex.setImage(pixels.get());
            dispWindow.render();
        }
    }
    catch (std::exception e) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Runtime error", e.what(), nullptr);
    }
	catch (char* e) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Runtime error", e, nullptr);
	}
	catch (std::string e) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Runtime error", e.c_str(), nullptr);
	}

    // Explicit cleanup (mainly for SDL subsystems)
    GLWindow::ReleaseGUI();
    return 0;
}
