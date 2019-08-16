/*************************************************************************
	> File Name: DecoderFactory.h
	> Author: pzx
	> Created Time: 2019年03月06日 星期三 14时49分26秒
************************************************************************/
#ifndef DECODERFACTORY_H_
#define DECODERFACTORY_H_
#include <unordered_map>
#include <map>
class MediaDecoder;
enum DecoderType
{
    DECODER_TYPE_MIN,
    DECODER_TYPE_FFMPEG_AUDIO,
    DECODER_TYPE_FFMPEG_VIDEO,
    DECODER_TYPE_OPENH264,
    DECODER_TYPE_FDKAAC,
    DECODER_TYPE_MAX
};
class DecoderFactory
{
    public:
    static MediaDecoder* getDecoder(DecoderType type);
    // TODO 避免非必要的编码器初始化
    static int init();
    static int destory();

    private:
    DecoderFactory();
    ~DecoderFactory();
    static std::map<DecoderType, MediaDecoder*> decoders_;
};
#endif  // DECODERFACTORY_H_
