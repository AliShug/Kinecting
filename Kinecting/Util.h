#pragma once
#include "stdafx.h"

struct Pt2i {
    int x, y;
    inline Pt2i offs(int xoffs, int yoffs) { return { x + xoffs,y + yoffs }; }
};

struct Dim {
    int width, height;

    inline int area() { return width*height; }
    inline bool contains(const Pt2i &pt) {
        return pt.x >= 0 && pt.x < width && pt.y >= 0 && pt.y < height;
    }
};

std::vector<std::string> splitStr(const std::string &s, const std::string &div);

inline BYTE getR(uint32_t pix) { return (pix >> 2 * 8) & 0xFF; }
inline BYTE getG(uint32_t pix) { return (pix >> 1 * 8) & 0xFF; }
inline BYTE getB(uint32_t pix) { return (pix >> 0 * 8) & 0xFF; }

struct aligned_free {
	void operator()(void* p) {
		_aligned_free(p);
	}
};

template<typename T>
T* aligned_malloc(std::size_t size, std::size_t alignment) {
#ifdef _DEBUG
	return static_cast<T*>(_aligned_malloc_dbg(size * sizeof(T), alignment, __FILE__, __LINE__));
#else
	return static_cast<T*>(_aligned_malloc(size * sizeof(T), alignment));
#endif
}

namespace Colors {
    const auto red = glm::vec3(1, 0, 0);
    const auto green = glm::vec3(0, 1, 0);
    const auto blue = glm::vec3(0, 0, 1);
    const auto yellow = glm::vec3(1, 1, 0);
    const auto orange = glm::vec3(1, 0.5, 0);
    const auto black = glm::vec3(0);
    const auto white = glm::vec3(1);
}

const float kinectXZ = tanf((70.6f * M_PI / 180.0f) / 2.0f);
const float kinectYZ = tanf((60.0f * M_PI / 180.0f) / 2.0f);