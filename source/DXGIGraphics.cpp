//
// Created by chen on 2022/6/30.
//

#include "DXGIGraphics.h"
#include "windows.h"
#define RESET_OBJECT(obj) { if(obj) obj->Release(); obj = NULL; }

namespace DX {

    bool DXGIGraphics::init() {

        HRESULT hr = S_OK;

        // Driver types supported
        D3D_DRIVER_TYPE DriverTypes[] =
                {
                        D3D_DRIVER_TYPE_HARDWARE,
                        D3D_DRIVER_TYPE_WARP,
                        D3D_DRIVER_TYPE_REFERENCE,
                };
        UINT NumDriverTypes = ARRAYSIZE(DriverTypes);

        // Feature levels supported
        D3D_FEATURE_LEVEL FeatureLevels[] =
                {
                        D3D_FEATURE_LEVEL_11_0,
                        D3D_FEATURE_LEVEL_10_1,
                        D3D_FEATURE_LEVEL_10_0,
                        D3D_FEATURE_LEVEL_9_1
                };
        UINT NumFeatureLevels = ARRAYSIZE(FeatureLevels);

        D3D_FEATURE_LEVEL FeatureLevel;

        //
        // Create D3D device
        //
        for (UINT DriverTypeIndex = 0; DriverTypeIndex < NumDriverTypes; ++DriverTypeIndex)
        {
            hr = D3D11CreateDevice(NULL, DriverTypes[DriverTypeIndex], NULL, 0, FeatureLevels, NumFeatureLevels, D3D11_SDK_VERSION, &m_hDevice, &FeatureLevel, &m_hContext);
            if (SUCCEEDED(hr))
            {
                break;
            }
        }
        if (FAILED(hr))
        {
            return FALSE;
        }

        //
        // Get DXGI device
        //
        IDXGIDevice *hDxgiDevice = NULL;
        hr = m_hDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&hDxgiDevice));
        if (FAILED(hr))
        {
            return FALSE;
        }

        //
        // Get DXGI adapter
        //
        IDXGIAdapter *hDxgiAdapter = NULL;
        hr = hDxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&hDxgiAdapter));
        RESET_OBJECT(hDxgiDevice);
        if (FAILED(hr))
        {
            return FALSE;
        }

        //
        // Get output
        //
        INT nOutput = 0;
        IDXGIOutput *hDxgiOutput = NULL;

        hr = hDxgiAdapter->EnumOutputs(nOutput, &hDxgiOutput);
        RESET_OBJECT(hDxgiAdapter);
        if (FAILED(hr))
        {
            return FALSE;
        }

        //
        // get output description struct
        //
        DXGI_OUTPUT_DESC        m_dxgiOutDesc;
        hDxgiOutput->GetDesc(&m_dxgiOutDesc);
        width = m_dxgiOutDesc.DesktopCoordinates.right - m_dxgiOutDesc.DesktopCoordinates.left;
        height = m_dxgiOutDesc.DesktopCoordinates.bottom - m_dxgiOutDesc.DesktopCoordinates.top;
        //
        // QI for Output 1
        //
        IDXGIOutput1 *hDxgiOutput1 = NULL;
        hr = hDxgiOutput->QueryInterface(__uuidof(hDxgiOutput1), reinterpret_cast<void**>(&hDxgiOutput1));
        RESET_OBJECT(hDxgiOutput);
        if (FAILED(hr))
        {
            return FALSE;
        }

        //
        // Create desktop duplication
        //
        hr = hDxgiOutput1->DuplicateOutput(m_hDevice, &m_hDeskDupl);
        if (hr == DXGI_ERROR_UNSUPPORTED)
        {
            ERROROUT("笔记本请使用集显运行程序，或者开启独显直连后再使用独显运行");
        }
        RESET_OBJECT(hDxgiOutput1);
        if (FAILED(hr))
        {
            return FALSE;
        }

        this->start();
        Sleep(100);
        INFOOUT("DXGIGraphics Start");
        return TRUE;
    }

    bool DXGIGraphics::getFrame0(void *pImageDate) {


        IDXGIResource *hDesktopResource = NULL;
        DXGI_OUTDUPL_FRAME_INFO FrameInfo;
        m_hDeskDupl->ReleaseFrame();
        HRESULT hr = m_hDeskDupl->AcquireNextFrame(500, &FrameInfo, &hDesktopResource);
        if (FAILED(hr))
        {
            if (hr == DXGI_ERROR_WAIT_TIMEOUT)
            {
                return true;
            }else if(hr == DXGI_ERROR_ACCESS_LOST)
            {
                reloadIDXGIOutputDuplication();
                return false;
            }
            else
            {
                return false;
            }
        }

        //
        // 查询下一帧暂存缓冲区
        //
        ID3D11Texture2D *hAcquiredDesktopImage = NULL;
        hr = hDesktopResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&hAcquiredDesktopImage));
        RESET_OBJECT(hDesktopResource);
        if (FAILED(hr))
        {
            return FALSE;
        }

        //
        // 复制旧 description
        //
        D3D11_TEXTURE2D_DESC frameDescriptor;
        hAcquiredDesktopImage->GetDesc(&frameDescriptor);

        //
        // 为填充帧图像创建新的暂存缓冲区
        //
        ID3D11Texture2D *hNewDesktopImage = NULL;
        frameDescriptor.Usage = D3D11_USAGE_STAGING;
        frameDescriptor.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        frameDescriptor.BindFlags = 0;
        frameDescriptor.MiscFlags = 0;
        frameDescriptor.MipLevels = 1;
        frameDescriptor.ArraySize = 1;
        frameDescriptor.SampleDesc.Count = 1;
        hr = m_hDevice->CreateTexture2D(&frameDescriptor, NULL, &hNewDesktopImage);
        if (FAILED(hr))
        {
            RESET_OBJECT(hAcquiredDesktopImage);
            m_hDeskDupl->ReleaseFrame();
            return FALSE;
        }

        //
        // 将下一个暂存缓冲区复制到新的暂存缓冲区
        //
        m_hContext->CopyResource(hNewDesktopImage, hAcquiredDesktopImage);

        RESET_OBJECT(hAcquiredDesktopImage);
        m_hDeskDupl->ReleaseFrame();

        //
        // 为映射位创建暂存缓冲区
        //
        IDXGISurface *hStagingSurf = NULL;
        hr = hNewDesktopImage->QueryInterface(__uuidof(IDXGISurface), (void **)(&hStagingSurf));
        RESET_OBJECT(hNewDesktopImage);
        if (FAILED(hr))
        {
            return FALSE;
        }
        DXGI_SURFACE_DESC hStagingSurfDesc;

        hStagingSurf->GetDesc(&hStagingSurfDesc);

        DXGI_MAPPED_RECT mappedRect;
        hr = hStagingSurf->Map(&mappedRect, DXGI_MAP_READ);
        int imgSize = width * height *4;
        copySize = imgSize;
        if (SUCCEEDED(hr))
        {
            frameLock.lock();
            memcpy(pImageDate, mappedRect.pBits, imgSize);
            frameLock.unlock();
            hStagingSurf->Unmap();
        }else {

        }

        RESET_OBJECT(hStagingSurf);
        return SUCCEEDED(hr);
    }

    void DXGIGraphics::destroy() {
        stop();
        RESET_OBJECT(m_hDeskDupl);
        RESET_OBJECT(m_hContext);
        RESET_OBJECT(m_hDevice);
        free(framePtr);
        INFOOUT("DXGIGraphics Stop");
    }

    void DXGIGraphics::start() {
        isRun = true;
        framePtr = malloc(width*height*4);
        std::thread t1(&DXGIGraphics::frame,this);
        t1.detach();
    }

    void DXGIGraphics::stop() {
        isRun = false;
        Sleep(100);
    }

    void DXGIGraphics::frame() {

        while(isRun)
        {
            getFrame0(this->framePtr);
            Sleep(1000/fps);
        }
    }

    bool DXGIGraphics::getFrame(void *pImageDate ) {
        if(pImageDate== nullptr)
        {
            ERROROUT("pImageDate maybe nullptr");
            return false;
        }
        frameLock.lock();
        memcpy(pImageDate,framePtr,copySize);
        frameLock.unlock();
        return true;
    }

    uint32_t DXGIGraphics::getBufferSize() const {
        return this->copySize;
    }

    bool DXGIGraphics::isRunning() {
        return isRun;
    }

    void DXGIGraphics::reloadIDXGIOutputDuplication() {
        printf("reloadIDXGIOutputDuplication \n");
    }
} // DX


