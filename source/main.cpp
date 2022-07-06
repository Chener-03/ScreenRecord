#include <QApplication>

#include <iostream>
#include "Logger.h"
#include <sys/time.h>
#include "Encode.h"
#include <initguid.h>
#include <audioclient.h>
#include "AudioCapture.h"

#include "CLI11.hpp"
AV::Encode* g_en = nullptr;

bool ctrlhandler(DWORD fdwctrltype)
{
    switch (fdwctrltype)
    {
        case CTRL_C_EVENT:
            g_en->stop();
            return(true);
        default:
            return false;
    }
}


int argHandle(int arg1, char** arg2)
{
    CLI::App app{"Version:0.0.1"};
    app.footer("https://github.com/czp541308303/ScreenRecord");
    app.set_help_flag("-h,--help","��ʾ����");
    app.get_formatter()->column_width(60);
    app.require_subcommand(1);

    auto sub1 = app.add_subcommand("video", "¼��");
    auto sub2 = app.add_subcommand("audioDevice", "ö����Ƶ�豸");
    auto sub3 = app.add_subcommand("captureDevice", "ö�ٿ�¼����Ļ");
    auto sub4 = app.add_subcommand("encodeType", "���뷽ʽ");

    sub1->fallthrough();
    sub2->fallthrough();
    sub3->fallthrough();

    std::string filepath;
    std::string filename;
    int fps;
    int video_bit_rate;
    int audio_bit_rate;
    int video_encoder_type;
    int audio_dev;

    time_t nowtime;
    struct tm* p;
    time(&nowtime);
    p = localtime(&nowtime);
    std::stringstream ss;
    ss << std::put_time(p,"%Y-%m-%d-%H-%M-%S");
    std::string fis = ss.str();

    sub1->add_option("-f,--file",filepath,"�ļ�·��")->check(CLI::ExistingPath)->required();
    sub1->add_option("-n,--name",filename,"�ļ���")->default_val(fis);
    sub1->add_option("-p",fps,"֡��")->default_val(60);
    sub1->add_option("-v,--video_bit_rate",video_bit_rate,"��Ƶ������")->default_val(4000000);
    sub1->add_option("-a,--audio_bit_rate",audio_bit_rate,"��Ƶ������")->default_val(640000);
    sub1->add_option("-e,--video_encoder_type",video_encoder_type,"���뷽ʽ")->default_val(0);
    sub1->add_option("-d,--audio_dev",audio_dev,"��Ƶ�����豸")->default_val(0);


    CLI11_PARSE(app, arg1, arg2);

    if (sub1->parsed())
    {
        AD::AudioCapture c;
        c.start(audio_dev);
        DX::DXGIGraphics g;
        g.init();
        std::string f = filepath+filename+".mp4";

        AV::Encode en(&c,&g,(char*)f.c_str(),video_bit_rate,audio_bit_rate,fps,video_encoder_type);
        g_en=&en;
        SetConsoleCtrlHandler((PHANDLER_ROUTINE)ctrlhandler, true);
        en.start();
        std::cout << "�� CTRL+C ���� " << std::endl;
        while (g_en->isStart())
        {
            Sleep(1000);
        }

    }
    else if (sub2->parsed())
    {
        AD::AudioCapture c;
        auto device = c.showDevices();
        for(auto dev : device)
        {
            static int i = 0;
            std::cout << "���:" << i << "\t" << dev.toString() << std::endl;
            i++;
        }
    }else if (sub3->parsed())
    {

    }else if (sub4->parsed())
    {
        std::cout << "0:h264_nvenc" << std::endl;
        std::cout << "1:h264_amf" << std::endl;
        std::cout << "2:h264_qsv" << std::endl;
        std::cout << "3:�����" << std::endl;
    }
}

int main(int argc, char** argv)
{
    argHandle(argc,argv);
}