#include "stdafx.h"
#include "PointCloud.h"

PointCloud::PointCloud(NormDepthImage &source, float camXZ, float camYZ) {
    // Maketh space
    _cloud.reserve(source.dim.area());

    // Calculate each selected pixel's camera-space position
    Pt2i pt;
    for (pt.y = 0; pt.y < source.dim.height; pt.y++) {
        for (pt.x = 0; pt.x < source.dim.width; pt.x++) {
            if (source.getMask(pt) == NormDepthImage::PICKED) {
                point_t newPoint;
                newPoint.pos.z = source.getDepth(pt);
                newPoint.screen.x = pt.x;
                newPoint.screen.y = pt.y;
                newPoint.pos.x = (float(pt.x) / float(source.dim.width) - 0.5f) * 2 * newPoint.pos.z * camXZ;
                newPoint.pos.y = ((1.0f - float(pt.y) / float(source.dim.height)) - 0.5f) * 2 * newPoint.pos.z * camYZ;

                _cloud.push_back(newPoint);
            }
        }
    }
}

PointCloud::position_t PointCloud::meanPosition() {
    position_t sum(0);
    for (auto p : _cloud) {
        sum += p.pos;
    }

    return sum / ((float) _cloud.size());
}

PointCloud::point_t PointCloud::medianPoint() {
    size_t n = _cloud.size() / 2;

    // Partial ordering on pos-x
    std::nth_element(_cloud.begin(), _cloud.begin() + n, _cloud.end(),
        [](const point_t &a, const point_t &b) -> bool {
        return a.pos.x < b.pos.x;
    });
    auto ptx = _cloud[n];

    // Partial ordering on pos-y
    std::nth_element(_cloud.begin(), _cloud.begin() + n, _cloud.end(),
        [](const point_t &a, const point_t &b) -> bool {
        return a.pos.y < b.pos.y;
    });
    auto pty = _cloud[n];

    // Construct the median point
    point_t pt;
    pt.pos.x = ptx.pos.x; pt.screen.x = ptx.screen.x;
    pt.pos.y = pty.pos.y; pt.screen.y = pty.screen.y;
    pt.pos.z = (ptx.pos.z + ptx.pos.z) / 2;
    pt.norm = (ptx.norm + pty.norm) / 2.0f;

    return pt;
}