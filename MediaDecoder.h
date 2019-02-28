/*************************************************************************
	> File Name: H264Parse.h
	> Author: pzx
	> Created Time: 2019年02月20日 星期三 15时06分05秒
************************************************************************/
#ifndef __BASEPARSER_H__
#define __BASEPARSER_H__

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
    virtual int init();
    virtual int destory();
    virtual int decodeFrame(unsigned char* frameData, unsigned int frameSize, int* width, int* height, int* pixFmt, int pts);
    AVCodec* codec_;
    AVCodecContext* codecCtx_;
    AVFrame* frame_;
    AVFrame* frameYUV_;
    AVPacket packet_;
    AVCodecID codecId_;
    struct SwsContext *img_convert_ctx_;
    FILE* fp_;
};

#endif //__BASEPARSER_H__
