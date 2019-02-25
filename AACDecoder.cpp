/*************************************************************************
	> File Name: H264Parse.h
	> Author: pzx
	> Created Time: 2019年02月20日 星期三 15时06分05秒
************************************************************************/
#include "AACDecoder.h"
#include "SDLPlayer.h"
#include <iostream>
using namespace std;


AACDecoder::AACDecoder()
{
    //aac
    codecId_ = AV_CODEC_ID_AAC;
}

AACDecoder::~AACDecoder()
{
}

int AACDecoder::decodeFrame(unsigned char* frameData, unsigned int frameSize, int* width, int* height, int* pixFmt)
{
    printf("framesize:%d\n",frameSize);
    packet_.size = frameSize;
    packet_.data = frameData;
    int ret;
    ret = avcodec_send_packet(codecCtx_, &packet_);
    if(ret != 0)
    {
        if(ret == AVERROR(EAGAIN))
        {
            cout << "EAGAIN" << endl;
        }
        else if(ret == AVERROR_EOF)
        {
            cout << "AVERROR_EOF" << endl;
        }
        else
        {
            cout << "avcodec_send_packet fail, errorcode:" << ret << endl;
            return -1;
        }
    }
    ret = avcodec_receive_frame(codecCtx_,frame_);
    if(ret != 0)
    {
        if(ret == AVERROR(EAGAIN))
        {
            cout << "EAGAIN" << endl;
        }
        else if(ret == AVERROR_EOF)
        {
            cout << "AVERROR_EOF" << endl;
        }
        else
        {
            cout << "avcodec_receive_frame fail, errorcode:" << ret  << endl;
        }
    }
    else
    {
        cout << "decode frame success" << endl;
    }
    // 显示数据
    g_sdlPlayer.playAudio(frame_->data[0], frame_->linesize[0]);
    *width = codecCtx_->width;
    *height = codecCtx_->height;
    *pixFmt = codecCtx_->pix_fmt;
    return 0;
}
