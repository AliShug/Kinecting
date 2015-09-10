// Kinecting.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "KinectDevice.h"

#include "GLWindow.h"
#include "NormDepthImage.h"
#include "PointCloud.h"
#include "GLObject.h"
#include "GLText.h"

using namespace std;
using namespace glm;

auto getTestPng(string file) {
    // Load a test image
    FIBITMAP *temp = FreeImage_Load(FIF_PNG, (file+".png").c_str(), PNG_DEFAULT);
    FIBITMAP *testBmp = FreeImage_ConvertTo32Bits(temp);
    FreeImage_Unload(temp);

    int imW, imH;
    imW = FreeImage_GetWidth(testBmp);
    imH = FreeImage_GetHeight(testBmp);

    auto testData = shared_ptr<uint32_t>(new uint32_t[imW * imH]);
    memcpy(testData.get(), FreeImage_GetBits(testBmp), 4 * imW * imH);
    FreeImage_Unload(testBmp);

    return testData;
}

void instructions(ostream &out) {
    out << "WS - Camera dolly | AD - Camera Strafe | QE - Camera Elevation\n";
    out << "Click/drag - Camera Look\n";
    out << "P - Pause\nR - Reset tracking\nO - Open pointcloud\nReturn - Save pointcloud\n\n";
}

mat3 pca(PointCloud &pc) {
    mat3 cov = pc.calcCov();
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
    return sortedEig;
}

int main(int argc, char *args[]) {
	HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO conInfo;
	GetConsoleScreenBufferInfo(hCon, &conInfo);
	int screenChars = (conInfo.srWindow.Bottom + 1) * (conInfo.srWindow.Right + 1);

	bool paused = false;

    // Data for testing (load from png)
    //auto testData = getTestPng("normalTestImg");

    // Buffered output
    stringstream log;
    stringstream message;

    // Projector stats
    const float projectorFovY = 27.5f;
    const float projectorFovX = 48.9f;
    const Dim projectorDim = { 845, 480 };

    //try
    {
        // Kinect and main window
        KinectDevice kinect;
        GLWindow dispWindow, projectorWindow;

        // Initialize the gui and kinect
        GLWindow::InitGUI();
        kinect.connect(kinect.DEPTH_STREAM);

        int frame_w = kinect.depthFrameInfo.w;
        int frame_h = kinect.depthFrameInfo.h;

        Dim fullSize = { frame_w, frame_h };
        Dim window = { 1280, 720 };

        // Current tracking point
        Pt2i centre = { fullSize.width / 2, fullSize.height / 2 };
        Pt2i trackPt = centre;

        // Create the GL contexts for separate windows (displaying the windows)
        dispWindow.showWindow("Kinecting - freelook", window);
        projectorWindow.showWindow("Kinecting - projector", window);
        projectorWindow.nextMonitor();
        projectorWindow.setFullscreen(true);

        // Setup on free-look context (camera)
        dispWindow.activate();
		auto &scene = dispWindow.scene;
		scene.camera.set(false, window, 60.0f);
        scene.readCameraSettings("camera.cfg");
        //

        // Setup on projector context (camera)
        // Note the offset viewport, important for the short-throw projector
        projectorWindow.activate();
        auto &projScene = projectorWindow.scene;
        projScene.camera.set(true, window, projectorFovY);
        projScene.readCameraSettings("projector.cfg");
        //

		// Storage for raw camera output
		auto rawData = unique_ptr<uint16_t>(new uint16_t[fullSize.area()]);
		auto floatData = unique_ptr<float>(new float[fullSize.area()]);

        // Image processing platform
        NormDepthImage img(fullSize);


        // Generate the main window's objects *****
        dispWindow.activate();
		
        // -- Kinect frustrum
        auto kinectFrustrum = scene.newObject("solidcolor.glsl", "object_vert.glsl");
        kinectFrustrum->genFrustrum(0.5f, 4.5f, { kinect.depthFrameInfo.xFov, kinect.depthFrameInfo.yFov });

        // -- Projector frustrum
        auto projFrustrum = scene.newObject("solidcolor.glsl", "object_vert.glsl");
        projFrustrum->genFrustrum(0.0f, 4.0f, { projectorFovX, projectorFovY }, Colors::red, true);

		//--  Camera output surface
		auto surf = scene.newObject("rgb_frag.glsl", "normal_vertex.glsl");
		surf->renderMode = GLObject::RenderMode::POINTS;
		surf->genQuad(fullSize);
        surf->pointSize = 2.0f;
		
		Texture normalTex, depthTex;
		normalTex.init(Texture::BGR, fullSize);
		depthTex.init(Texture::DEPTH_FLOAT, fullSize);

		surf->shaders.bindTexture(normalTex, "UInputImg");
		surf->shaders.bindTexture(depthTex, "UInputDepth");

		// -- Cube (test)
		auto obj = scene.newObject("object_frag.glsl", "object_vert.glsl");
		obj->genCuboid(0.1f, 0.1f, 0.1f);
        //obj->hide();

        // -- Tracking line
        auto trackLine0 = scene.newObject("solidcolor.glsl", "object_vert.glsl");
        trackLine0->renderMode = GLObject::RenderMode::LINE_STRIP;
		auto trackLine1 = scene.newObject("solidcolor.glsl", "object_vert.glsl");
		trackLine1->renderMode = GLObject::RenderMode::LINE_STRIP;
		auto trackLine2 = scene.newObject("solidcolor.glsl", "object_vert.glsl");
		trackLine2->renderMode = GLObject::RenderMode::LINE_STRIP;

		// -- Tracked cloud
		PointCloud pointCloud;
		auto frangibleCloud = scene.newObject("solidcolor.glsl", "object_vert.glsl");
		frangibleCloud->renderMode = GLObject::RenderMode::POINTS;
		frangibleCloud->pointSize = 3.0f;

		// TEMP
		// hide surface
		//surf->hide();


        // Generate the projector window's objects ****
        projectorWindow.activate();

        // -- Tracked cloud
        auto baseCloud = projScene.newObject("solidcolor.glsl", "object_vert.glsl");
        baseCloud->renderMode = GLObject::RenderMode::POINTS;
        baseCloud->pointSize = 2.0f;

        //--  Second depth/normals output surface
        auto surf2 = projScene.newObject("rgb_frag.glsl", "normal_vertex.glsl");
        surf2->renderMode = GLObject::RenderMode::POINTS;
        surf2->genQuad(fullSize);

        Texture normalTex2, depthTex2;
        normalTex2.init(Texture::BGR, fullSize);
        depthTex2.init(Texture::DEPTH_FLOAT, fullSize);

        surf2->shaders.bindTexture(normalTex2, "UInputImg");
        surf2->shaders.bindTexture(depthTex2, "UInputDepth");


        // Begin reading in frames...
        kinect.asyncListen();
        // ... and run the main loop
        SDL_Event e;
        bool quit = false;
        while (!quit) {
            while (SDL_PollEvent(&e) != 0) {
                // Handle all waiting events
                projectorWindow.handleEvent(e);
                dispWindow.handleEvent(e);

                // General (shared) application control
                if (e.type == SDL_KEYDOWN) {
                    switch (e.key.keysym.scancode) {
                        // Tracking reset
                    case SDL_SCANCODE_R:
                        trackPt = centre;
                        break;

                        // Pause
                    case SDL_SCANCODE_P:
                        paused = !paused;
                        if (paused) {
                            message.str("Paused");
                        }
                        else {
                            message.str("Running");
                        }
                        break;

                        // Cloud save/load
                    case SDL_SCANCODE_RETURN:
                        pointCloud.saveToFile("cloud.txt");
                        message.str("Saved pointcloud to 'cloud.txt'");
                        break;
                    case SDL_SCANCODE_O:
                        pointCloud.loadFromFile("cloud.txt");
                        paused = true;
                        message.str("Loaded pointcloud 'cloud.txt'");
                        break;
                    }
                }

                // Closing either window quits the application
                if (e.type == SDL_QUIT || (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE)) {
                    quit = true;
                }
            }


            {
                // Exception-proof lock for depth read
				lock_guard<decltype(kinect.frameLock)> lck(kinect.frameLock);
                memcpy(rawData.get(), kinect.depthData.get(), fullSize.area()*sizeof(uint16_t));
            }
            
            // Processing on active incoming stream
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

                projectorWindow.activate();
                depthTex2.setImage(floatData.get());

				dispWindow.activate();
				depthTex.setImage(floatData.get());

				// **************************************************
				// *** Image Processing

				// Smooth depth
				//img.threshold_meanDepthBlur(4, 0.001f);

				// Smooth and flood-fill the normals
				img.calcNormals(kinectXZ, kinectYZ);
				img.OPT_threshold_gaussNormalBlur(12, 0.8f, 0.01f);
				img.threshold_normalFlood(trackPt, 0.1f, 0.01f);

                // Now we can generate the stress map and normals
                //img.masked_laplaceSmooth(1);
                img.masked_stressMap();
                pointCloud.generateFromImage(img);
                //pointCloud.innerEdge();
			}

            // Display the point cloud (convert to object mesh data)
			frangibleCloud->genPointCloud(pointCloud);
            baseCloud->genPointCloud(Colors::white, pointCloud);

            // Tracking
            auto medPos = pointCloud.medianPoint();
            trackPt.x = int(medPos.screen.x); trackPt.y = int(medPos.screen.y);
            trackLine0->genLine(medPos.pos, vec3(0));
            obj->setPosition(medPos.pos);

            // Pull out a formatted uint32 image
            auto pixels = img.getFormattedImg();

			// Principal component analysis
			if (pointCloud.cloud.size() > 100) {
                mat3 pcaMat = pca(pointCloud);

                vec3 pos = pointCloud.meanPosition();
                trackLine0->genLine(pos, pos + pcaMat[0], Colors::red);
                trackLine1->genLine(pos, pos + pcaMat[1], Colors::green);
                trackLine2->genLine(pos, pos + pcaMat[2], Colors::blue);

                // Transform frangible cloud into calculated local space
                auto invMeanPos = translate(mat4(), -pointCloud.meanPosition());
                frangibleCloud->applyTransform(invMeanPos);
                frangibleCloud->applyTransform(inverse(pcaMat));
			}


            // Text display
            scene.getTextOverlay()->drawText({ 20, 20 }, message.str());

            // Projector frustrum
            projFrustrum->setPosition(projScene.camera.eye);
            projFrustrum->setRotation(vec3(projScene.camera.angle.y, 0.0f, M_PI/2 - projScene.camera.angle.x));

            // Display
            // TODO: change
            GLObject::projectorVP = projScene.camera.calcProjection() * projScene.camera.calcView();
			normalTex.setImage(pixels.get());
            dispWindow.render();

            // Projector display
            projectorWindow.activate();
            normalTex2.setImage(pixels.get());
            projectorWindow.render();
        }
    }
 //   catch (exception e) {
 //       SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Runtime error", e.what(), nullptr);
 //   }
	//catch (char* e) {
	//	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Runtime error", e, nullptr);
	//}
	//catch (string e) {
	//	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Runtime error", e.c_str(), nullptr);
	//}

    // Explicit cleanup (mainly for SDL subsystems)
    GLWindow::ReleaseGUI();
	#ifdef _DEBUG
		_CrtDumpMemoryLeaks();
	#endif
    return 0;
}
