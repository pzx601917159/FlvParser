/*************************************************************************
	> File Name: H264Parse.h
	> Author: pzx
	> Created Time: 2019年02月20日 星期三 15时06分05秒
************************************************************************/
#include "OpenH264Decoder.h"
#include <iostream>
#include <stdlib.h>
#include <limits.h>
#include "SDLPlayer.h"
using namespace std;

OpenH264Decoder::OpenH264Decoder()
{
}

OpenH264Decoder::~OpenH264Decoder()
{
}

int OpenH264Decoder::init()
{
    int ret = WelsCreateDecoder(&decoder_);
    if(ret == 0)
    {
        decParam_ = {0};
        decParam_.uiTargetDqLayer = UCHAR_MAX;
        decParam_.eEcActiveIdc = ERROR_CON_FRAME_COPY_CROSS_IDR;
        decParam_.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_DEFAULT;
        ret = decoder_->Initialize(&decParam_);
        if(ret == 0)
        {
            printf("h264 parser init success\n");
            return 0;
        }
    }
    printf("h264 parser init failed\n");
    return -1;
}

int OpenH264Decoder::destory()
{
    decoder_->Uninitialize();
    WelsDestroyDecoder(decoder_);
    return 0;
}

int OpenH264Decoder::decodeFrame(unsigned char* frameData, unsigned int frameSize, int* width, int* height, int* pixFmt, int pts)
{
    frameData = frameData + 4;
    frameSize -= 4;
    printf("openh264 parse h264\n");
    unsigned char* dst[3] = {0};
    SBufferInfo dstBufInfo = {0};
    if(decoder_->DecodeFrame2(frameData, frameSize, dst, &dstBufInfo))
    {
        if(dstBufInfo.iBufferStatus == 1)
        {
            printf("decode success pts:%ld\n", dstBufInfo.uiOutYuvTimeStamp);
            // int format = sDstBufInfo.UsrData.sSystemBuffer.iFormat;  // I420
            int width = dstBufInfo.UsrData.sSystemBuffer.iWidth;
            int height = dstBufInfo.UsrData.sSystemBuffer.iHeight;
            printf("width:%d height:%d\n",width, height);
            int y_src_width = dstBufInfo.UsrData.sSystemBuffer.iStride[0];  // y_dst_width + padding
            int uv_src_width = dstBufInfo.UsrData.sSystemBuffer.iStride[1]; // uv_dst_width + padding
            int y_dst_width = width;
            int uv_dst_width = width / 2;
            int y_dst_height = height;
            int uv_dst_height = height / 2;
            // y1 ... y1280, padding, y1281 ... y2560, padding, y2561 ... y921600, padding
            unsigned char *y_plane = dst[0];
            // u1 ... u640, padding, u641 ... u1280, padding, u1281 ... u230400, padding
            unsigned char *u_plane = dst[1];
            // v1 ... v640, padding, v641 ... v1280, padding, v1281 ... v230400, padding
            unsigned char *v_plane = dst[2];
            // y1 ... y1280, y1281 ... y2560, y2561 ... y921600
            unsigned char *y_dst = (unsigned char *)malloc(width * height);
            // u1 ... u640, u641 ... u1280, u1281 ... u230400
            unsigned char *u_dst = (unsigned char *)malloc(width * height / 4);
            // v1 ... v640, v641 ... v1280, v1281 ... v230400
            unsigned char *v_dst = (unsigned char *)malloc(width * height / 4);
            int rows = height;
            for (int row = 0; row < rows; row++)
            {
                memcpy(y_dst + y_dst_width * row, y_plane + y_src_width * row, y_dst_width);
            }
            rows = height / 2;
            for (int row = 0; row < rows; row++)
            {
                memcpy(u_dst + uv_dst_width * row, u_plane + uv_src_width * row, uv_dst_width);
                memcpy(v_dst + uv_dst_width * row, v_plane + uv_src_width * row, uv_dst_width);
            }
            // yuv file:
            // y1 ... y921600, u1 ... u230400, v1 ... v230400
            fwrite(y_dst, 1, y_dst_width * y_dst_height, fp_);
            fwrite(u_dst, 1, uv_dst_width * uv_dst_height, fp_);
            fwrite(v_dst, 1, uv_dst_width * uv_dst_height, fp_);
            unsigned char* buf = (unsigned char*)malloc(width * height * 12 / 8);
            memcpy(buf, y_dst,y_dst_width * y_dst_height);
            memcpy(buf + width*height, u_dst,uv_dst_width * uv_dst_height);
            memcpy(buf + width*height + width*height/4, v_dst,uv_dst_width * uv_dst_height);

            SDLPlayer::instance()->play(buf);
        }
        else
        {
            printf("decode failed:%d\n",dstBufInfo.iBufferStatus);
        }
    }
    else
    {
        printf("decode failed\n");
    }
    return 0;
}
