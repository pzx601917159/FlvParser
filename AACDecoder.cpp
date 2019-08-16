/*************************************************************************
	> File Name: H264Parse.h
	> Author: pzx
	> Created Time: 2019年02月20日 星期三 15时06分05秒
************************************************************************/
#include "AACDecoder.h"
#include <iostream>
extern "C"
{
#include "libswresample/swresample.h"
}
#include "SDLPlayer.h"

#define MAX_AUDIO_FRAME_SIZE 192000


AACDecoder::AACDecoder()
{
    // aac
    codecId_ = AV_CODEC_ID_AAC;
    codec_ = nullptr;
    codecCtx_ = nullptr;
    au_convert_ctx_ = nullptr;
}

AACDecoder::~AACDecoder()
{
}

int AACDecoder::init()
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
    codecCtx_->codec_type = AVMEDIA_TYPE_AUDIO;
    codecCtx_->sample_rate = 44100;
    codecCtx_->channels = 2;
    codecCtx_->bit_rate = 16;
    codecCtx_->sample_fmt = AV_SAMPLE_FMT_S16;
    if(avcodec_open2(codecCtx_, codec_, NULL) < 0)
    {
        std::cout << "avcodec_open2 error" << std::endl;
        return -1;
    }
    frame_ = av_frame_alloc();
    if(!frame_)
    {
        std::cout << "av_frame_alloc error" << std::endl;
        return -1;
    }
    frameYUV_ = av_frame_alloc();
    if(!frameYUV_)
    {
        std::cout << "av_frame_alloc error" << std::endl;
        return -1;
    }
    av_init_packet(&packet_);
    return 0;
}

int AACDecoder::decodeFrame(unsigned char* frameData, unsigned int frameSize,
        int* width, int* height, int* pixFmt, int pts)
{
    AVFrame *frame = av_frame_alloc();
    if(!codecCtx_ || !codec_ || !frameSize)
    {
        printf("codec is null\n");
        return -1;
    }
    printf("framesize:%d\n", frameSize);
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
    ret = avcodec_receive_frame(codecCtx_, frame_);
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
        std::cout << "decode frame success" << std::endl;
    }
    if (!au_convert_ctx_)
    {
        au_convert_ctx_ = swr_alloc();

        int64_t channelslayout = av_get_default_channel_layout(
                frame_->channels);
        AVSampleFormat out_fmt = AV_SAMPLE_FMT_S16;

        au_convert_ctx_ = swr_alloc_set_opts(au_convert_ctx_,
                channelslayout, out_fmt, frame_->sample_rate,
                av_get_default_channel_layout(frame_->channels),
                codecCtx_->sample_fmt, codecCtx_->sample_rate, 0, NULL);

        swr_init(au_convert_ctx_);
    }

    out_buf_ = reinterpret_cast<uint8_t *>(malloc(MAX_AUDIO_FRAME_SIZE*2));
    swr_convert(au_convert_ctx_, &out_buf_,
            swr_get_out_samples(au_convert_ctx_, frame_->nb_samples),
            (const uint8_t **)(frame_->data), frame_->nb_samples);

    // swr_convert(au_convert_ctx_, &out_buf_, MAX_AUDIO_FRAME_SIZE,
    // (const uint8_t**)frame_->data, frame_->nb_samples);
    int out_buf_size = av_samples_get_buffer_size(NULL, 2, frame_->nb_samples,
            AV_SAMPLE_FMT_S16, 1);
    // 显示数据
    //SDLPlayer::instance()->playAudio2(out_buf_, out_buf_size);
    printf("audio line size:%d %d\n", frame_->linesize[0], frame_->linesize[1]);
    return 0;
}
