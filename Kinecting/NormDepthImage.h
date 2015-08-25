#pragma once
#include "stdafx.h"
#include "Util.h"

// Convenience class for performing image operations
// Stores vec3 normals and floating-point depth
class NormDepthImage {
public:
    // Internal types
    typedef glm::vec3 normal_t;
    typedef glm::simdVec4 store_t;
    typedef std::unique_ptr<normal_t> normal_p;
    typedef std::unique_ptr<float> depth_p;
    typedef std::unique_ptr<uint32_t> fmt_p;
    typedef std::unique_ptr<char> mask_p;
    typedef std::unique_ptr<store_t, aligned_free> store_p;

    enum Mask {
        NONE = 0,
        PICKED = 1
    };

    // Default constructor just allocates image space
    NormDepthImage(Dim size)
        : _data(aligned_malloc<store_t>(size.area(), 16))
        , _workingData(aligned_malloc<store_t>(size.area(), 16))
        //, _depth(new float[size.area()])
        //, _workingDepth(new float[size.area()])
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
            // Don't write to .w (depth)
			auto writeable = glm::vec4_cast(d[i]);
            writeable.z = float(getB(data[i])) / 255; // correct to [0..1]

            writeable.x = float(getR(data[i])) / 127.5f;
            writeable.x -= 1.0f;

            writeable.y = float(getG(data[i])) / 127.5f;
            writeable.y -= 1.0f;
			d[i] = store_t(writeable);
        }
    }

    // Set depth on an already constructed image (in m)
    void setDepth(const float *data) {
        auto d = _data.get();
        for (int i = 0; i < dim.area(); i++) {
			auto writeable = glm::vec4_cast(d[i]);
            if (data[i] < 500.0f || data[i] > 4500.0f)  writeable.w = -100000.0f;
            else                                        writeable.w = data[i] / 1000.0f;
			d[i] = store_t(writeable);
        }
    }

    ~NormDepthImage() = default;

    // Raw data accessors
    auto getRawNormals() { return _data.get(); }
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
    inline int ptInd(const Pt2i &pt) { return pt.y*dim.width + pt.x; }
	inline char getMask(const Pt2i &pt) { return _mask.get()[ptInd(pt)]; }
	inline char getDepth(const Pt2i &pt) {
		return glm::vec4_cast(_data.get()[ptInd(pt)]).w;
	}

    // Publicly accessible members
    Dim dim;

protected:

    // Formatting utility function
    inline uint32_t formatPixel(store_t &pix) {
        uint32_t r, g, b;
		auto castPix = glm::vec4_cast(pix);
        r = (castPix.x + 1) * 127.5f;
        g = (castPix.y + 1) * 127.5f;
        b = (castPix.z * 255);
        return 0xFF000000 | r << 16 | g << 8 | b;
    }

    // Data members
    store_p _data;
    store_p _workingData;
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
            , tol(thresh, thresh, thresh, dThresh)
            , checked(new char[size.area()]) {}

        ~_QLinearFill() {}

        void fill(Pt2i seed);
        void linFill(int x, int y);
        bool checkPx(int px, int cmp);

        Dim size;
        NormDepthImage *image;
        store_t tol;

        std::queue<FillRange> rangeQueue;
        std::unique_ptr<char> checked;

        //float *pDepth;
        //normal_t *pNorm;
        char *pMask;

        store_t *pData;
    };
};

