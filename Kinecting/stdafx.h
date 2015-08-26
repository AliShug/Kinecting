// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// TODO: Can allow support for older windows versions
#include "targetver.h" // <-----

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <immintrin.h>
#include <malloc.h>

#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <fstream>
#include <streambuf>
#include <sstream>
#include <string>
#include <exception>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <queue>
#include <algorithm>
#include <cinttypes>

#include <Kinect.h>

#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>

#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/simd_vec4.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <FreeImage.h>

// OpenCV
//#include <opencv2/core/core.hpp>
//#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/imgproc/imgproc.hpp>

// TODO: reference additional headers your program requires here
