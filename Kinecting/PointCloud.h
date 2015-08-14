#pragma once
#include "stdafx.h"
#include "NormDepthImage.h"

class PointCloud {
public:
    typedef glm::vec3 position_t;
    typedef glm::vec3 normal_t;

    struct point_t {
        position_t pos;
        normal_t norm;
    };

    // Generate a point cloud from an (optionally) masked normal/depth image
    PointCloud(NormDepthImage &source, float camXZ, float camYZ);
    ~PointCloud() = default;

    // Average position
    position_t meanPosition();

protected:
    typedef std::vector<point_t> cloud_t;

    cloud_t _cloud;
};
