/*************************************************************************
	> File Name: DecoderFactory.h
	> Author: pzx
	> Created Time: 2019年03月06日 星期三 14时49分26秒
************************************************************************/
#include "DecoderFactory.h"
#include "AACDecoder.h"
#include "FdkAACDecoder.h"
#include "H264Decoder.h"
#include "OpenH264Decoder.h"
#include <map>

std::map<DecoderType, MediaDecoder*> DecoderFactory::decoders_;

DecoderFactory::DecoderFactory()
{
}

DecoderFactory::~DecoderFactory()
{
}

// 初始化
int DecoderFactory::init()
{
    MediaDecoder* aacDecoder = new AACDecoder();
    aacDecoder->init();
    decoders_[DECODER_TYPE_FFMPEG_AUDIO] =  aacDecoder;
    MediaDecoder* h264Decoder = new H264Decoder();
    h264Decoder->init();
    decoders_[DECODER_TYPE_FFMPEG_VIDEO] =  h264Decoder;
    MediaDecoder* fdkAacDecoder = new FdkAACDecoder();
    fdkAacDecoder->init();
    decoders_[DECODER_TYPE_FDKAAC] =  fdkAacDecoder;
    MediaDecoder* openH264Decoder = new OpenH264Decoder();
    openH264Decoder->init();
    decoders_[DECODER_TYPE_OPENH264] =  openH264Decoder;
    return 0;
}

// 时放资源
int DecoderFactory::destory()
{
    for(auto it = decoders_.begin(); it != decoders_.end();)
    {
        it->second->destory();
        delete it->second;
        decoders_.erase(it++);
    }
}

MediaDecoder* DecoderFactory::getDecoder(DecoderType type)
{
    if(decoders_.find(type) !=  decoders_.end())
    {
        return decoders_[type];
    }
    return NULL;
}
