//
// Created by chen on 2022/7/1.
//

#ifndef UNTITLED_VIDEOUTILS_H
#define UNTITLED_VIDEOUTILS_H


namespace AV{
    void BGRA2BGR(void* bgra,void* bgr,int w,int h);

    int BGRA2YUV420(void* src_frame, void* dst_frame, int width, int height);
}

#endif //UNTITLED_VIDEOUTILS_H
