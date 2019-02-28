cmake_minimum_required(VERSION 3.11)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BIN_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(PRJ_NAME flvparser)
file(GLOB cppsrc "${SRC_DIR}/*.cpp")
add_compile_options(-std=c++11)
add_executable(${PRJ_NAME} ${cppsrc})
target_link_libraries(${PRJ_NAME}
    avcodec
    avformat
    avutil 
    swscale
    SDL2 
    fdk-aac 
    openh264
    pthread 
    swresample)
