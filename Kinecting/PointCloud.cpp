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

                newPoint.pos.x = ((float) pt.x / (float) source.dim.width - 0.5f) * 2 * newPoint.pos.z * camXZ;
                newPoint.pos.y = ((float) pt.y / (float) source.dim.height - 0.5f) * 2 * newPoint.pos.z * camYZ;

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