cmake_minimum_required(VERSION 3.22)
project(untitled)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)
add_definitions(-w)

set(CMAKE_PREFIX_PATH C:\\Qt\\6.3.1\\mingw_64)
set(DX_LIB_64 ${PROJECT_SOURCE_DIR}\\3rdparty\\dxsdk\\Lib\\x64)
set(FFMPEG_INCLUDE ${PROJECT_SOURCE_DIR}\\3rdparty\\ffmpeg/include)
set(FFMPEG_LIB ${PROJECT_SOURCE_DIR}\\3rdparty\\ffmpeg/lib)


file(GLOB_RECURSE SRC_DIR_LIST_LIBYUV 3rdparty/libyuv/source/*.cc)

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED
        )


include_directories(${PROJECT_SOURCE_DIR}/source
        ${PROJECT_SOURCE_DIR}/header
        ${PROJECT_SOURCE_DIR}/qrc
        ${FFMPEG_INCLUDE}
        3rdparty/libyuv/include
        3rdparty/cli11
)
link_directories(
      ${DX_LIB_64}
      ${FFMPEG_LIB}
)


add_executable(untitled source/main.cpp source/DXGIGraphics.cpp header/DXGIGraphics.h source/Graphics.cpp header/Graphics.h header/Logger.h header/timeutils.h
        source/Encode.cpp header/Encode.h
        ${SRC_DIR_LIST_LIBYUV} header/VideoUtils.h source/GDIGraphics.cpp header/GDIGraphics.h source/AudioCapture.cpp header/AudioCapture.h
        header/QBlockingQueue.hpp source/timeutils.cpp source/VideoUtils.cpp)

target_link_libraries(untitled
        Qt::Core
        Qt::Gui
        Qt::Widgets
        dxgi.lib
        d3d11.lib
        d3d9.lib
        avcodec.lib
        avformat.lib
        avutil.lib
        swscale.lib
        uuid.lib
        ksuser.lib
        ole32.lib
        avrt.lib
        psapi.lib
        swresample.lib
        )

if (WIN32)
    set(DEBUG_SUFFIX)
    if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif ()
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif ()
    endif ()
    if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
    endif ()
    foreach (QT_LIB Core Gui Widgets)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach (QT_LIB)
endif ()
