#include "stdafx.h"
#include "PointCloud.h"

using namespace glm;

void PointCloud::generateFromImage(NormDepthImage &source, float camXZ, float camYZ) {
    // Maketh space
	cloud.clear();
    cloud_indices.clear();

    cloud.reserve(source.dim.area());
    cloud_indices.reserve(source.dim.area());

    _source_dim = source.dim;

    // Calculate each selected pixel's camera-space position
    Pt2i pt;
    for (pt.y = 0; pt.y < source.dim.height; pt.y++) {
        for (pt.x = 0; pt.x < source.dim.width; pt.x++) {
            if (source.getMask(pt) == NormDepthImage::PICKED) {
                point_t newPoint;
                newPoint.pos.z = source.getDepth(pt);

				if (newPoint.pos.z > 0.5f && newPoint.pos.z < 4.5f) {
					// Transform to camera space
                    newPoint.screen.x = pt.x;
					newPoint.screen.y = pt.y;
					newPoint.pos.x = (float(pt.x) / float(source.dim.width) - 0.5f) * 2 * newPoint.pos.z * camXZ;
					newPoint.pos.y = ((1.0f - float(pt.y) / float(source.dim.height)) - 0.5f) * 2 * newPoint.pos.z * camYZ;

                    // Adjacency information
                    int ajCount = 0;
                    if (source.getMask(pt.offs(-1, 0)) == NormDepthImage::PICKED) {
                        newPoint.adj |= AJ_LEFT;
                        ajCount++;
                    }
                    if (source.getMask(pt.offs( 1, 0)) == NormDepthImage::PICKED) {
                        newPoint.adj |= AJ_RIGHT; 
                        ajCount++;
                    }
                    if (source.getMask(pt.offs(0, -1)) == NormDepthImage::PICKED) {
                        newPoint.adj |= AJ_UP; 
                        ajCount++;
                    }
                    if (source.getMask(pt.offs(0,  1)) == NormDepthImage::PICKED) {
                        newPoint.adj |= AJ_DOWN;  
                        ajCount++;
                    }

                    if (ajCount > 1) {
                        // Colour the edge points
                        if (newPoint.adj != AJ_ALL) newPoint.col = Colors::black;
                        cloud_indices.push_back(cloud.size());
                        cloud.push_back(newPoint);

                        continue; // skip to next point
                    }
				}
            }

            // No index for unused points (pixels)
            cloud_indices.push_back(-1);
        }
    }
}

void PointCloud::innerEdge() {
    if (cloud.size() < 100) return;

    for (unsigned int i = 0; i < cloud.size(); i++) {
        auto &pt = cloud[i];

        Pt2i sPt = pt.screen;
        Pt2i l, r, u, d;
        l = sPt.offs(-1,  0);
        r = sPt.offs( 1,  0);
        u = sPt.offs( 0, -1);
        d = sPt.offs( 0,  1);

        if (pt.adj & AJ_LEFT && cloud[cloud_indices[l.y*_source_dim.width + l.x]].adj != AJ_ALL ||
            pt.adj & AJ_RIGHT && cloud[cloud_indices[r.y*_source_dim.width + r.x]].adj != AJ_ALL ||
            pt.adj & AJ_UP && cloud[cloud_indices[u.y*_source_dim.width + u.x]].adj != AJ_ALL ||
            pt.adj & AJ_DOWN && cloud[cloud_indices[d.y*_source_dim.width + d.x]].adj != AJ_ALL) {
            pt.col = Colors::orange;
        }
    }
}

const PointCloud::position_t PointCloud::meanPosition() {
	if (cloud.empty()) return position_t(0.0f);

    position_t sum(0);
    for (auto p : cloud) {
        sum += p.pos;
    }

    return sum / ((float) cloud.size());
}

PointCloud::point_t PointCloud::medianPoint() {
	if (cloud.empty()) return point_t();

    size_t n = cloud.size() / 2;

    // Partial ordering on pos-x
    std::nth_element(cloud.begin(), cloud.begin() + n, cloud.end(),
        [](const point_t &a, const point_t &b) -> bool {
        return a.pos.x < b.pos.x;
    });
    auto ptx = cloud[n];

    // Partial ordering on pos-y
    std::nth_element(cloud.begin(), cloud.begin() + n, cloud.end(),
        [](const point_t &a, const point_t &b) -> bool {
        return a.pos.y < b.pos.y;
    });
    auto pty = cloud[n];

    // Construct the median point
    point_t pt;
    pt.pos.x = ptx.pos.x; pt.screen.x = ptx.screen.x;
    pt.pos.y = pty.pos.y; pt.screen.y = pty.screen.y;
    pt.pos.z = (ptx.pos.z + ptx.pos.z) / 2;
    pt.norm = (ptx.norm + pty.norm) / 2.0f;

    return pt;
}

const mat3 PointCloud::calcCov() {
	if (cloud.size() <= 1) return mat3();
	position_t mean = meanPosition();

	mat3 cov;
	for (auto p : cloud) {
		cov += outerProduct(p.pos - mean, p.pos - mean) * 1000.0f;
	}
	cov *= 1.0f / (cloud.size() - 1);

	return cov;
}

void PointCloud::loadFromFile(const std::string &file) {
	throw "Load pointcloud-file not implemented";

	std::ifstream fileIn;
	fileIn.open(file);

	while (fileIn.good()) {
		point_t pt;
		//fileIn >> "n";
		// TODO
	}

	fileIn.close();
}

void PointCloud::saveToFile(const std::string &file) {
	std::ofstream fileOut;
	fileOut.open(file, std::ofstream::trunc);

	for (auto pt : cloud) {
		fileOut << std::hexfloat << "n " << pt.norm.x << " " << pt.norm.y << " " << pt.norm.z << std::endl;
		fileOut << std::hexfloat << "p " << pt.pos.x << " " << pt.pos.y << " " << pt.pos.z << std::endl;
		fileOut << std::hexfloat << "s " << pt.screen.x << " " << pt.screen.y << " " << std::endl;
	}

	fileOut.close();
}