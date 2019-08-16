/*************************************************************************
	> File Name: Decoder.h
	> Author: pzx
	> Created Time: 2019年03月04日 星期一 17时11分10秒
************************************************************************/
#ifndef DECODER_H_
#define DECODER_H_
#include "MediaDecoder.h"
class Decoder
{
    public:
    Decoder();
    ~Decoder();
    int init();
    int destory();
    // 解码视频
    int decodeVideo();
    // 解码音频
    int decodeAudio();
    private:
    MediaDecoder* videoDecoder_;
    MediaDecoder* audioDecoder_;
};
#endif  // DECODER_H_
