#pragma once
#include "stdafx.h"
#include "NormDepthImage.h"

class PointCloud {
public:
    typedef glm::vec3 position_t;
    typedef Pt2i screenpos_t;
    typedef glm::vec3 normal_t;
	typedef glm::vec3 color_t;

    struct point_t {
        screenpos_t screen;
        position_t pos;
        normal_t norm;
		color_t col = color_t(0.5f, 0.5f, 1.0f);
        char adj = AJ_NONE;
        float stress = 0.0f;
    };

    enum adjacency {
        AJ_NONE = 0,
        AJ_LEFT = 1,
        AJ_RIGHT = 2,
        AJ_UP = 4,
        AJ_DOWN = 8,
        AJ_ALL = 0xF
    };

	typedef std::vector<point_t> cloud_t;

    // Generate a point cloud from an (optionally) masked normal/depth image
    void generateFromImage(NormDepthImage &source, float camXZ, float camYZ);

    void innerEdge();

	PointCloud() = default;
    ~PointCloud() = default;

    // Average position
    const position_t meanPosition();
    point_t medianPoint();

	// Covariance
	const glm::mat3 calcCov();

	// Test data sets
	void loadFromFile(const std::string &file);
	void saveToFile(const std::string &file);

	// Internal cloud
    cloud_t cloud;
    std::vector<int> cloud_indices;

private:
    Dim _source_dim;
};

