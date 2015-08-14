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