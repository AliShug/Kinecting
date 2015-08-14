#include "stdafx.h"
#include "Util.h"

std::vector<std::string> splitStr(const std::string &s, const std::string &div) {
    std::vector<int> breaks;
    std::vector<std::string> parts;
    breaks.push_back(0);

    for (int i = 0; i < s.size(); i++) {
        char c = s[i];
        if (div.find(c) != std::string::npos) {
            breaks.push_back(i);
        }
    }

    for (int i = 0; i < breaks.size(); i++) {
        if (i < breaks.size() - 1)
            parts.push_back(s.substr(breaks[i] + 1, breaks[i + 1] - breaks[i] - 1));
        else
            parts.push_back(s.substr(breaks[i] + 1));
    }

    return parts;
}

//inline BYTE getR(uint32_t pix) { return (pix >> 2 * 8) & 0xFF; }
//inline BYTE getG(uint32_t pix) { return (pix >> 1 * 8) & 0xFF; }
//inline BYTE getB(uint32_t pix) { return (pix >> 0 * 8) & 0xFF; }