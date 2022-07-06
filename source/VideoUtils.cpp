//
// Created by chen on 2022/7/4.
//
#include "VideoUtils.h"

#include <iostream>
#include "libyuv/convert.h"

namespace AV{
    void BGRA2BGR(void* bgra,void* bgr,int w,int h)
    {
        for (int j = 0, k =0; j < w*h*4; j+=4) {
            memcpy((void*)((uint64_t)bgr+k),(void*)((uint64_t)bgra+j),3);
            k+=3;
        }
    }

    int BGRA2YUV420(void* src_frame, void* dst_frame, int width, int height)
    {
        uint8_t* yplane= (uint8_t*)dst_frame;
        uint8_t* uplane= (uint8_t*)dst_frame	+ width	* height;
        uint8_t* vplane= uplane+	(width	* height/4);
        int n = libyuv::ARGBToI420((uint8_t*)src_frame,width*4,yplane, width,uplane, width/2,vplane, width/2, width, height);
        return n;
    }
}