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
        // 已写入字节数
        DWORD bytesWritten = 0;
        // 位图大小，颜色默认为32位即rgba
        int bmpSize = width * height * 4;

        // 文件头
        BITMAPFILEHEADER bmpHeader;
        // 文件总大小 = 文件头 + 位图信息头 + 位图数据
        bmpHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bmpSize;
        // 固定
        bmpHeader.bfType = 0x4D42;
        // 数据偏移，即位图数据所在位置
        bmpHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
        // 保留为0
        bmpHeader.bfReserved1 = 0;
        // 保留为0
        bmpHeader.bfReserved2 = 0;
        // 写文件头
        WriteFile(hFile, (LPSTR) &bmpHeader, sizeof(bmpHeader), &bytesWritten, NULL);

        // 位图信息头
        BITMAPINFOHEADER bmiHeader;
        // 位图信息头大小
        bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        // 位图像素宽度
        bmiHeader.biWidth = width;
        // 位图像素高度，负高度即上下翻转
        bmiHeader.biHeight = -height;
        // 必须为1
        bmiHeader.biPlanes = 1;
        // 像素所占位数
        bmiHeader.biBitCount = 32;
        // 0表示不压缩
        bmiHeader.biCompression = 0;
        // 位图数据大小
        bmiHeader.biSizeImage = bmpSize;
        // 水平分辨率(像素/米)
        bmiHeader.biXPelsPerMeter = 0;
        // 垂直分辨率(像素/米)
        bmiHeader.biYPelsPerMeter = 0;
        // 使用的颜色，0为使用全部颜色
        bmiHeader.biClrUsed = 0;
        // 重要的颜色数，0为所有颜色都重要
        bmiHeader.biClrImportant = 0;

        // 写位图信息头
        WriteFile(hFile, (LPSTR) &bmiHeader, sizeof(bmiHeader), &bytesWritten, NULL);
        // 写位图数据
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