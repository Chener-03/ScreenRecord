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
        AVFormatContext *ic = NULL;//mp4���������
        AVCodecContext *vc = NULL;//��Ƶ������������
        AVCodecContext *ac = NULL;//��Ƶ������������
        AVStream* vs = NULL;//��Ƶ��
        AVStream* as = NULL;//��Ƶ��
        SwrContext* asc = NULL;//��Ƶ�ز���������
        AVFrame* yuv = NULL;//���yuv
        AVFrame* pcm = NULL;//�ز����������pcm
        int vpts = 0;//��Ƶ��pts;
        int apts = 0;//��Ƶ��pts;
        int _nb_samples = 1024;
        char* filename_;
        int width = 0;
        int height = 0;
        int ysize = 0;  //width * height
        int uvsize = 0; //ysize/4
        int video_bit_rate_=0;
        int audio_bit_rate_=0;
        int fps=0;
        int video_encoder_type=0; //0 NVIDIA    1 AMD   2 INTEL     3�����
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
        void* videoFramePtr = nullptr;  //��ƵBGRA֡����
        void* videoFrameYUVPtr = nullptr;  //��ƵYUV֡����
        void* audioFramePtr = nullptr;  //��Ƶ֡����

    private:
        Encode(){};
    };

} // AV

#endif //UNTITLED_ENCODE_H
