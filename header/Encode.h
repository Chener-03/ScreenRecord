//
// Created by chen on 2022/6/30.
//

#ifndef UNTITLED_ENCODE_H
#define UNTITLED_ENCODE_H

#ifdef	__cplusplus
extern "C"
{
#endif
#include <libavcodec\avcodec.h>
#include <libavformat\avformat.h>
#include <libswscale\swscale.h>
#include <libavdevice\avdevice.h>
#include <libavutil\fifo.h>
#include <libavutil\imgutils.h>
#include <libswresample\swresample.h>
#ifdef __cplusplus
};
#endif
#include "Logger.h"
#include <vector>
#include <memory>
#include <tuple>
#include <array>
#include <QThread>
#include "AudioCapture.h"
#include "DXGIGraphics.h"
#include "timeutils.h"
#include "VideoUtils.h"

namespace AV {

    class Encode {

    public:
        explicit Encode(AD::AudioCapture* adc,DX::Graphics* gra,char* filename,
                        int video_bit_rate=4000000,
                        int audio_bit_rate=640000,
                        int fps=60,
                        int video_encoder_type=0);

        void start();
        void stop();
        void pause();
        bool isStart() const{
            return isBegin;
        }

    private:
        AD::AudioCapture* audioCapture;
        DX::Graphics* graphics;
        AVFormatContext *ic = NULL;//mp4输出上下文
        AVCodecContext *vc = NULL;//视频编码器上下文
        AVCodecContext *ac = NULL;//音频编码器上下文
        AVStream* vs = NULL;//视频流
        AVStream* as = NULL;//音频流
        SwrContext* asc = NULL;//音频重采样上下文
        AVFrame* yuv = NULL;//输出yuv
        AVFrame* pcm = NULL;//重采样后输出的pcm
        int vpts = 0;//视频的pts;
        int apts = 0;//音频的pts;
        int _nb_samples = 1024;
        char* filename_;
        int width = 0;
        int height = 0;
        int ysize = 0;  //width * height
        int uvsize = 0; //ysize/4
        int video_bit_rate_=0;
        int audio_bit_rate_=0;
        int fps=0;
        int video_encoder_type=0; //0 NVIDIA    1 AMD   2 INTEL     3软编码
        volatile bool isBegin = false;
        volatile bool isPause = false;

    private:
        void initVideoContext();
        void initAudioContext();
        bool IsVideoBefore();
        void EncodeVideo(void* yuv420);
        void EncodeAudio(void* d);
        void run();
        int vsize = 0; //width*height*4
        int asize = 0; //_nb_samples*2*2
        void* videoFramePtr = nullptr;  //视频BGRA帧数据
        void* videoFrameYUVPtr = nullptr;  //视频YUV帧数据
        void* audioFramePtr = nullptr;  //音频帧数据

    private:
        Encode(){};
    };

} // AV

#endif //UNTITLED_ENCODE_H
