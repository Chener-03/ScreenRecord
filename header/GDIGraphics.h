//
// Created by chen on 2022/7/1.
//

#ifndef UNTITLED_GDIGRAPHICS_H
#define UNTITLED_GDIGRAPHICS_H

#include "Graphics.h"
#include <mutex>
#include <iostream>

namespace DX{
    class GDIGraphics : public Graphics{

    public:
        bool init() override;

        bool getFrame(void *pImageDate ) override;

        void destroy() override;

    private:
        int fps = 60;
        std::mutex frameLock;
        void* framePtr = nullptr;
        volatile bool isRun = false;
        uint32_t copySize = 0;
        void start();
        void stop();
        void frame();
        bool getFrame0(void *pImageDate, uint32_t& size);

    };
}




#endif //UNTITLED_GDIGRAPHICS_H
