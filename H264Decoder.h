/*************************************************************************
	> File Name: H264Parse.h
	> Author: pzx
	> Created Time: 2019年02月20日 星期三 15时06分05秒
************************************************************************/
#ifndef H264DECODER_H_
#define H264DECODER_H_
#include "MediaDecoder.h"
class H264Decoder:public MediaDecoder
{
    public:
    H264Decoder();
    ~H264Decoder();
    int decodeVideo();
    private:
    MediaDecoder* audioDecoder_;
    MediaDecoder* videoDecoder_;
};
#endif  // H264DECODER_H_
