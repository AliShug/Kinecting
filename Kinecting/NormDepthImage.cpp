#include "stdafx.h"
#include "NormDepthImage.h"

using namespace glm;
using namespace std;

void NormDepthImage::calcNormals(float camXZ, float camYZ) {
    Pt2i pt;
	auto dd = _data.get();
	auto posd = _position.get();
	store_t pix;

    // First pass calculates the camera-space position of each depth pixel,
	// storing it in the position buffer for later use
    for (pt.y = 0; pt.y < dim.height; pt.y++) {
        for (pt.x = 0; pt.x < dim.width; pt.x++) {
            vec3 pos;
			pix = dd[ptInd(pt)];
			pos.z = pix.Data.m128_f32[3];

            pos.x = ((float)pt.x / (float)dim.width - 0.5f) * 2 * pos.z * camXZ;
            pos.y = ((float)pt.y / (float)dim.height - 0.5f) * 2 * pos.z * camYZ;

            posd[ptInd(pt)] = store_t(pos, pos.z);
        }
    }

    // Second pass resolves the normals based on local differencing (dFdx / dFdy)
    for (pt.y = 0; pt.y < dim.height; pt.y++) {
        for (pt.x = 0; pt.x < dim.width; pt.x++) {
            vec3 norm, a, b;
			vec3 pos = vec4_cast(posd[ptInd(pt)]).xyz;

            // dFdx
            Pt2i x1, x2;
            vec3 p1, p2;
            x1 = pt.offs(-1, 0);
            x2 = pt.offs( 1, 0);
            if (dim.contains(x1))  p1 = vec4_cast(posd[ptInd(x1)]).xyz;
            else                   p1 = pos;
            if (dim.contains(x2))  p2 = vec4_cast(posd[ptInd(x2)]).xyz;
            else                   p2 = pos;

            // ... a = ?
            a = p2 - p1;

            // dFdy
            x1 = pt.offs(0, -1);
            x2 = pt.offs(0,  1);
            if (dim.contains(x1))  p1 = vec4_cast(posd[ptInd(x1)]).xyz;
            else                   p1 = pos;
            if (dim.contains(x2))  p2 = vec4_cast(posd[ptInd(x2)]).xyz;
            else                   p2 = pos;

            // ... b = ?
            b = p2 - p1;

            norm = normalize(cross(normalize(a), normalize(b)));
            dd[ptInd(pt)] = store_t(norm, pos.z);

			if (pos.z < 0.0f) {
				dd[ptInd(pt)] = store_t(0.0f, 0.0f, 0.0f, -1.0f);
			}
        }
    }
}

void NormDepthImage::threshold_gaussNormalBlur(const int radius, float thresh, float dThresh) {
	OPT_threshold_gaussNormalBlur(radius, thresh, dThresh);
    /*using namespace glm;
    Pt2i pt;
    
    std::vector<float> gauss;
    const float param = powf(((float) radius) / 3, 2);
    for (int i = 0; i < radius; i++) {
        gauss.push_back(1 / sqrtf(2 * M_PI * param) * expf(-((float) (i*i)) / (2*param)));
    };

    // Blur along X
    for (pt.y = 0; pt.y < dim.height; pt.y++) {
        for (pt.x = 0; pt.x < dim.width; pt.x++) {
            // Skip bad pixels
            float d1 = getDepth(pt), d2;
            if (d1 < 0) {
                continue;
            }

            normal_t p1 = getNorm(pt), p2;
            normal_t sum = p1*gauss[0];
            float count = 0;

            for (int offX = 1; offX < radius; offX++) {
                if (dim.contains(pt.offs(offX, 0))) {
                    p2 = getNorm(pt.offs(offX, 0));
                    d2 = getDepth(pt.offs(offX, 0));

                    // Threshold
                    if (fabs(p1.r - p2.r) < thresh &&
                        fabs(p1.g - p2.g) < thresh &&
                        fabs(p1.b - p2.b) < thresh &&
                        fabs(d1 - d2) < dThresh) {
                        sum += p2*gauss[offX];
                        count++;
                    }
                    else {
                        sum += p1*gauss[offX];
                    }
                }
                if (dim.contains(pt.offs(-offX, 0))) {
                    p2 = getNorm(pt.offs(-offX, 0));
                    d2 = getDepth(pt.offs(-offX, 0));

                    // Threshold
                    if (fabs(p1.r - p2.r) < thresh &&
                        fabs(p1.g - p2.g) < thresh &&
                        fabs(p1.b - p2.b) < thresh &&
                        fabs(d1 - d2) < dThresh) {
                        sum += p2*gauss[offX];
                        count++;
                    }
                    else {
                        sum += p1*gauss[offX];
                    }
                }
            }

            setWNorm(pt, sum);
            //setWPix(pt, pixel(1, 0, 0));
        }
    }

    // Blur along Y
    for (pt.y = 0; pt.y < dim.height; pt.y++) {
        for (pt.x = 0; pt.x < dim.width; pt.x++) {
            // Skip bad pixels
            float d1 = getDepth(pt), d2;
            if (d1 < 0) {
                continue;
            }

            normal_t p1 = getWNorm(pt), p2;
            normal_t sum = p1*gauss[0];
            float count = 0;

            for (int offY = 1; offY < radius; offY++) {
                if (dim.contains(pt.offs(0, offY))) {
                    p2 = getWNorm(pt.offs(0, offY));
                    d2 = getDepth(pt.offs(0, offY));

                    // Threshold
                    if (fabs(p1.r - p2.r) < thresh &&
                        fabs(p1.g - p2.g) < thresh &&
                        fabs(p1.b - p2.b) < thresh &&
                        fabs(d1 - d2) < dThresh) {
                        sum += p2*gauss[offY];
                        count++;
                    }
                    else {
                        sum += p1*gauss[offY];
                    }
                }
                if (dim.contains(pt.offs(0, -offY))) {
                    p2 = getWNorm(pt.offs(0, -offY));
                    d2 = getDepth(pt.offs(0, -offY));

                    // Threshold
                    if (fabs(p1.r - p2.r) < thresh &&
                        fabs(p1.g - p2.g) < thresh &&
                        fabs(p1.b - p2.b) < thresh &&
                        fabs(d1 - d2) < dThresh) {
                        sum += p2*gauss[offY];
                        count++;
                    }
                    else {
                        sum += p1*gauss[offY];
                    }
                }
            }

            setNorm(pt, sum);
        }
    }*/
}

void NormDepthImage::OPT_threshold_gaussNormalBlur(const int radius, float thresh, float dThresh) {
    Pt2i pt;

    // Gaussian values pre-distributed into vector of vector-types for fast SIMD multiplication
    store_p gauss(aligned_malloc<store_t>(radius, 16));
    const float param = powf(((float) radius) / 3, 2);
    for (int i = 0; i < radius; i++) {
        gauss.get()[i] = store_t(1 / sqrtf(2 * M_PI * param) * expf(-((float) (i*i)) / (2 * param)));
    };

    // Data
    store_t p1, p2, sum, diff;
    __m128 res;
	auto g = gauss.get();
    auto n = _data.get();
    auto wn = _workingData.get();

    // Comparison values for threshold
    auto test = store_t(thresh, thresh, thresh, dThresh);
    auto allOnes = store_t(-1.0f);

    // Blur along X
    int i = 0, j = 0;
    for (pt.y = 0; pt.y < dim.height; pt.y++) {
        j = pt.y * dim.width;
        for (pt.x = 0; pt.x < dim.width; pt.x++) {
            i = j + pt.x;

            // Skip bad pixels
            p1 = n[i];
            // depth in w-component
            if (p1.Data.m128_f32[3] < 0) {
                continue;
            }

            // Running total
            sum = p1*g[0];

            // left
            for (int offX = 1; offX < radius; offX++) {
                if (0 <= pt.x - offX) {
                    p2 = n[i - offX];

                    // Threshold
                    // sub and compare to threshold values
					diff = p2 - p1;
					for (int i = 0; i < 4; i++) {
						diff.Data.m128_f32[i] = fabs(diff.Data.m128_f32[i]);
					}
                    res = _mm_cmp_ps(diff.Data, test.Data, _CMP_LT_OQ);
                    // check they're all 1 (<thresh)
                    if (_mm_testc_ps(res, allOnes.Data)) {
                        sum += p2*g[offX];
                    }
                    else {
                        sum += p1*g[offX];
                    }
                }
				else continue;
            }
            // right
            for (int offX = 1; offX < radius; offX++) {
                if (pt.x + offX < dim.width) {
                    p2 = n[i + offX];

                    // Threshold
                    // sub and compare to threshold values
					diff = p2 - p1;
					for (int i = 0; i < 4; i++) {
						diff.Data.m128_f32[i] = fabs(diff.Data.m128_f32[i]);
					}
					res = _mm_cmp_ps(diff.Data, test.Data, _CMP_LT_OQ);
                    // check they're all 1 (<thresh)
                    if (_mm_testc_ps(res, allOnes.Data)) {
                        sum += p2*g[offX];
                    }
                    else {
                        sum += p1*g[offX];
                    }
                }
				else continue;
            }


            sum.Data.m128_f32[3] = 0.0f;
			sum = glm::fastNormalize(sum);
			sum.Data.m128_f32[3] = p1.Data.m128_f32[3];
            wn[i] = sum;
        }
    }

    // Blur along Y
    for (pt.y = 0; pt.y < dim.height; pt.y++) {
        j = pt.y * dim.width;
        for (pt.x = 0; pt.x < dim.width; pt.x++) {
            i = j + pt.x;

            // Skip bad pixels
            p1 = wn[i];
            if (p1.Data.m128_f32[3] < 0) {
                continue;
            }

            sum = p1*g[0];

            // below
            for (int offY = 1; offY < radius; offY++) {
                if (0 <= pt.y - offY) {
                    p2 = wn[i - offY*dim.width];

                    // Threshold
                    // sub and compare to threshold values
					diff = p2 - p1;
					for (int i = 0; i < 4; i++) {
						diff.Data.m128_f32[i] = fabs(diff.Data.m128_f32[i]);
					}
					res = _mm_cmp_ps(diff.Data, test.Data, _CMP_LT_OQ);
                    // check they're all 1 (<thresh)
                    if (_mm_testc_ps(res, allOnes.Data)) {
                        sum += p2*g[offY];
                    }
                    else {
                        sum += p1*g[offY];
                    }
                }
				else continue;
            }
            // above
            for (int offY = 1; offY < radius; offY++) {
                if (pt.y + offY < dim.height) {
                    p2 = wn[i + offY*dim.width];

                    // Threshold
                    // sub and compare to threshold values
					diff = p2 - p1;
					for (int i = 0; i < 4; i++) {
						diff.Data.m128_f32[i] = fabs(diff.Data.m128_f32[i]);
					}
					res = _mm_cmp_ps(diff.Data, test.Data, _CMP_LT_OQ);
                    // check they're all 1 (<thresh)
                    if (_mm_testc_ps(res, allOnes.Data)) {
                        sum += p2*g[offY];
                    }
                    else {
                        sum += p1*g[offY];
                    }
                }
				else continue;
            }

            sum.Data.m128_f32[3] = 0.0f;
			sum = glm::fastNormalize(sum);
			sum.Data.m128_f32[3] = p1.Data.m128_f32[3];
			n[i] = sum;
        }
    }
}

void NormDepthImage::threshold_meanNormalBlur(int radius, float thresh) {
	throw "Unsupported operation";
    /*
    Pt2i pt;
    
    // Blur along X
    for (pt.y = 0; pt.y < dim.height; pt.y++) {
        for (pt.x = 0; pt.x < dim.width; pt.x++) {
            normal_t p1 = getNorm(pt), p2;
            normal_t sum = p1;
            float count = 1;

            // Skip bad pixels
            if (getDepth(pt) < 0) {
                continue;
            }

            for (int offX = 1; offX < radius; offX++) {
                if (dim.contains(pt.offs(offX, 0))) {
                    p2 = getNorm(pt.offs(offX, 0));
                    // Threshold
                    if (fabs(p1.r - p2.r) < thresh &&
                        fabs(p1.g - p2.g) < thresh &&
                        fabs(p1.b - p2.b) < thresh) {
                        sum += p2;
                        count++;
                    }
                }
                if (dim.contains(pt.offs(-offX, 0))) {
                    p2 = getNorm(pt.offs(-offX, 0));
                    // Threshold
                    if (fabs(p1.r - p2.r) < thresh &&
                        fabs(p1.g - p2.g) < thresh &&
                        fabs(p1.b - p2.b) < thresh) {
                        sum += p2;
                        count++;
                    }
                }
            }

            setWNorm(pt, sum / count);
        }
    }

    // Blur along Y
    for (pt.y = 0; pt.y < dim.height; pt.y++) {
        for (pt.x = 0; pt.x < dim.width; pt.x++) {
            normal_t p1 = getWNorm(pt), p2;
            normal_t sum = p1;
            float count = 1;

            // Skip bad pixels
            if (getDepth(pt) < 0) {
                continue;
            }

            for (int offY = 1; offY < radius; offY++) {
                if (dim.contains(pt.offs(0, offY))) {
                    p2 = getWNorm(pt.offs(0, offY));
                    // Threshold
                    if (fabs(p1.r - p2.r) < thresh &&
                        fabs(p1.g - p2.g) < thresh &&
                        fabs(p1.b - p2.b) < thresh) {
                        sum += p2;
                        count++;
                    }
                }
                if (dim.contains(pt.offs(0, -offY))) {
                    p2 = getWNorm(pt.offs(0, -offY));
                    // Threshold
                    if (fabs(p1.r - p2.r) < thresh &&
                        fabs(p1.g - p2.g) < thresh &&
                        fabs(p1.b - p2.b) < thresh) {
                        sum += p2;
                        count++;
                    }
                }
            }

            setNorm(pt, sum / count);
        }
    }*/
}

void NormDepthImage::threshold_meanDepthBlur(int radius, float thresh) {
	throw "Unsupported operation";
    /*

    Pt2i pt;
    
    // Blur along X
    for (pt.y = 0; pt.y < dim.height; pt.y++) {
        for (pt.x = 0; pt.x < dim.width; pt.x++) {
            float p1 = getDepth(pt), p2;
            float sum = p1;
            float count = 1;

            // Skip bad pixels
            if (p1 < 0) {
                setWDepth(pt, p1);
                continue;
            }

            for (int offX = 1; offX < radius; offX++) {
                if (dim.contains(pt.offs(offX, 0))) {
                    p2 = getDepth(pt.offs(offX, 0));
                    // Threshold
                    if (p1 - p2 < thresh) {
                        sum += p2;
                        count++;
                    }
                }
                if (dim.contains(pt.offs(-offX, 0))) {
                    p2 = getDepth(pt.offs(-offX, 0));
                    // Threshold
                    if (p1 - p2 < thresh) {
                        sum += p2;
                        count++;
                    }
                }
            }

            setWDepth(pt, sum / count);
        }
    }

    // Blur along Y
    for (pt.y = 0; pt.y < dim.height; pt.y++) {
        for (pt.x = 0; pt.x < dim.width; pt.x++) {
            float p1 = getWDepth(pt), p2;
            float sum = p1;
            float count = 1;

            // Skip bad pixels
            if (p1 < 0) {
                setDepth(pt, p1);
                continue;
            }

            for (int offY = 1; offY < radius; offY++) {
                if (dim.contains(pt.offs(0, offY))) {
                    p2 = getWDepth(pt.offs(0, offY));
                    // Threshold
                    if (p1 - p2 < thresh) {
                        sum += p2;
                        count++;
                    }
                }
                if (dim.contains(pt.offs(0, -offY))) {
                    p2 = getWDepth(pt.offs(0, -offY));
                    // Threshold
                    if (p1 - p2 < thresh) {
                        sum += p2;
                        count++;
                    }
                }
            }

            setDepth(pt, sum / count);
        }
    }*/
}


void NormDepthImage::threshold_normalFlood(Pt2i seed, float thresh, float dThresh) {
    if (dim.contains(seed)) {
        // Working in the mask
        clearMask();

        // Subclass contains algorithm code and structures
        _QLinearFill filler(this, thresh, dThresh);
        filler.fill(seed);
    }
    else {
        std::cerr << "Flood-fill seed outside range" << std::endl;
    }
}

// *****************************************************************************************************************
// ** Stress-map generation

void NormDepthImage::masked_laplaceSmooth(int iterations) {
    /*Pt2i pt;
    auto dd = _data.get();
    auto pos = _position.get();

    vector<vec3> points;
    points.reserve(4);

    for (int i = 0; i < iterations; i++) {
        for (pt.y = 0; pt.y < dim.height; pt.y++) {
            for (pt.x = 0; pt.x < dim.width; pt.x++) {
                auto pdata = pos[ptInd(pt)];
                vec3 p = *reinterpret_cast<vec3*>(&pdata);
                
                auto here = pos[ptInd(pt)];
                Pt2i offsPt = pt.offs(0, 1);
                auto data0 = (offsPt.y < dim.height && getMask(offsPt) == PICKED) ? pos[ptInd(offsPt)] : here;
                offsPt = pt.offs(0, -1);
                auto data1 = (offsPt.y >= 0 && getMask(offsPt) == PICKED) ? pos[ptInd(offsPt)] : here;
                offsPt = pt.offs(-1, 0);
                auto data2 = (offsPt.x >= 0 && getMask(offsPt) == PICKED) ? pos[ptInd(offsPt)] : here;
                offsPt = pt.offs(1, 0);
                auto data3 = (offsPt.x < dim.width && getMask(offsPt) == PICKED) ? pos[ptInd(offsPt)] : here;

                vec3 l, r, u, d, m;
                m = *reinterpret_cast<vec3*>(&here);
                u = *reinterpret_cast<vec3*>(&data0);
                d = *reinterpret_cast<vec3*>(&data1);
                l = *reinterpret_cast<vec3*>(&data2);
                r = *reinterpret_cast<vec3*>(&data3);

                vec3 deltaP = 0.25f * (u + d + l + r) - m;
                vec3 newPoint = m + 0.5f * deltaP;

                pos[ptInd(pt)] = store_t(newPoint, newPoint.z);
                auto data = dd[ptInd(pt)];
                data.Data.m128_f32[3] = newPoint.z;
                dd[ptInd(pt)] = data;
            }
        }
    }*/
}

void NormDepthImage::masked_stressMap() {
    Pt2i pt;
    auto n = _data.get();
    auto stress = _stress.get();
    auto pos = _workingData.get();
    store_t pix;

    int offsetDist = 12;

    for (pt.y = 0; pt.y < dim.height; pt.y++) {
        for (pt.x = 0; pt.x < dim.width; pt.x++) {
            if (getMask(pt) != PICKED) continue;

            // Attempt 1 - difference in adjacent normals
            /*float diff = 0;
            simdVec4 myNormal = n[ptInd(pt)];
            myNormal.Data.m128_f32[3] = 0.0f;
            simdVec4 normals[4];

            Pt2i offsPt = pt.offs(0, 1);
            normals[0] = (offsPt.y < dim.height && getMask(offsPt) == PICKED) ? n[ptInd(offsPt)] : myNormal;
            offsPt = pt.offs(0, -1);
            normals[1] = (offsPt.y >= 0 && getMask(offsPt) == PICKED) ? n[ptInd(offsPt)] : myNormal;
            offsPt = pt.offs(-1, 0);
            normals[2] = (offsPt.x >= 0 && getMask(offsPt) == PICKED) ? n[ptInd(offsPt)] : myNormal;
            offsPt = pt.offs(1, 0);
            normals[3] = (offsPt.x < dim.width && getMask(offsPt) == PICKED) ? n[ptInd(offsPt)] : myNormal;

            for (int i = 0; i < 4; i++) {
                normals[i].Data.m128_f32[3] = 0.0f;
                // dirty, dirty pointer stuff
                vec3 a = *reinterpret_cast<vec3*>(&myNormal.Data);
                vec3 b = *reinterpret_cast<vec3*>(&normals[i].Data);
                diff += angle(normalize(a), normalize(b));
            }

            stress[ptInd(pt)] = diff*diff * 5.0f;*/

            // Attempt 2 - difference in normals for offset depth
            auto here = pos[ptInd(pt)];
            Pt2i offsPt = pt.offs(0, offsetDist);
            auto data0 = (offsPt.y < dim.height && getMask(offsPt) == PICKED) ? pos[ptInd(offsPt)] : here;
            offsPt = pt.offs(0, -offsetDist);
            auto data1 = (offsPt.y >= 0 && getMask(offsPt) == PICKED) ? pos[ptInd(offsPt)] : here;
            offsPt = pt.offs(-offsetDist, 0);
            auto data2 = (offsPt.x >= 0 && getMask(offsPt) == PICKED) ? pos[ptInd(offsPt)] : here;
            offsPt = pt.offs(offsetDist, 0);
            auto data3 = (offsPt.x < dim.width && getMask(offsPt) == PICKED) ? pos[ptInd(offsPt)] : here;

            vec3 l, r, u, d, m, a, b, n1, n2;
            m = *reinterpret_cast<vec3*>(&here);
            u = *reinterpret_cast<vec3*>(&data0);
            d = *reinterpret_cast<vec3*>(&data1);
            l = *reinterpret_cast<vec3*>(&data2);
            r = *reinterpret_cast<vec3*>(&data3);

            // Normal 1 left-up
            a = m - l;
            b = m - u;
            n1 = normalize(cross(normalize(a), normalize(b)));

            // Normal 2 right-down
            a = m - r;
            b = m - d;
            n2 = normalize(cross(normalize(a), normalize(b)));

            stress[ptInd(pt)] = angle(n1, n2);
        }
    }
}

// *****************************************************************************************************************
// ** Queue-Linear flood fill algorithm code (here be dragons)

void NormDepthImage::_QLinearFill::fill(Pt2i seed) {
    using namespace std;

    // Clear queue
    rangeQueue = decltype(rangeQueue)();

    // Clear 'checked'
    int area = size.area();
    auto c = checked.get();
    for (int i = 0; i < area; i++) {
        c[i] = 0;
    }

    // Grab pointers
    //pDepth = image->_depth.get();
    //pNorm = image->_data.get();
    pMask = image->_mask.get();
    pData = image->_data.get();

    // Starting val
    auto baseVal = pData[image->ptInd(seed)];

    // First call
    linFill(seed.x, seed.y);

    // Keep calling as long as there are ranges on the queue
    while (!rangeQueue.empty()) {
        FillRange r = rangeQueue.front();
        rangeQueue.pop();

        // Check above and below each pixel in the floodfill range
        int downPxI = (size.width * (r.y + 1)) + r.startX;
        int upPxI = (size.width * (r.y - 1)) + r.startX;
        int upY = r.y - 1;
        int downY = r.y + 1;
        int tmpI;

        for (int i = r.startX; i <= r.endX; i++) {
            // Start fill upwards
            tmpI = i + size.width*upY;
            if (r.y > 0 && (!c[upPxI]) && checkPx(tmpI, tmpI + size.width)) {
                linFill(i, upY);
            }

            tmpI = i + size.width*downY;
            if (r.y < (size.height - 1) && (!c[downPxI]) && checkPx(tmpI, tmpI - size.width)) {
                linFill(i, downY);
            }

            downPxI++;
            upPxI++;
        }
    }
}

void NormDepthImage::_QLinearFill::linFill(int x, int y) {
    int lFill = x;
    int ind = x + size.width*y;

    auto c = checked.get();

    while (true) {
        pMask[ind] = Mask::PICKED;
        c[ind] = 1;
        lFill--;
        ind--;

        // Exit at edge of img or colour area
        if (lFill <= 0 || c[ind] || !checkPx(ind, ind + 1)) {
            break;
        }
    }
    lFill++;

    int rFill = x;
    ind = x + size.width*y;
    while (true) {
        pMask[ind] = Mask::PICKED;
        c[ind] = 1;
        rFill++;
        ind++;

        // Exit at right edge of img or colour area
        if (rFill >= size.width || c[ind] || !checkPx(ind, ind - 1)) {
            break;
        }
    }
    rFill--;

    // Add range to queue
    rangeQueue.push(FillRange(lFill, rFill, y));
}

inline bool NormDepthImage::_QLinearFill::checkPx(int px, int cmp) {
    const store_t allOnes(-1.0f);
    store_t a, b;
    __m128 res;
    a = pData[px]; b = pData[cmp];

	if (b.Data.m128_f32[3] <= 0.5f) return false;

    // Test if abs(a-b) < tol | dTol per-component
	store_t diff = a - b;
	for (int i = 0; i < 4; i++) {
		diff.Data.m128_f32[i] = fabs(diff.Data.m128_f32[i]);
	}
	res = _mm_cmp_ps(diff.Data, tol.Data, _CMP_LT_OQ);
    // check they're all 1 (<thresh)
    return _mm_testc_ps(res, allOnes.Data) != 0;
}
