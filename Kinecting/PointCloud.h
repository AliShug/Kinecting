#pragma once
#include "stdafx.h"
#include "NormDepthImage.h"

class PointCloud {
public:
    typedef glm::vec3 position_t;
    typedef glm::vec2 screenpos_t;
    typedef glm::vec3 normal_t;
	typedef glm::vec3 color_t;

    struct point_t {
        screenpos_t screen;
        position_t pos;
        normal_t norm;
		color_t col = color_t(0.5f, 0.5f, 1.0f);
    };

	typedef std::vector<point_t> cloud_t;

    // Generate a point cloud from an (optionally) masked normal/depth image
    void generateFromImage(NormDepthImage &source, float camXZ, float camYZ);

	PointCloud() = default;
    ~PointCloud() = default;

    // Average position
    position_t meanPosition();
    point_t medianPoint();

	// Covariance
	glm::mat3 calcCov();

	// Test data sets
	void loadFromFile(const std::string &file);
	void saveToFile(const std::string &file);

	// Internal cloud
    cloud_t cloud;
};

