/*************************************************************************
	> File Name: H264Parse.h
	> Author: pzx
	> Created Time: 2019年02月20日 星期三 15时06分05秒
************************************************************************/
#ifndef __H264PARSER_H__
#define __H264PARSER_H__
#include "MediaDecoder.h"
class H264Decoder:public MediaDecoder
{
    public:
    H264Decoder();
    ~H264Decoder();
};
#endif //__H264PARSER_H__
