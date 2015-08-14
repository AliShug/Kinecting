#pragma once
#include "stdafx.h"


struct FrameInfo {
    size_t bpp = 0, pixels = 0, frameSize = 0;
    int w = 0, h = 0;
    float xFov = 0.0f, yFov = 0.0f;
    uint16_t minVal, maxVal;

    FrameInfo() {}

    FrameInfo(IFrameDescription *desc) {
        desc->get_BytesPerPixel(&bpp);
        desc->get_LengthInPixels(&pixels);
        frameSize = bpp*pixels;
        desc->get_HorizontalFieldOfView(&xFov);
        desc->get_VerticalFieldOfView(&yFov);
        desc->get_Width(&w);
        desc->get_Height(&h);
    }
};

class KinectDevice {
public:
    enum Streams {
        NO_STREAM    = 0,
        DEPTH_STREAM = 1,
        COLOR_STREAM = 1 << 1,
        IR_STREAM    = 1 << 2,
        HDIR_STREAM  = 1 << 3,
        INDEX_STREAM = 1 << 4
    };


    KinectDevice() = default;
    ~KinectDevice() { disconnect(); }

    void setStreams(int streams) { _streams = streams; }

    void connect(int streams);
    void asyncListen();
    void listen();

    void disconnect();


    std::mutex frameLock;
    
    std::shared_ptr<uint16_t>   depthData;
    std::shared_ptr<uint16_t>   prevDepthData;
    std::shared_ptr<uint16_t>   colorData;
    std::shared_ptr<uint16_t>   irData;
    std::shared_ptr<uint16_t>   hdirData;
    std::shared_ptr<BYTE>       indexData;

    // Frame information
    FrameInfo depthFrameInfo, colorFrameInfo, irFrameInfo, hdirFrameInfo, indexFrameInfo;

protected:
    //std::shared_ptr<uint16_t> _depthBuffers[2] = { nullptr, nullptr };
    //int _depthBufferIndex = 0;
    void depthSwap() {
        if (depthData && prevDepthData) {
            //std::lock_guard<std::mutex> lck(frameLock);

            // Toggle
            //prevDepthData = _depthBuffers[_depthBufferIndex];
            //_depthBufferIndex ^= 1;
            //depthData = _depthBuffers[_depthBufferIndex];
            std::swap(depthData, prevDepthData);
        }
    }

    IKinectSensor *_sensor = nullptr;
    IMultiSourceFrameReader *_reader = nullptr;
    IMultiSourceFrame *_lastFrame = nullptr;

    int _streams = Streams::NO_STREAM;
    WAITABLE_HANDLE _frameEvent;
    std::atomic<bool> _listening = false;
    std::thread _listenThread;
    void getFrameInformation();
};

