
cmake_minimum_required(VERSION 3.10.2)
message(STATUS "source ---  ${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_ANDROID_ARCH_ABI ---  ${CMAKE_ANDROID_ARCH_ABI}")

set(ARCH_ABI "")
if(${CMAKE_ANDROID_ARCH_ABI} STREQUAL "arm64-v8a")
    set(ARCH_ABI "ABI_ARCH_AARCH64")
elseif(${CMAKE_ANDROID_ARCH_ABI} STREQUAL "armeabi-v7a")
    set(ARCH_ABI "ABI_ARCH_ARM")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D${ARCH_ABI} -O2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
#find_package (utilsKit REQUIRED CONFIG)
include_directories(src/main/cpp/include)

add_library(
        soak_player
        SHARED
        src/main/cpp/player/SoakPlayer.cpp
        src/main/cpp/player/SoakAudio.cpp
        src/main/cpp/player/SoakVideo.cpp
        src/main/cpp/player/SoakCallJava.cpp
        src/main/cpp/player/SoakFFmpeg.cpp
        src/main/cpp/player/PlayStatus.cpp
        src/main/cpp/player/SoakQueue.cpp
)
add_library(avcodec SHARED IMPORTED)
set_target_properties(avcodec
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/cpp/jniLibs/${ANDROID_ABI}/libavcodec.so)

add_library(avdevice SHARED IMPORTED)
set_target_properties(avdevice
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/cpp/jniLibs/${ANDROID_ABI}/libavdevice.so)

add_library(avfilter SHARED IMPORTED)
set_target_properties(avfilter
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/cpp/jniLibs/${ANDROID_ABI}/libavfilter.so)

add_library(avformat SHARED IMPORTED)
set_target_properties(avformat
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/cpp/jniLibs/${ANDROID_ABI}/libavformat.so)



add_library(avutil SHARED IMPORTED)
set_target_properties(avutil
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/cpp/jniLibs/${ANDROID_ABI}/libavutil.so)


add_library(postproc SHARED IMPORTED)
set_target_properties(postproc
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/cpp/jniLibs/${ANDROID_ABI}/libpostproc.so)


add_library(swresample SHARED IMPORTED)
set_target_properties(swresample
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/cpp/jniLibs/${ANDROID_ABI}/libswresample.so)

add_library(swscale SHARED IMPORTED)
set_target_properties(swscale
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/cpp/jniLibs/${ANDROID_ABI}/libswscale.so)


add_library(mp3lame SHARED IMPORTED)
set_target_properties(mp3lame
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/cpp/jniLibs/${ANDROID_ABI}/libmp3lame.so)

add_library(fdk-aac SHARED IMPORTED)
set_target_properties(fdk-aac
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/cpp/jniLibs/${ANDROID_ABI}/libfdk-aac.so)


add_library(x264 SHARED IMPORTED)
set_target_properties(x264
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/cpp/jniLibs/${ANDROID_ABI}/libx264.so)

add_library(ssl SHARED IMPORTED)
set_target_properties(ssl
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/cpp/jniLibs/${ANDROID_ABI}/libssl.so)

add_library(crypto SHARED IMPORTED)
set_target_properties(crypto
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/cpp/jniLibs/${ANDROID_ABI}/libcrypto.so)

target_link_libraries( # Specifies the target library.
        soak_player
        avcodec
        avdevice
        avfilter
        avformat
       # crypto
       # ssl
        avutil
        postproc
        swresample
        swscale
        mp3lame
        fdk-aac
        x264
        OpenSLES
        android
        log
        )