#include "stdafx.h"
#include "KinectDevice.h"


void KinectDevice::connect(int streams) {
    if (_streams == Streams::NO_STREAM) {
        _streams = streams;
        HRESULT hr = GetDefaultKinectSensor(&_sensor);

        if (hr != S_OK) throw std::exception("Cannot find Kinect Sensor");
        else {
            hr = _sensor->Open();
        }

        if (hr != S_OK) throw std::exception("Cannot open connection");
        else {
            DWORD srcTypes = 0;
            if (_streams & Streams::DEPTH_STREAM)  srcTypes |= FrameSourceTypes_Depth;
            if (_streams & Streams::COLOR_STREAM)  srcTypes |= FrameSourceTypes_Color;
            if (_streams & Streams::IR_STREAM)     srcTypes |= FrameSourceTypes_Infrared;
            if (_streams & Streams::HDIR_STREAM)   srcTypes |= FrameSourceTypes_LongExposureInfrared;
            if (_streams & Streams::INDEX_STREAM)  srcTypes |= FrameSourceTypes_BodyIndex;

            hr = _sensor->OpenMultiSourceFrameReader(srcTypes, &_reader);
        }

        if (hr != S_OK) throw std::exception("Cannot open frame reader");
        else {
            hr = _reader->SubscribeMultiSourceFrameArrived(&_frameEvent);
        }

        if (hr != S_OK) throw std::exception("Cannot subscribe frame event listener");

        getFrameInformation();
    }
    else if (_sensor && _reader) {
        // Change the streams
        if (_listenThread.joinable()) {
            _listening = false;
            _listenThread.join();
        }

        _reader->UnsubscribeMultiSourceFrameArrived(_frameEvent);
        _reader->Release();

        _streams = streams;
        DWORD srcTypes = 0;
        if (_streams & Streams::DEPTH_STREAM)  srcTypes |= FrameSourceTypes_Depth;
        if (_streams & Streams::COLOR_STREAM)  srcTypes |= FrameSourceTypes_Color;
        if (_streams & Streams::IR_STREAM)     srcTypes |= FrameSourceTypes_Infrared;
        if (_streams & Streams::HDIR_STREAM)   srcTypes |= FrameSourceTypes_LongExposureInfrared;
        if (_streams & Streams::INDEX_STREAM)  srcTypes |= FrameSourceTypes_BodyIndex;

        HRESULT hr = _sensor->OpenMultiSourceFrameReader(srcTypes, &_reader);

        if (hr != S_OK) throw std::exception("Cannot open frame reader");
        else {
            hr = _reader->SubscribeMultiSourceFrameArrived(&_frameEvent);
        }

        if (hr != S_OK) throw std::exception("Cannot subscribe frame event listener");
    }
    else throw std::exception("Attempted to connect to device after initialization failure");
}

void KinectDevice::asyncListen() {
    _listenThread = std::thread(&KinectDevice::listen, this);
}

void KinectDevice::listen() {
    if (_listening) throw std::exception("Already listening for new frames");

    _listening = true;
    while (_listening) {
        int idx = WaitForSingleObject((HANDLE) _frameEvent, 100);
        switch (idx) {
        case WAIT_TIMEOUT:
            std::cout << ".";
            continue;
        case WAIT_OBJECT_0:
            IMultiSourceFrameArrivedEventArgs *frameArgs = nullptr;
            IMultiSourceFrameReference *frameRef = nullptr;
            HRESULT hr = _reader->GetMultiSourceFrameArrivedEventData(_frameEvent, &frameArgs);

            if (hr == S_OK) {
                hr = frameArgs->get_FrameReference(&frameRef);
                frameArgs->Release();
            }

            if (hr == S_OK) {
                //if (_lastFrame) _lastFrame->Release();
                hr = frameRef->AcquireFrame(&_lastFrame);
                frameRef->Release();
            }

            if (hr == S_OK) {
                // Store frame data
                //std::cout << "Got a frame YEAH" << std::endl;

                IDepthFrameReference                *depthRef   = nullptr;
                IColorFrameReference                *colorRef   = nullptr;
                IInfraredFrameReference             *irRef      = nullptr;
                ILongExposureInfraredFrameReference *hdirRef    = nullptr;
                IBodyIndexFrameReference            *indexRef   = nullptr;

                IDepthFrame                         *depth      = nullptr;
                IColorFrame                         *color      = nullptr;
                IInfraredFrame                      *ir         = nullptr;
                ILongExposureInfraredFrame          *hdir       = nullptr;
                IBodyIndexFrame                     *index      = nullptr;

                size_t size;
                uint16_t *buff;
                BYTE *cbuff;

                frameLock.lock();
                if (_streams & Streams::DEPTH_STREAM) {
                    _lastFrame->get_DepthFrameReference(&depthRef);
                    depthRef->AcquireFrame(&depth);
                    
                    if (depth) {
                        depthSwap();
                        depth->AccessUnderlyingBuffer(&size, &buff);
                        memcpy(depthData.get(), buff, size * sizeof(uint16_t));
                        depth->Release();
                    }
                    
                    depthRef->Release();
                }
                if (_streams & Streams::COLOR_STREAM) {
                    _lastFrame->get_ColorFrameReference(&colorRef);
                    colorRef->AcquireFrame(&color);
                    //color->AccessUnderlyingBuffer(&size, &buff);
                    //memcpy(_colorData.get(), buff, size);
                    color->Release();
                    colorRef->Release();
                }
                if (_streams & Streams::IR_STREAM) {
                    _lastFrame->get_InfraredFrameReference(&irRef);
                    irRef->AcquireFrame(&ir);
                    ir->AccessUnderlyingBuffer(&size, &buff);
                    memcpy(irData.get(), buff, size);
                    ir->Release();
                    irRef->Release();
                }
                if (_streams & Streams::HDIR_STREAM) {
                    _lastFrame->get_LongExposureInfraredFrameReference(&hdirRef);
                    hdirRef->AcquireFrame(&hdir);
                    hdir->AccessUnderlyingBuffer(&size, &buff);
                    memcpy(hdirData.get(), buff, size);
                    hdir->Release();
                    hdirRef->Release();
                }
                if (_streams & Streams::INDEX_STREAM) {
                    _lastFrame->get_BodyIndexFrameReference(&indexRef); 
                    indexRef->AcquireFrame(&index);
                    index->AccessUnderlyingBuffer(&size, &cbuff);
                    memcpy(indexData.get(), cbuff, size);
                    index->Release();
                    indexRef->Release();
                }

                frameLock.unlock();

                _lastFrame->Release();
            }
        }
    }
}

void KinectDevice::disconnect() {
    if (_listenThread.joinable()) {
        _listening = false;
        _listenThread.join();
    }

    if (_reader) {
        _reader->Release();
        _reader = nullptr;
    }

    if (_sensor) {
        _sensor->Close();
        _sensor = nullptr;
    }
}

void KinectDevice::getFrameInformation() {
    // Get the frame information
    IDepthFrameSource *depthSrc;
    IColorFrameSource *colorSrc;
    IInfraredFrameSource *irSrc;
    ILongExposureInfraredFrameSource *hdirSrc;
    IBodyIndexFrameSource *indexSrc;

    IFrameDescription *depthDesc, *colorDesc, *irDesc, *hdirDesc, *indexDesc;

    if (_streams & Streams::DEPTH_STREAM) {
        _sensor->get_DepthFrameSource(&depthSrc);
        depthSrc->get_FrameDescription(&depthDesc);
        depthFrameInfo = FrameInfo(depthDesc);

        // Min/max vals
        depthSrc->get_DepthMinReliableDistance(&depthFrameInfo.minVal);
        depthSrc->get_DepthMaxReliableDistance(&depthFrameInfo.maxVal);

        // Allocate
        depthData =         std::shared_ptr<uint16_t>(new uint16_t[depthFrameInfo.frameSize]);
        prevDepthData =     std::shared_ptr<uint16_t>(new uint16_t[depthFrameInfo.frameSize]);
    }
    else {
        depthData = nullptr;
        prevDepthData = nullptr;
    }

    if (_streams & Streams::COLOR_STREAM) {
        _sensor->get_ColorFrameSource(&colorSrc);
        colorSrc->get_FrameDescription(&colorDesc);
        colorFrameInfo = FrameInfo(colorDesc);

        colorData = std::shared_ptr<uint16_t>(new uint16_t[colorFrameInfo.frameSize]);
    }
    if (_streams & Streams::IR_STREAM) {
        _sensor->get_InfraredFrameSource(&irSrc);
        irSrc->get_FrameDescription(&irDesc);
        irFrameInfo = FrameInfo(irDesc);

        irData = std::shared_ptr<uint16_t>(new uint16_t[irFrameInfo.frameSize]);
    }
    if (_streams & Streams::HDIR_STREAM) {
        _sensor->get_LongExposureInfraredFrameSource(&hdirSrc);
        hdirSrc->get_FrameDescription(&hdirDesc);
        hdirFrameInfo = FrameInfo(hdirDesc);

        hdirData = std::shared_ptr<uint16_t>(new uint16_t[hdirFrameInfo.frameSize]);
    }
    if (_streams & Streams::INDEX_STREAM) {
        _sensor->get_BodyIndexFrameSource(&indexSrc);
        indexSrc->get_FrameDescription(&indexDesc);
        indexFrameInfo = FrameInfo(indexDesc);

        indexData = std::shared_ptr<BYTE>(new BYTE[indexFrameInfo.frameSize]);
    }
}
