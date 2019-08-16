/*************************************************************************
	> File Name: H264Parse.h
	> Author: pzx
	> Created Time: 2019年02月20日 星期三 15时06分05秒
************************************************************************/
#ifndef MEDIADECODER_H_
#define MEDIADECODER_H_

#include <queue>
extern "C"
{
    #include "libavformat/avformat.h"
    #include "libavutil/avutil.h"
    #include "libavcodec/avcodec.h"
    #include "libswscale/swscale.h"
    #include "libavutil/imgutils.h"
}
class MediaDecoder
{
    public:
    MediaDecoder();
    ~MediaDecoder();

    // 解码
    int decode();
    virtual int init();
    virtual int destory();
    virtual int decodeFrame(unsigned char* frameData, unsigned int frameSize,
            int* width, int* height, int* pixFmt, int pts);

    protected:
    AVCodec* codec_;
    AVCodecContext* codecCtx_;
    AVFrame* frame_;
    AVFrame* frameYUV_;
    AVPacket packet_;
    AVCodecID codecId_;
    struct SwsContext *img_convert_ctx_;
    bool initSpsPPs_;
};

#endif  // MEDIADECODER_H_
