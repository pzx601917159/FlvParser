/*************************************************************************
	> File Name: H264Parse.h
	> Author: pzx
	> Created Time: 2019年02月20日 星期三 15时06分05秒
************************************************************************/
#include "MediaDecoder.h"
#include <iostream>
#include "SDLPlayer.h"

MediaDecoder::MediaDecoder()
{
    codecId_ = AV_CODEC_ID_H264;
    codec_ = nullptr;
    initSpsPPs_ = false;
}

MediaDecoder::~MediaDecoder()
{
}

int MediaDecoder::init()
{
    avcodec_register_all();
    codec_  = avcodec_find_decoder(codecId_);
    if(!codec_)
    {
        std::cout << "avcodec_find_decoder error" << std::endl;
        return -1;
    }
    codecCtx_ = avcodec_alloc_context3(codec_);
    if(!codecCtx_)
    {
        std::cout << "avcodec_alloc_context3 error" << std::endl;
        return -1;
    }
    codecCtx_->width = 320;
    codecCtx_->height = 240;
    codecCtx_->pix_fmt = AV_PIX_FMT_YUV420P;  // YUV420P
    codecCtx_->flags|AV_CODEC_FLAG_GLOBAL_HEADER;
    //screenWidth_ = VideoState::videoWidth_;
    //screenHeight_ = VideoState::videoHeight_;
    frame_ = av_frame_alloc();
    if(!frame_)
    {
        std::cout << "av_frame_alloc error" << std::endl;
        return -1;
    }

    av_init_packet(&packet_);
    return 0;
}

int MediaDecoder::destory()
{
    // TODO 释放资源
    av_frame_free(&frame_);
    avcodec_close(codecCtx_);
    avcodec_free_context(&codecCtx_);
    return 0;
}

int MediaDecoder::decodeFrame(unsigned char* frameData, unsigned int frameSize,
        int* width, int* height, int* pixFmt, int pts)
{
    if(!initSpsPPs_)
    {
        codecCtx_->extradata_size = frameSize;
        codecCtx_->extradata = reinterpret_cast<uint8_t*>(av_malloc(frameSize + AV_INPUT_BUFFER_PADDING_SIZE));
        // pps sps拷贝即可无需解码
        memcpy(codecCtx_->extradata, frameData, frameSize);
        initSpsPPs_ = true;
        if(avcodec_open2(codecCtx_, codec_, NULL) < 0)
        {
            std::cout << "avcodec_open2 error" << std::endl;
            return -1;
        }
        return 0;
    }
    AVFrame *frame = av_frame_alloc();
    packet_.size = frameSize;
    packet_.data = frameData;
    int ret;
    ret = avcodec_send_packet(codecCtx_, &packet_);
    if(ret != 0)
    {
        if(ret == AVERROR(EAGAIN))
        {
            std::cout << "EAGAIN" << std::endl;
        }
        else if(ret == AVERROR_EOF)
        {
            std::cout << "AVERROR_EOF" << std::endl;
        }
        else
        {
            std::cout << "avcodec_send_packet fail, errorcode:" <<
                ret << std::endl;
            return -1;
        }
    }
    ret = avcodec_receive_frame(codecCtx_, frame);
    if(ret != 0)
    {
        if(ret == AVERROR(EAGAIN))
        {
            std::cout << "EAGAIN" << std::endl;
        }
        else if(ret == AVERROR_EOF)
        {
            std::cout << "AVERROR_EOF" << std::endl;
        }
        else
        {
            std::cout << "avcodec_receive_frame fail, errorcode:" <<
                ret  << std::endl;
        }
    }
    else
    {
        if(frame_->pts ==  AV_NOPTS_VALUE)
        {
            // std::cout << "no pts value" << std::endl;
        }
        else
        {
            std::cout << "decode frame success pts:" << frame_->pts
                << std::endl;
        }
    }
    // 显示数据
    // SDLPlayer::instance()->play(frame_, pts, codecCtx_->width,
    //        codecCtx_->height);
    VideoState::videoFrames_.push(frame);
    return 0;
}
