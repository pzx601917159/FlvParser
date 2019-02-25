/*************************************************************************
	> File Name: H264Parse.h
	> Author: pzx
	> Created Time: 2019年02月20日 星期三 15时06分05秒
************************************************************************/
#ifndef __OPENH264PARSER_H__
#define __OPENH264PARSER_H__

#include "wels/codec_api.h"
#include "MediaDecoder.h"
class OpenH264Decoder:public MediaDecoder
{
    public:
    OpenH264Decoder();
    ~OpenH264Decoder();
    virtual int init();
    virtual int destory();
    virtual int decodeFrame(unsigned char* frameData, unsigned int frameSize, int* width, int* height, int* pixFmt);
    private:
    ISVCDecoder* decoder_;
    SDecodingParam decParam_;
};

#endif //__OPENH264PARSER_H__
