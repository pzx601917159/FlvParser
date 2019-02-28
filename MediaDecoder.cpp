/*************************************************************************
	> File Name: H264Parse.h
	> Author: pzx
	> Created Time: 2019年02月20日 星期三 15时06分05秒
************************************************************************/
#include "MediaDecoder.h"
#include <iostream>
#include "SDLPlayer.h"
using namespace std;

MediaDecoder::MediaDecoder()
{
    codecId_ = AV_CODEC_ID_H264;
    fp_ = fopen("test.yuv","w+");
    codec_ = NULL;
}

MediaDecoder::~MediaDecoder()
{
    if(fp_)
    {
        fclose(fp_);
    }
}

int MediaDecoder::init()
{
    avcodec_register_all();
    codec_  = avcodec_find_decoder(codecId_);
    if(!codec_)
    {
        cout << "avcodec_find_decoder error" << endl;
        return -1;
    }
    codecCtx_ = avcodec_alloc_context3(codec_);
    if(!codecCtx_)
    {
        cout << "avcodec_alloc_context3 error" << endl;
        return -1;
    }
    codecCtx_->width = 320;
    codecCtx_->height = 240;
    codecCtx_->pix_fmt = AV_PIX_FMT_YUV420P;//YUV420P
    if(avcodec_open2(codecCtx_, codec_, NULL) < 0)
    {
        cout << "avcodec_open2 error" << endl;
        return -1;
    }
    frame_ = av_frame_alloc();
    if(!frame_)
    {
        cout << "av_frame_alloc error" << endl;
        return -1;
    }
    frameYUV_ = av_frame_alloc();
    if(!frameYUV_)
    {
        cout << "av_frame_alloc error" << endl;
        return -1;
    }
    unsigned char* out_buffer=(unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P,  codecCtx_->width, codecCtx_->height,1));
    img_convert_ctx_ = sws_getContext(codecCtx_->width, codecCtx_->height, codecCtx_->pix_fmt,
		codecCtx_->width, codecCtx_->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
    av_image_fill_arrays(frameYUV_->data, frameYUV_->linesize,out_buffer,
		AV_PIX_FMT_YUV420P,codecCtx_->width, codecCtx_->height,1); 

    av_init_packet(&packet_);
    return 0;
}

int MediaDecoder::destory()
{
    return 0;
}

int MediaDecoder::decodeFrame(unsigned char* frameData, unsigned int frameSize, int* width, int* height, int* pixFmt, int pts)
{
    //printf("framesize:%d\n",frameSize);
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
        if(frame_->pts ==  AV_NOPTS_VALUE)
        {
            //cout << "no pts value" << endl;
        }
        else
        {
            cout << "decode frame success pts:" << frame_->pts << endl;
        }
    }
    /*
    sws_scale(img_convert_ctx_, (const unsigned char* const*)frame_->data, frame_->linesize, 0, codecCtx_->height, 
					frameYUV_->data, frameYUV_->linesize);

    int y_size = codecCtx_->width*codecCtx_->height;  
	fwrite(frameYUV_->data[0],1,y_size,fp_);    //Y 
	fwrite(frameYUV_->data[1],1,y_size/4,fp_);  //U
	fwrite(frameYUV_->data[2],1,y_size/4,fp_);  //V 
    */
    // 显示数据
    SDLPlayer::instance()->play(frame_, pts);
    *width = codecCtx_->width;
    *height = codecCtx_->height;
    *pixFmt = codecCtx_->pix_fmt;
    return 0;
}
