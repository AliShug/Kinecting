#pragma once
#include "stdafx.h"
#include "Util.h"

// Convenience class for performing image operations
// Stores vec3 normals and floating-point depth
class NormDepthImage {
public:
    // Internal types
    typedef glm::vec3 normal_t;
    typedef std::unique_ptr<normal_t> normal_p;
    typedef std::unique_ptr<float> depth_p;
    typedef std::unique_ptr<uint32_t> fmt_p;
    typedef std::unique_ptr<char> mask_p;

    enum Mask {
        NONE = 0,
        PICKED = 1
    };

    // Default constructor just allocates image space
    NormDepthImage(Dim size)
        : _data(new glm::vec3[size.area()])
        , _workingData(new glm::vec3[size.area()])
        , _depth(new float[size.area()])
        , _workingDepth(new float[size.area()])
        , _mask(new char[size.area()])
        , dim(size) {
        
        clearMask();
    }
    
    // Data constructor(s) also copy and convert data into the image
    NormDepthImage(Dim size, const uint32_t *color, const float *depth) : NormDepthImage(size) {
        setData(color);
        setDepth(depth);
    }

    // Set data on an already constructed image
    void setData(const uint32_t *data) {
        auto d = _data.get();
        for (int i = 0; i < dim.area(); i++) {
            d[i] = glm::vec3(getR(data[i]), getG(data[i]), getB(data[i]));
            d[i].z /= 255; // correct to [0..1]

            d[i].x /= 127.5f;
            d[i].x -= 1.0f;

            d[i].y /= 127.5f;
            d[i].y -= 1.0f;
        }
    }

    // Set depth on an already constructed image (in m)
    void setDepth(const float *data) {
        auto d = _depth.get();
        for (int i = 0; i < dim.area(); i++) {
            if (data[i] < 500.0f || data[i] > 4500.0f) d[i] = -100000.0f;
            else d[i] = data[i] / 1000.0f;
        }
    }

    ~NormDepthImage() = default;

    // Raw data accessors
    auto getRawNormals() { return _data.get(); }
    auto getRawDepth() { return _depth.get(); }
    auto getRawMask() { return _mask.get(); }

    // Constructs a formatted 32bpp image from the stored image
    std::unique_ptr<uint32_t> getFormattedImg() {
        auto p = fmt_p(new uint32_t[dim.area()]);

        for (int i = 0; i < dim.area(); i++) {
            auto pix = _data.get()[i];
            auto mask = _mask.get()[i];
            p.get()[i] = (mask == Mask::PICKED) ? (i%2) * 0xFF<<16 : formatPixel(pix);
        }

        return p;
    }

    // Clears the internal mask value(s)
    void clearMask() {
        memset(_mask.get(), Mask::NONE, dim.area());
    }

    // Calculates the normals from the stored depth
    void calcNormals(float camXZ, float camYZ);

    // Performs a thresholded gaussian(ish) blur on the normals
    void threshold_gaussNormalBlur(int radius, float thresh, float dThresh);
    void OPT_threshold_gaussNormalBlur(int radius, float thresh, float dThresh);

    // Performs a thresholded mean blur on the normals
    void threshold_meanNormalBlur(int radius, float thresh);

    // Performs a thresholded mean blur on the depth
    void threshold_meanDepthBlur(int radius, float thresh);


    // Performs a thresholded flood fill
    void threshold_normalFlood(Pt2i seed, float thresh, float dThresh);


    // Quick-access methods
    inline normal_t getNorm(const Pt2i &pt) { return _data.get()[pt.y*dim.width + pt.x]; }
    inline void setNorm(const Pt2i &pt, normal_t &val) { _data.get()[pt.y*dim.width + pt.x] = val; }

    inline normal_t getWNorm(const Pt2i &pt) { return _workingData.get()[pt.y*dim.width + pt.x]; }
    inline void setWNorm(const Pt2i &pt, normal_t &val) { _workingData.get()[pt.y*dim.width + pt.x] = val; }

    inline float getDepth(const Pt2i &pt) { return _depth.get()[pt.y*dim.width + pt.x]; }
    inline void setDepth(const Pt2i &pt, float val) { _depth.get()[pt.y*dim.width + pt.x] = val; }

    inline float getWDepth(const Pt2i &pt) { return _workingDepth.get()[pt.y*dim.width + pt.x]; }
    inline void setWDepth(const Pt2i &pt, float val) { _workingDepth.get()[pt.y*dim.width + pt.x] = val; }

    inline char getMask(const Pt2i &pt) { return _mask.get()[pt.y*dim.width + pt.x]; }
    inline void setMask(const Pt2i &pt, char val) { _mask.get()[pt.y*dim.width + pt.x] = val; }


    // Publicly accessible members
    Dim dim;

protected:

    // Formatting utility function
    inline uint32_t formatPixel(normal_t &pix) {
        uint32_t r, g, b;
        r = (pix.x + 1) * 127.5f;
        g = (pix.y + 1) * 127.5f;
        b = (pix.z * 255);
        return 0xFF000000 | r << 16 | g << 8 | b;
    }

    // Data members
    normal_p _data;
    depth_p _depth;
    normal_p _workingData;
    depth_p _workingDepth;
    mask_p _mask;


    // Flood fill structure
    struct _QLinearFill {
        struct FillRange {
            int startX;
            int endX;
            int y;

            FillRange(int sX, int eX, int sY)
                : startX(sX)
                , endX(eX)
                , y(sY) {}
        };

        _QLinearFill(NormDepthImage *nd, float thresh, float dThresh)
            : size(nd->dim)
            , image(nd)
            , tol(thresh)
            , dTol(dThresh)
            , checked(new char[size.area()]) {}

        ~_QLinearFill() {}

        void fill(Pt2i seed);
        void linFill(int x, int y);
        bool checkPx(int px, int cmp);

        Dim size;
        NormDepthImage *image;
        float tol, dTol;

        std::queue<FillRange> rangeQueue;
        std::unique_ptr<char> checked;

        float *pDepth;
        normal_t *pNorm;
        char *pMask;
    };
};

