/*************************************************************************
	> File Name: H264Parse.h
	> Author: pzx
	> Created Time: 2019年02月20日 星期三 15时06分05秒
************************************************************************/
#ifndef __AACPARSER_H__
#define __AACPARSER_H__
#include "MediaDecoder.h"
class AACDecoder:public MediaDecoder
{
    public:
    AACDecoder();
    ~AACDecoder();
    virtual int decodeFrame(unsigned char* frameData, unsigned int frameSize, int* width, int* height, int* pixFmt);
};

#endif //__AACPARSER_H__
