//
// Created by chen on 2022/6/30.
//

#ifndef UNTITLED_DXGIGRAPHICS_H
#define UNTITLED_DXGIGRAPHICS_H

#include "Graphics.h"
#include "windows.h"
#include <DXGI.h>
#include <D3D11.h>
#include <dxgi1_2.h>
#include <thread>
#include "Logger.h"

namespace DX {

    class DXGIGraphics : public Graphics{
    public:
        bool init() override;
        inline uint32_t getBufferSize() const;
        bool getFrame(void *pImageDate ) override;
        void destroy() override;
        bool isRunning() override;

    private:
        int fps = 100;
        ID3D11Device           *m_hDevice = nullptr;
        ID3D11DeviceContext    *m_hContext = nullptr;
        IDXGIOutputDuplication *m_hDeskDupl = nullptr;
        std::mutex frameLock;
        void* framePtr = nullptr;
        volatile bool isRun = false;
        uint32_t copySize = 0;
        void start();
        void stop();
        void frame();
        bool getFrame0(void *pImageDate );
        void reloadIDXGIOutputDuplication();
    };

} // DX

#endif //UNTITLED_DXGIGRAPHICS_H
