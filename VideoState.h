/*************************************************************************
	> File Name: VideoState.h
	> Author: pzx
	> Created Time: 2019年03月04日 星期一 17时34分18秒
************************************************************************/
#ifndef VIDEOSTATE_H_
#define VIDEOSTATE_H_

#include <queue>
#include <memory>
#include <vector>
#include "ByteUtil.h"

class Tag;  // 存储flv的tag

constexpr int32_t MAX_TAG_SIZE = 1024;
constexpr int32_t MAX_PACKET_SIZE = 1024;
constexpr int32_t MAX_FRAME_SIZE = 1024;

struct AVFrame;

// 存储视频的以下全局变量
class VideoState
{
    public:
    // video
    static double videoWidth_;
    static double videoHeight_;
    // audio
    static int audioFreq_;     // 采样率
    static int audioFomrat_;   // 音频位数
    static int audioChannels_;  // 通道数
    static std::queue<Tag*> videoTags_;      // 存储flv数据包
    static std::queue<Tag*> audioTags_;      // 存储flv数据包
    static std::queue<AVFrame*> videoFrames_;      // 存储flv数据包
    static std::queue<AVFrame*> audioFrames_;      // 存储flv数据包
};
#endif  // VIDEOSTATE_H_
