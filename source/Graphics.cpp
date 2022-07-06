//
// Created by chen on 2022/6/30.
//

#include "Graphics.h"
#include "windows.h"
#include "d3d9.h"
#include "Logger.h"

namespace DX {


    Graphics::Graphics() {

    }

    void Graphics::getMonitors() {
        HRESULT hr = S_OK;

        IDirect3D9Ex *d3d9ex = nullptr;
        hr = Direct3DCreate9Ex(D3D_SDK_VERSION, &d3d9ex);
        if (FAILED(hr)) {
            ERROROUT("Direct3DCreate9Ex ERROR");
            return;
        }

        int adapter_count = d3d9ex->GetAdapterCount();

        for (int i = 0; i < adapter_count; i++) {
            Monitor monitor;
            memset(&monitor, 0, sizeof(Monitor));

            LUID luid = {0, 0};
            hr = d3d9ex->GetAdapterLUID(i, &luid);
            if (FAILED(hr)) {
                continue;
            }

            monitor.low_part = (uint64_t) luid.LowPart;
            monitor.high_part = (uint64_t) luid.HighPart;

            HMONITOR hMonitor = d3d9ex->GetAdapterMonitor(i);
            if (hMonitor) {
                MONITORINFO monitor_info;
                monitor_info.cbSize = sizeof(MONITORINFO);
                BOOL ret = GetMonitorInfoA(hMonitor, &monitor_info);
                if (ret) {
                    monitor.left = monitor_info.rcMonitor.left;
                    monitor.right = monitor_info.rcMonitor.right;
                    monitor.top = monitor_info.rcMonitor.top;
                    monitor.bottom = monitor_info.rcMonitor.bottom;
                    monitors.push_back(monitor);
                }
            }
        }
        d3d9ex->Release();
    }

    void Graphics::saveBmpToDisk(std::string filename, void *data) {
        HANDLE hFile = CreateFileA(filename.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
                                   NULL);
        if (hFile == NULL) {
            return;
        }
        // ��д���ֽ���
        DWORD bytesWritten = 0;
        // λͼ��С����ɫĬ��Ϊ32λ��rgba
        int bmpSize = width * height * 4;

        // �ļ�ͷ
        BITMAPFILEHEADER bmpHeader;
        // �ļ��ܴ�С = �ļ�ͷ + λͼ��Ϣͷ + λͼ����
        bmpHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bmpSize;
        // �̶�
        bmpHeader.bfType = 0x4D42;
        // ����ƫ�ƣ���λͼ��������λ��
        bmpHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
        // ����Ϊ0
        bmpHeader.bfReserved1 = 0;
        // ����Ϊ0
        bmpHeader.bfReserved2 = 0;
        // д�ļ�ͷ
        WriteFile(hFile, (LPSTR) &bmpHeader, sizeof(bmpHeader), &bytesWritten, NULL);

        // λͼ��Ϣͷ
        BITMAPINFOHEADER bmiHeader;
        // λͼ��Ϣͷ��С
        bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        // λͼ���ؿ��
        bmiHeader.biWidth = width;
        // λͼ���ظ߶ȣ����߶ȼ����·�ת
        bmiHeader.biHeight = -height;
        // ����Ϊ1
        bmiHeader.biPlanes = 1;
        // ������ռλ��
        bmiHeader.biBitCount = 32;
        // 0��ʾ��ѹ��
        bmiHeader.biCompression = 0;
        // λͼ���ݴ�С
        bmiHeader.biSizeImage = bmpSize;
        // ˮƽ�ֱ���(����/��)
        bmiHeader.biXPelsPerMeter = 0;
        // ��ֱ�ֱ���(����/��)
        bmiHeader.biYPelsPerMeter = 0;
        // ʹ�õ���ɫ��0Ϊʹ��ȫ����ɫ
        bmiHeader.biClrUsed = 0;
        // ��Ҫ����ɫ����0Ϊ������ɫ����Ҫ
        bmiHeader.biClrImportant = 0;

        // дλͼ��Ϣͷ
        WriteFile(hFile, (LPSTR) &bmiHeader, sizeof(bmiHeader), &bytesWritten, NULL);
        // дλͼ����
        WriteFile(hFile, data, bmpSize, &bytesWritten, NULL);
        CloseHandle(hFile);
    }

    void Graphics::mallocBuffer(void* &buffer, uint32_t &size) {
        size = width * height * 4;
        buffer = malloc(size);
    }

    int Graphics::getWidth() const {
        return width;
    }

    int Graphics::getHeight() const {
        return height;
    }

} // DX