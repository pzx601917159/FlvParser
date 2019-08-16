/*************************************************************************
	> File Name: VideoState.h
	> Author: pzx
	> Created Time: 2019年03月04日 星期一 17时34分18秒
************************************************************************/
#include "VideoState.h"
#include <queue>
// video
double VideoState::videoWidth_ = 320;
double VideoState::videoHeight_ = 240;
// audio
int VideoState::audioFreq_;     // 采样率
int VideoState::audioFomrat_;   // 音频位数
int VideoState::audioChannels_;  // 通道数
std::queue<Tag*> VideoState::videoTags_;      // 存储flv数据包
std::queue<Tag*> VideoState::audioTags_;      // 存储flv数据包
std::queue<AVFrame*> VideoState::videoFrames_;      // 存储解码后的数据包
std::queue<AVFrame*> VideoState::audioFrames_;      // 存储解码后的数据包
