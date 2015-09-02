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

void instructions(std::ostream &out) {
    out << "WS - Camera dolly | AD - Camera Strafe | QE - Camera Elevation\n";
    out << "Click/drag - Camera Look\n";
    out << "P - Pause\nR - Reset tracking\nO - Open pointcloud\nReturn - Save pointcloud\n\n";
}

int main(int argc, char *args[]) {
	HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO conInfo;
	GetConsoleScreenBufferInfo(hCon, &conInfo);
	int screenChars = (conInfo.srWindow.Bottom + 1) * (conInfo.srWindow.Right + 1);

    KinectDevice kinect;
    GLWindow dispWindow;

	bool paused = false;

    // Data for testing (load from png)
    //auto testData = getTestPng("normalTestImg");

    // Buffered output
    std::stringstream log;

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
		std::cout << scene.camera.fov.x;

		const float camXZ = tanf((70.6f * M_PI / 180.0f) / 2.0f);
		const float camYZ = tanf((60.0f * M_PI / 180.0f) / 2.0f);

		// Storage for raw camera output
		auto rawData = std::unique_ptr<uint16_t>(new uint16_t[fullSize.area()]);
		auto floatData = std::unique_ptr<float>(new float[fullSize.area()]);

        // Image processing platform
        NormDepthImage img(fullSize);

		
		// Camera output surface
		auto surf = scene.newObject("rgb_frag.glsl", "normal_vertex.glsl");
		surf->renderMode = GLObject::RenderMode::POINTS;
		surf->genQuad(fullSize);
		
		Texture normalTex, depthTex;
		normalTex.init(Texture::BGR, fullSize);
		depthTex.init(Texture::DEPTH_FLOAT, fullSize);

		surf->shaders.bindTexture(normalTex, "UInputImg");
		surf->shaders.bindTexture(depthTex, "UInputDepth");

		// Cube (test)
		auto obj = scene.newObject("object_frag.glsl", "object_vert.glsl");
		obj->genCuboid(0.1f, 0.1f, 0.1f);

        // Tracking line
        auto trackLine0 = scene.newObject("solidcolor.glsl", "object_vert.glsl");
        trackLine0->renderMode = GLObject::RenderMode::LINE_STRIP;
		auto trackLine1 = scene.newObject("solidcolor.glsl", "object_vert.glsl");
		trackLine1->renderMode = GLObject::RenderMode::LINE_STRIP;
		auto trackLine2 = scene.newObject("solidcolor.glsl", "object_vert.glsl");
		trackLine2->renderMode = GLObject::RenderMode::LINE_STRIP;

		// Tracked cloud
		PointCloud pointCloud;
		auto frangibleCloud = scene.newObject("solidcolor.glsl", "object_vert.glsl");
		frangibleCloud->renderMode = GLObject::RenderMode::POINTS;
		frangibleCloud->pointSize = 3.0f;
        auto baseCloud = scene.newObject("solidcolor.glsl", "object_vert.glsl");
        baseCloud->renderMode = GLObject::RenderMode::POINTS;
        baseCloud->pointSize = 2.0f;

        // Current tracking point
        Pt2i centre = { fullSize.width / 2, fullSize.height / 2 };
        Pt2i trackPt = centre;

		// TEMP
		// hide surface
		surf->hide();


        // Begin reading in frames...
        kinect.asyncListen();
        // ... and run the main loop
        SDL_Event e;
        bool quit = false;
        while (!quit) {
            while (SDL_PollEvent(&e) != 0) {
                // Handle all waiting events
                dispWindow.handleEvent(e);

                // Tracking reset
                if (e.type == SDL_KEYDOWN) {
                    if (e.key.keysym.scancode == SDL_SCANCODE_R) {
                        trackPt = centre;
                    }
					else if (e.key.keysym.scancode == SDL_SCANCODE_P) {
						paused = !paused;
					}
					else if (e.key.keysym.scancode == SDL_SCANCODE_RETURN) {
						pointCloud.saveToFile("cloud.txt");
						std::cout << "Saved pointcloud to 'cloud.txt'" << std::endl;
					}
                    else if (e.key.keysym.scancode == SDL_SCANCODE_O) {
                        pointCloud.loadFromFile("cloud.txt");
                        paused = true;
                    }
                }

                if (e.type == SDL_QUIT || (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE)) {
                    quit = true;
                }
            }


            {
                // Exception-proof lock for depth read
				std::lock_guard<decltype(kinect.frameLock)> lck(kinect.frameLock);
                memcpy(rawData.get(), kinect.depthData.get(), fullSize.area()*sizeof(uint16_t));
            }
            
			if (!paused) {
				// Convert depth to float-32 (and mm to m)
				const int count = frame_w * frame_h;
				auto fd = floatData.get();
				auto rd = rawData.get();

				for (int i = 0; i < count; i++) {
					fd[i] = float(rd[i]) / 1.0f;
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
				img.calcNormals(camXZ, camYZ);

				// Blur preprocess
				img.OPT_threshold_gaussNormalBlur(12, 0.8f, 0.01f);
				//img.threshold_meanNormalBlur(4, 0.28f);

				// Flood-fill
				img.threshold_normalFlood(trackPt, 0.1f, 0.01f);
			}


			// **************************************************
			// *** Point Cloud

            // Now we can generate a point-cloud
            pointCloud.generateFromImage(img, camXZ, camYZ);
            // Display it
			frangibleCloud->genPointCloud(pointCloud);
            baseCloud->genPointCloud({ 1, 0, 0 }, pointCloud);

            // re-track
            auto medPos = pointCloud.medianPoint();
            trackPt.x = int(medPos.screen.x); trackPt.y = int(medPos.screen.y);

            // tracking line
            trackLine0->genLine(medPos.pos, glm::vec3(0));

            // tracking object
            obj->setPosition(medPos.pos);

            // Pull out a formatted uint32 image
            auto pixels = img.getFormattedImg();

			// Principal component analysis
			if (pointCloud.cloud.size() > 100) {
                // Convenience
                using namespace glm;

				// Set up for logging
                if (!paused) {
                    DWORD nul;
                    FillConsoleOutputCharacter(hCon, ' ', screenChars, { 0, 0 }, &nul);
                }
				SetConsoleCursorPosition(hCon, { 0, 0 });

                instructions(log);
                log << (paused ? "--PAUSED--" : "--RUNNING--") << std::endl;

                // Linear algebra - let the library deal with the tricky stuff
				mat3 cov = pointCloud.calcCov();
				Eigen::Matrix3f mat = Eigen::Map<Eigen::Matrix3f>(value_ptr(cov));
				Eigen::EigenSolver<Eigen::Matrix3f> solver(mat);
				Eigen::Vector3f eigenVals = solver.eigenvalues().real();
				Eigen::Matrix3f eigenVecs = solver.eigenvectors().real();

                // Matrix containing eigenvectors in COLUMNS
                mat3 eigM;
                memcpy(value_ptr(eigM), eigenVecs.data(), sizeof(mat3));

                // Sort on eigenvalues (quick bubble-sort)
                float x = 0.0f, y = 0.0f, z = 0.0f;
                int xI = 0, yI = 0, zI = 0;
                for (int i = 0; i < 3; i++) {
                    float e = eigenVals[i];
                    if (e > x) {
                        // shuffle down
                        z = y;
                        zI = yI;
                        y = x;
                        yI = xI;
                        x = e;
                        xI = i;
                    }
                    else if (e > y) {
                        z = y;
                        zI = yI;
                        y = e;
                        yI = i;
                    }
                    else {
                        z = e;
                        zI = i;
                    }
                }

                log << std::setprecision(2);
                log << "Eigenvalues        " << eigenVals[0] << "\t" << eigenVals[1] << "\t" << eigenVals[2] << std::endl;
                log << "Sorted eigenvalues " << x << "\t" << y << "\t" << z << std::endl;

                // Ordered eigenvectors
                mat3 sortedEig;
                sortedEig[0] = eigM[xI];
                sortedEig[1] = eigM[yI];
                sortedEig[2] = eigM[zI];
                static mat3 prevEig = sortedEig;

                // Stabilization - match up the dot-sign
                for (int i = 0; i < 3; i++) {
                    sortedEig[i] *= (dot(sortedEig[i], prevEig[i]) < 0) ? -1 : 1;
                }
                prevEig = sortedEig;

                log << "Eigenvectors: columnwise - unsorted - unstabilized --" << eigM << std::endl;
                log << "Eigenvectors: columnwise - sorted   - stabilized   --" << sortedEig << std::endl;

                // Visualise PCA transform
                vec3 pos = pointCloud.meanPosition();
                trackLine0->genLine(pos, pos + sortedEig[0], Colors::red);
                trackLine1->genLine(pos, pos + sortedEig[1], Colors::green);
                trackLine2->genLine(pos, pos + sortedEig[2], Colors::blue);

				// Transform frangible cloud into calculated local space
				auto invMeanPos = translate(mat4(), -pointCloud.meanPosition());
				frangibleCloud->applyTransform(invMeanPos);
				frangibleCloud->applyTransform(inverse(sortedEig));

                // Flush output
                std::cout << log.str();
                log.str("");
			}


            // Display
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
	#ifdef _DEBUG
		_CrtDumpMemoryLeaks();
	#endif
    return 0;
}
