#include "stdafx.h"
#include "NormDepthImage.h"

void NormDepthImage::calcNormals(float camXZ, float camYZ) {
    using namespace glm;
    Pt2i pt;

    auto dd = _depth.get();
    auto nd = _data.get();

    // First pass calculates the camera-space position of each depth pixel
    for (pt.y = 0; pt.y < dim.height; pt.y++) {
        for (pt.x = 0; pt.x < dim.width; pt.x++) {
            vec3 pos;
            pos.z = getDepth(pt);

            pos.x = ((float)pt.x / (float)dim.width - 0.5f) * 2 * pos.z * camXZ;
            pos.y = ((float)pt.y / (float)dim.height - 0.5f) * 2 * pos.z * camYZ;

            setWPix(pt, pos);
        }
    }

    // Second pass resolves the normals based on local differencing (dFdx / dFdy)
    for (pt.y = 0; pt.y < dim.height; pt.y++) {
        for (pt.x = 0; pt.x < dim.width; pt.x++) {
            vec3 norm, a, b;
            vec3 pos = getWPix(pt);

            // dFdx
            Pt2i x1, x2;
            vec3 p1, p2;
            x1 = pt.offs(-1, 0);
            x2 = pt.offs( 1, 0);
            if (dim.contains(x1))  p1 = getWPix(x1);
            else                    p1 = pos;
            if (dim.contains(x2))  p2 = getWPix(x2);
            else                    p2 = pos;

            // ... a = ?
            a = p2 - p1;

            // dFdy
            x1 = pt.offs(0, -1);
            x2 = pt.offs(0,  1);
            if (dim.contains(x1))  p1 = getWPix(x1);
            else                    p1 = pos;
            if (dim.contains(x2))  p2 = getWPix(x2);
            else                    p2 = pos;

            // ... b = ?
            b = p2 - p1;

            norm = normalize(cross(normalize(a), normalize(b)));
            setPix(pt, norm);
        }
    }
}

void NormDepthImage::threshold_gaussNormalBlur(const int radius, float thresh, float dThresh) {
    using namespace glm;
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

            pixel_t p1 = getPix(pt), p2;
            pixel_t sum = p1*gauss[0];
            float count = 0;

            for (int offX = 1; offX < radius; offX++) {
                if (dim.contains(pt.offs(offX, 0))) {
                    p2 = getPix(pt.offs(offX, 0));
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
                    p2 = getPix(pt.offs(-offX, 0));
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

            setWPix(pt, sum);
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

            pixel_t p1 = getWPix(pt), p2;
            pixel_t sum = p1*gauss[0];
            float count = 0;

            for (int offY = 1; offY < radius; offY++) {
                if (dim.contains(pt.offs(0, offY))) {
                    p2 = getWPix(pt.offs(0, offY));
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
                    p2 = getWPix(pt.offs(0, -offY));
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

            setPix(pt, sum);
        }
    }
}

void NormDepthImage::threshold_meanNormalBlur(int radius, float thresh) {
    using namespace glm;
    Pt2i pt;
    
    // Blur along X
    for (pt.y = 0; pt.y < dim.height; pt.y++) {
        for (pt.x = 0; pt.x < dim.width; pt.x++) {
            pixel_t p1 = getPix(pt), p2;
            pixel_t sum = p1;
            float count = 1;

            // Skip bad pixels
            if (getDepth(pt) < 0) {
                continue;
            }

            for (int offX = 1; offX < radius; offX++) {
                if (dim.contains(pt.offs(offX, 0))) {
                    p2 = getPix(pt.offs(offX, 0));
                    // Threshold
                    if (fabs(p1.r - p2.r) < thresh &&
                        fabs(p1.g - p2.g) < thresh &&
                        fabs(p1.b - p2.b) < thresh) {
                        sum += p2;
                        count++;
                    }
                }
                if (dim.contains(pt.offs(-offX, 0))) {
                    p2 = getPix(pt.offs(-offX, 0));
                    // Threshold
                    if (fabs(p1.r - p2.r) < thresh &&
                        fabs(p1.g - p2.g) < thresh &&
                        fabs(p1.b - p2.b) < thresh) {
                        sum += p2;
                        count++;
                    }
                }
            }

            setWPix(pt, sum / count);
        }
    }

    // Blur along Y
    for (pt.y = 0; pt.y < dim.height; pt.y++) {
        for (pt.x = 0; pt.x < dim.width; pt.x++) {
            pixel_t p1 = getWPix(pt), p2;
            pixel_t sum = p1;
            float count = 1;

            // Skip bad pixels
            if (getDepth(pt) < 0) {
                continue;
            }

            for (int offY = 1; offY < radius; offY++) {
                if (dim.contains(pt.offs(0, offY))) {
                    p2 = getWPix(pt.offs(0, offY));
                    // Threshold
                    if (fabs(p1.r - p2.r) < thresh &&
                        fabs(p1.g - p2.g) < thresh &&
                        fabs(p1.b - p2.b) < thresh) {
                        sum += p2;
                        count++;
                    }
                }
                if (dim.contains(pt.offs(0, -offY))) {
                    p2 = getWPix(pt.offs(0, -offY));
                    // Threshold
                    if (fabs(p1.r - p2.r) < thresh &&
                        fabs(p1.g - p2.g) < thresh &&
                        fabs(p1.b - p2.b) < thresh) {
                        sum += p2;
                        count++;
                    }
                }
            }

            setPix(pt, sum / count);
        }
    }
}

void NormDepthImage::threshold_meanDepthBlur(int radius, float thresh) {
    using namespace glm;

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
    }
}


void NormDepthImage::threshold_normalFlood(Pt2i seed, float thresh, float dThresh) {
    // Working in the mask
    clearMask();

    // Subclass contains algorithm code and structures
    _QLinearFill filler(this, thresh, dThresh);
    filler.fill(seed);
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
    pDepth = image->_depth.get();
    pNorm = image->_data.get();
    pMask = image->_mask.get();

    // Starting val
    pixel_t baseCol = image->getPix(seed);
    float baseDepth = image->getDepth(seed);

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
    pixel_t a, b;
    a = pNorm[px]; b = pNorm[cmp];

    float aD, bD;
    aD = pDepth[px]; bD = pDepth[cmp];

    return
        fabs(a.x - b.x) < tol &&
        fabs(a.y - b.y) < tol &&
        fabs(a.z - b.z) < tol &&
        fabs(aD - bD) < dTol;
}
