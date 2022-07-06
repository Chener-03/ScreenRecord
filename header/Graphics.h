//
// Created by chen on 2022/6/30.
//

#ifndef UNTITLED_GRAPHICS_H
#define UNTITLED_GRAPHICS_H
#include <stdint.h>
#include <vector>
#include <string>
#include <mutex>
using std::vector;
namespace DX {

    struct Monitor
    {
        uint64_t low_part;
        uint64_t high_part;

        int left;
        int top;
        int right;
        int bottom;
    };

    class Graphics {
    public:
        Graphics();
        virtual bool init() = 0;
        virtual bool getFrame(void* pImageDate ) = 0;
        virtual void destroy()=0;
        void saveBmpToDisk(std::string filename,void* data );
        void mallocBuffer(void* &buffer,uint32_t& size);
        virtual bool isRunning() = 0;

    public:
        int getWidth() const;
        int getHeight() const;


    protected:
        int width = 0;
        int height = 0;
        vector<Monitor> monitors;
        void getMonitors();

    };

} // DX

#endif //UNTITLED_GRAPHICS_H
