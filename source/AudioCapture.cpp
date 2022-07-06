//
// Created by chen on 2022/7/1.
//

#include <iostream>
#include <thread>
#include "AudioCapture.h"
namespace AD{


    AudioCapture::AudioCapture() {
        qbq = new QBlockingQueue<BYTE>(cacheSize);
        CoInitialize(nullptr);
        const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
        const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
        HRESULT hr = CoCreateInstance(
                CLSID_MMDeviceEnumerator, NULL,
                CLSCTX_ALL, IID_IMMDeviceEnumerator,
                (void**)&pOutputEnumerator);

        getDevices();

    }

    bool AudioCapture::getDevices() {
        currentDevice = {"null",EDataFlow_enum_count,0};
        IMMDevice * pDevice;
        audioDevices.clear();
        IMMDeviceCollection *pCollection = NULL;
        pOutputEnumerator->EnumAudioEndpoints(eRender ,DEVICE_STATE_ACTIVE, &pCollection);
        UINT count;
        LPWSTR pwszID = NULL;
        pCollection->GetCount(&count);
        for (UINT i = 0; i < count; i++) {
            pCollection->Item(i, &pDevice);
            pDevice->GetId(&pwszID);
            audioDevices.push_back({getDeviceName(pwszID), eRender, pwszID});
        }

        pOutputEnumerator->EnumAudioEndpoints(eCapture ,DEVICE_STATE_ACTIVE, &pCollection);
        UINT count1;
        LPWSTR pwszID1 = NULL;
        pCollection->GetCount(&count1);
        for (UINT i = 0; i < count1; i++) {
            pCollection->Item(i, &pDevice);
            pDevice->GetId(&pwszID1);
            audioDevices.push_back({getDeviceName(pwszID1), eCapture, pwszID1});
        }


    }


    std::string AudioCapture::getDeviceName(LPCWSTR id) {
        IPropertyStore* pProps = NULL;
        PROPVARIANT varName;
        IMMDevice *tmp;
        pOutputEnumerator->GetDevice(id,&tmp);
        if(tmp!=nullptr){
            pOutputEnumerator->GetDevice(id,&tmp);
        }

        tmp->OpenPropertyStore(STGM_READ, &pProps) ;
        PropVariantInit(&varName);
        pProps->GetValue(PKEY_Device_FriendlyName, &varName) ;
        QString name=QString().fromUtf16(reinterpret_cast<const char16_t *>(varName.pwszVal));
        PropVariantClear(&varName);
        return std::string(name.toLocal8Bit().toStdString().c_str());;
    }

    bool AudioCapture::start(int i) {
        currentDevice = audioDevices.at(i);
        if (currentDevice.type==0)
        {
            isRun = true;
            std::thread t0(&AudioCapture::runMuteThread,this);
            t0.detach();

            std::thread t(&AudioCapture::start0,this);
            t.detach();

            return true;
        }else if(currentDevice.type==1)
        {
            isRun = true;

            std::thread t(&AudioCapture::start0,this);
            t.detach();
            return true;
        }
        return false;

    }

    void AudioCapture::start0() {

        pOutputEnumerator->GetDevice(currentDevice.id,&eRenderDevice);
        eRenderDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&eRenderClient);

        WAVEFORMATEX        *pwfx;
        eRenderClient->GetMixFormat(&pwfx);
        pwfx->wFormatTag = WAVE_FORMAT_PCM;
        pwfx->wBitsPerSample = 16;
        pwfx->nChannels = 2;
        pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample / 8;
        pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;
        pwfx->cbSize=0;

        DWORD flags = AUDCLNT_STREAMFLAGS_EVENTCALLBACK ;

        if (currentDevice.type==0)
        {
            flags |= AUDCLNT_STREAMFLAGS_LOOPBACK;
        } else if (currentDevice.type == 1)
        {
            flags |= AUDCLNT_STREAMFLAGS_NOPERSIST;
        }

        eRenderClient->Initialize(AUDCLNT_SHAREMODE_SHARED,flags,REFTIMES_PER_SEC,0,pwfx,NULL);

        eRenderClient->GetBufferSize(&eRenderBufferFrameCount);

        eRenderClient->GetService(IID_IAudioCaptureClient, (void**)&eRenderIAudioCaptureClient);
        eRenderEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        eRenderClient->SetEventHandle(eRenderEvent);
        eRenderClient->Start();

        QByteArray data;

        while (isRun)
        {
            WaitForSingleObject(eRenderEvent,INFINITY);
            eRenderIAudioCaptureClient->GetNextPacketSize(&packetLength);
            BYTE* pData;
            UINT32 numFramesAvailable;
            DWORD flags;
            if(packetLength!=0)
            {
                eRenderIAudioCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL);
                int dataSize = numFramesAvailable*(pwfx->nChannels * pwfx->wBitsPerSample / 8);
                data.resize(dataSize);
                if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
                {
                    memset(data.data(),0,dataSize);
                } else{
                    memcpy_s(data.data(), dataSize, pData, dataSize);
                }
                eRenderIAudioCaptureClient->ReleaseBuffer(numFramesAvailable);
            }
            qbq->put((BYTE*)data.data(),data.size(),BlockingQueueEnum::DELETEHEAD);
            emit notifyData(data);
        }
        eRenderClient->Stop();

        eRenderDevice->Release();
        eRenderClient->Release();
        eRenderIAudioCaptureClient->Release();
        CloseHandle(eRenderEvent);
    }

    void AudioCapture::stop() {
        this->isRun = false;
    }

    void AudioCapture::runMuteThread() {
        HRESULT hr;
        REFERENCE_TIME hnsRequestedDuration = 10000000;
        REFERENCE_TIME hnsActualDuration;
        IMMDeviceEnumerator *pEnumerator = NULL;
        IMMDevice *pDevice = NULL;
        IAudioClient *pAudioClient = NULL;
        IAudioRenderClient *pRenderClient = NULL;
        WAVEFORMATEX *pwfx = NULL;
        UINT32 bufferFrameCount;
        UINT32 numFramesAvailable;
        UINT32 numFramesPadding;
        BYTE *pData;
        DWORD flags = 0;

        hr = pOutputEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
        DWORD d;

        hr = pDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&pAudioClient);

        hr = pAudioClient->GetMixFormat(&pwfx);


        if(pwfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
            pwfx->wFormatTag = WAVE_FORMAT_PCM;
        else if(pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
        {
            PWAVEFORMATEXTENSIBLE pEx = reinterpret_cast<PWAVEFORMATEXTENSIBLE>(pwfx);
            if (IsEqualGUID(KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, pEx->SubFormat))
            {
                pEx->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
                pEx->Samples.wValidBitsPerSample = 16;
            }
        }
        else
            goto LOOP1;
        pwfx->wBitsPerSample = 16;
        pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample / 8;
        pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;
        LOOP1:

//        hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, hnsRequestedDuration, 0, pwfx, NULL);
        hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, 0, 0, pwfx, NULL);

        HANDLE _AudioSamplesReadyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

        pAudioClient->SetEventHandle(_AudioSamplesReadyEvent);

        hr = pAudioClient->GetBufferSize(&bufferFrameCount);
        hr = pAudioClient->GetService(IID_IAudioRenderClient, (void**)&pRenderClient);



        hr = pAudioClient->Start();
        while (isRun)
        {
            WaitForSingleObject(_AudioSamplesReadyEvent,INFINITY);
            hr = pAudioClient->GetCurrentPadding(&numFramesPadding);
            if (numFramesPadding == bufferFrameCount) continue;

            numFramesAvailable = bufferFrameCount - numFramesPadding;
            hr = pRenderClient->GetBuffer(numFramesAvailable, &pData);

            memset(pData,0,numFramesAvailable * pwfx->nBlockAlign);
            flags = 0;

            hr = pRenderClient->ReleaseBuffer(numFramesAvailable, flags);
        }
        pAudioClient->Stop();
        pEnumerator->Release();
        pDevice->Release();
        pAudioClient->Release();
        pRenderClient->Release();
    }

    std::vector<AudioDevice> AudioCapture::showDevices() const {
        return this->audioDevices;
    }

    void AudioCapture::getFrame(void* ptr,int size) {
        qbq->get((BYTE*)ptr,size,BlockingQueueEnum::WAIT);
    }

    bool AudioCapture::isRunning() const {
        return isRun;
    }

}