/*************************************************************************
	> File Name: Decoder.h
	> Author: pzx
	> Created Time: 2019年03月04日 星期一 17时11分10秒
************************************************************************/
#include "Decoder.h"

Decoder::Decoder()
{
    videoDecoder_ = nullptr;
    audioDecoder_ = nullptr;
}

int Decoder::init()
{
    return 0;
}

int Decoder::destory()
{
    return 0;
}

// 解码视频
int Decoder::decodeVideo()
{
    return 0;
}

// 解码音频
int Decoder::decodeAudio()
{
    return 0;
}
