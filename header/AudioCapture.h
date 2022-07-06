//
// Created by chen on 2022/7/1.
//

#ifndef UNTITLED_AUDIOCAPTURE_H
#define UNTITLED_AUDIOCAPTURE_H

#include <QObject>

#include <mmdeviceapi.h>
#include <process.h>
#include <setupapi.h>
#include <initguid.h>
#include <functiondiscoverykeys_devpkey.h>
#include <QEventLoop>
#include <audioclient.h>
#include <qdebug.h>
#include <unistd.h>
#include <QFile>
#include "QBlockingQueue.hpp"

namespace AD{


#define NS_PER_SEC 1000000000
#define REFTIMES_PER_SEC  NS_PER_SEC/100

    struct AudioDevice{
        std::string name;
        _EDataFlow type;
        LPWSTR id;
        void log() const
        {
            printf("name:%s  id:%p  type:%d\n",name.c_str(),id,type);
        }
        std::string toString()
        {
            char* ls = (char*)malloc(100);
            sprintf(ls,"name:%s  id:%p  type:%d",name.c_str(),id,type);
            std::string ret = std::string(ls);
            free(ls);
            return ret;
        }
    };

    class AudioCapture : public QObject{
        Q_OBJECT
    public:
        AudioCapture();
        bool start(int i = 0);
        void getFrame(void* ptr,int size);
        void stop();
        std::vector<AudioDevice> showDevices() const;
        bool isRunning() const;

    private:

        std::vector<AudioDevice> audioDevices;
        AudioDevice currentDevice;
        IMMDeviceEnumerator *pOutputEnumerator;
        bool getDevices();
        std::string getDeviceName(LPCWSTR id);
        volatile bool isRun = false;

        IMMDevice * eRenderDevice;
        IAudioClient * eRenderClient;
        UINT32 eRenderBufferFrameCount;
        UINT32 packetLength;
        HANDLE eRenderEvent;
        IAudioCaptureClient* eRenderIAudioCaptureClient;
        void start0();
        void runMuteThread();

        QBlockingQueue<BYTE>* qbq;
        int cacheSize = 19200;

    signals:
        void notifyData(QByteArray& bt);

    };
}



#endif //UNTITLED_AUDIOCAPTURE_H
