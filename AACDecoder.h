/*************************************************************************
	> File Name: H264Parse.h
	> Author: pzx
	> Created Time: 2019年02月20日 星期三 15时06分05秒
************************************************************************/
#ifndef __AACDECODER_H__
#define __AACDECODER_H__
#include "MediaDecoder.h"
class AACDecoder:public MediaDecoder
{
    public:
    AACDecoder();
    ~AACDecoder();
    virtual int init();
    virtual int destory(){}
    virtual int decodeFrame(unsigned char* frameData, unsigned int frameSize, int* width, int* height, int* pixFmt, int pts);

    struct SwrContext *au_convert_ctx_;
    uint8_t* out_buf_;
};

#endif //__AACDECODER_H__
