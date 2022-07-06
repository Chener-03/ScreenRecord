//
// Created by chen on 2022/6/30.
//

#include "Encode.h"

namespace AV {


    Encode::Encode(AD::AudioCapture *adc, DX::Graphics *gra, char *filename, int video_bit_rate, int audio_bit_rate,
                   int fps, int video_encoder_type)
            : video_bit_rate_(video_bit_rate),
              audio_bit_rate_(audio_bit_rate), fps(fps), video_encoder_type(video_encoder_type),
              filename_(filename), audioCapture(adc), graphics(gra) {
        // ��ʼ������
        width = gra->getWidth();
        height = gra->getHeight();
        ysize = width * height;
        uvsize = ysize / 4;
        vsize = width * height * 4;
        asize = _nb_samples * 2 * 2;

        this->initVideoContext();
        this->initAudioContext();
        videoFramePtr = malloc(vsize);
        audioFramePtr = malloc(asize);
        videoFrameYUVPtr = malloc(width * height * 3 / 2);
        if (!audioCapture->isRunning() || !graphics->isRunning())
            ERROROUT("��Ƶ�ɼ� | ��Ƶ�ɼ� δȫ������");
    }

    void Encode::initVideoContext() {
        avformat_alloc_output_context2(&ic, 0, 0, filename_);
        AVCodec *codec_av = nullptr;

        if (this->video_encoder_type == 0) {
            codec_av = avcodec_find_encoder_by_name("h264_nvenc");
            if (!codec_av) {
                ERROROUT("��֧�ֱ�������h264_nvenc,Ĭ��ʹ�������");
                this->video_encoder_type = 3;
            }
        }

        if (this->video_encoder_type == 1) {
            codec_av = avcodec_find_encoder_by_name("h264_amf");
            if (!codec_av) {
                ERROROUT("��֧�ֱ�������h264_amf,Ĭ��ʹ�������");
                this->video_encoder_type = 3;
            }
        }

        if (this->video_encoder_type == 2) {
            codec_av = avcodec_find_encoder_by_name("h264_qsv");
            if (!codec_av) {
                ERROROUT("��֧�ֱ�������h264_qsv,Ĭ��ʹ�������");
                this->video_encoder_type = 3;
            }
        }

        if (this->video_encoder_type == 3) {
            codec_av = avcodec_find_encoder(AV_CODEC_ID_H264);
        }

        vc = avcodec_alloc_context3(codec_av);
        vc->bit_rate = video_bit_rate_;
        vc->width = width;
        vc->height = height;
        //ʱ�����
        vc->time_base = {1, fps};
        vc->framerate = {fps, 1};
        //�������С
        vc->gop_size = 50;
        //B֡����Ϊ0
        vc->max_b_frames = 0;
        vc->pix_fmt = AV_PIX_FMT_YUV420P;
        vc->codec_id = AV_CODEC_ID_H264;
        av_opt_set(vc->priv_data, "preset", "default", 0);
        vc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

        avcodec_open2(vc, codec_av, NULL);
        //�����Ƶ�������������
        vs = avformat_new_stream(ic, NULL);
        vs->codecpar->codec_tag = 0;//���ڲ���������������Ϣ
        avcodec_parameters_from_context(vs->codecpar, vc);

        yuv = av_frame_alloc();
        yuv->format = AV_PIX_FMT_YUV420P;
        yuv->width = width;
        yuv->height = height;
        yuv->pts = 0;
        av_frame_get_buffer(yuv, 32);

    }

    void Encode::initAudioContext() {
        AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
        ac = avcodec_alloc_context3(codec);
        ac->bit_rate = audio_bit_rate_;
        ac->sample_rate = 48000;
        ac->sample_fmt = AV_SAMPLE_FMT_FLTP;
        ac->channels = 2;
        ac->channel_layout = AV_CH_LAYOUT_STEREO;
        ac->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        avcodec_open2(ac, codec, NULL);
        as = avformat_new_stream(ic, NULL);
        as->codecpar->codec_tag = 0;
        avcodec_parameters_from_context(as->codecpar, ac);

        av_dump_format(ic, 0, filename_, 1);

        //4 ��Ƶ�ز��������Ĵ���
        asc = swr_alloc_set_opts(asc, ac->channel_layout, ac->sample_fmt, ac->sample_rate,//�����ʽ
                                 av_get_default_channel_layout(2), (AVSampleFormat) AV_SAMPLE_FMT_S16, 48000,//�������
                                 0, 0);

        swr_init(asc);

        pcm = av_frame_alloc();
        pcm->format = ac->sample_fmt;
        pcm->channels = ac->channels;
        pcm->channel_layout = ac->channel_layout;
        pcm->nb_samples = _nb_samples;
        av_frame_get_buffer(pcm, 0);
    }

    void Encode::start() {
        this->isPause = false;
        this->isBegin = true;
        avio_open(&ic->pb, filename_, AVIO_FLAG_WRITE);
        avformat_write_header(ic, NULL);
        std::thread t1(&Encode::run, this);
        t1.detach();
    }

    void Encode::stop() {
        this->isPause = false;
        this->isBegin = false;
        QThread::msleep(100);
        //д��β����Ϣ����
        av_write_trailer(ic);
        avio_close(ic->pb);
        QThread::msleep(100);
    }

    void Encode::pause() {
        isPause = !isPause;
    }

    void Encode::run() {
        unsigned long long time1 = TIME::getTimestampMs();
        int spttime = 1000 / fps;
        while (isBegin) {
            if (isPause) {
                QThread::msleep(100);
                continue;
            }

            if (IsVideoBefore()) {
                graphics->getFrame(videoFramePtr);
                AV::BGRA2YUV420(videoFramePtr, videoFrameYUVPtr, width, height);
                EncodeVideo(videoFrameYUVPtr);

/*                // ʱ��ͬ��
                unsigned long long time2 = TIME::getTimestampMs();
                if (time2 - time1 < spttime)
                {
                    QThread::msleep(spttime-(time2-time1));
                }
                time1 = time2;*/
            } else {
                audioCapture->getFrame(audioFramePtr, asize);
                EncodeAudio(audioFramePtr);
            }

        }
    }

    bool Encode::IsVideoBefore() {
        //����Ƶʱ������бȽ�
        if (!ic || !as || !vs) return false;
        int ret = av_compare_ts(vpts, vc->time_base, apts, ac->time_base);
        if (ret <= 0) {
            return true;
        }
        return false;
    }

    void Encode::EncodeVideo(void *yuv420) {
        thread_local std::vector<uint8_t> yuv_data;
        if (yuv_data.size() != ysize * 3 / 2) {
            yuv_data.resize(ysize * 3 / 2);
        }
        memcpy(yuv_data.data(), yuv420, width * height * 3 / 2);

        uint8_t *yuv_dataptr = yuv_data.data();
        //����YUV���ݵ�֡������֡���ݴ����ڴ���룬�������п���
        for (int i = 0; i < height; i++) {
            memcpy(yuv->data[0] + i * yuv->linesize[0], yuv_dataptr + width * i, width);
        }
        const int uv_stride = width / 2;
        for (int i = 0; i < height / 2; i++) {
            memcpy(yuv->data[1] + i * yuv->linesize[1], yuv_dataptr + ysize + uv_stride * i, uv_stride);
            memcpy(yuv->data[2] + i * yuv->linesize[2], yuv_dataptr + ysize + uvsize + uv_stride * i, uv_stride);
        }

        yuv->pts = vpts++;

        avcodec_send_frame(vc, yuv);
        AVPacket *p = av_packet_alloc();
        av_init_packet(p);
        int ret = avcodec_receive_packet(vc, p);
        if (ret != 0 || p->size <= 0) {
            av_packet_free(&p);
            return;
        }
        av_packet_rescale_ts(p, vc->time_base, vs->time_base);
        p->stream_index = vs->index;//��Ƶ��������
        if (!ic || !p || p->size <= 0) return;
        //av_write_frame
        if (av_interleaved_write_frame(ic, p) != 0) return;
    }

    void Encode::EncodeAudio(void *d) {
        const uint8_t *data[AV_NUM_DATA_POINTERS] = {0};
        data[0] = (uint8_t *) d;

        //1 ��Ƶ�ز���
        int len = swr_convert(asc, pcm->data, pcm->nb_samples,
                              data, pcm->nb_samples);

        //2 ��Ƶ�ı���
        int ret = avcodec_send_frame(ac, pcm);
        if (ret != 0) {
            return;
        }

        AVPacket *pkt = av_packet_alloc();
        av_init_packet(pkt);
        ret = avcodec_receive_packet(ac, pkt);
        if (ret != 0) {
            av_packet_free(&pkt);
            return;
        }

        pkt->stream_index = as->index;//��Ƶ��������

        pkt->pts = apts;
        pkt->dts = pkt->pts;
        apts += av_rescale_q(pcm->nb_samples, {1, ac->sample_rate}, ac->time_base);

        if (!ic || !pkt || pkt->size <= 0) return;
        if (av_interleaved_write_frame(ic, pkt) != 0) return;
    }
} // AV