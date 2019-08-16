/*************************************************************************
	> File Name: time.h
	> Author: pzx
	> Created Time: 2019年02月26日 星期二 16时43分13秒
************************************************************************/
#ifndef CLOCK_H_
#define CLOCK_H_
#include <time.h>
#include <inttypes.h>
// 获取微妙级别的时间
inline uint64_t getTime()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

// 每秒钟的音频数据
// freq 采样率
// channels 通道数
// samples
inline int getBytesPersecond(int freq, int channels)
{
    // 2byte
    return freq * channels  * 2;
}
#endif  // CLOCK_H_
