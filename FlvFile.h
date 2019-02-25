/*************************************************************************
	> File Name: FlvFile.h
	> Author: pzx
	> Created Time: 2019年02月21日 星期四 09时58分50秒
************************************************************************/
#ifndef __FLVFILE_H__
#define __FLVFILE_H__
#include <vector>
#include <inttypes.h>
#include <iostream>

class FlvParser;
enum AVCPacketType
{
    AVC_PACKET_TYPE_SEQUENCE_HEADER,
    AVC_PACKET_TYPE_NALU,
    AVC_PACKET_TYPE_SEQUENCE_END
};
enum FrameType
{
    FRAME_TYPE_KEY = 1,     // AVC
    FRAME_TYPE_INTER,   // AVC
    FRAME_TYPE_DI,      // H.263
    FRAME_TYPE_GK,      // SERVER
    FRAME_TYPE_VIDEO_INFO
};

enum CodecId
{
    CODEC_ID_JPEG = 1,
    CODEC_ID_H263,
    CODEC_ID_SCREEN,
    CODEC_ID_VP6,
    CODEC_ID_VP6A,
    CODEC_ID_SCREEN2,
    CODEC_ID_AVC
};
// tag类型
enum TagType
{
    TAG_TYPE_AUDIO = 0X08,
    TAG_TYPE_VIDEO = 0X09,
    TAG_TYPE_SCRIPT = 0X12
};

enum SoundFormat
{
    AUDIO_FORMAT_LINEAR_PCM_PE,  // linear PCM, platform endian
    AUDIO_FORMAT_ADPCM,
    AUDIO_FORMAT_MP3,
    AUDIO_FORMAT_LINEAR_PCM_LE, // linear PCM, little endian
    AUDIO_FORMAT_NELLY_16K,
    AUDIO_FORMAT_NELLY_8K,
    AUDIO_FORMAT_NELLY,
    AUDIO_FORMAT_G711A_PCM,
    AUDIO_FORMAT_G711MU_PCM,
    AUDIO_FORMAT_RESERVED,
    AUDIO_FORMAT_AAC,
    AUDIO_FORMAT_SPEEX,
    AUDIO_FORMAT_MP3_8K,
    AUDIO_FORMAT_DSS
};

enum SoundRate
{
    SOUND_RATE_5P5K, // 5.5K
    SOUND_RATE_11K,
    SOUND_RATE_22K,
    SOUND_RATE_44K
};

enum SoundSize
{
    SOUND_SIZE_8BIT,
    SOUND_SIZE_16BIT
};

enum SoundType
{
    SOUND_TYPE_MONO,
    SOUND_TYPE_STEREO
};

enum AACPacketType
{
    AAC_PACKET_TYPE_AAC_SEQUENCE_HEADER,    // aac sequence header
    AAC_PACKET_TYPE_AAC_RAW     // 原始数据
};

struct BinaryData
{
    unsigned char* data_;
    uint32_t size_;
};

// tag头
class TagHeader
{
    public:
    TagType tagType_;       // 1byte
    uint32_t dataSize_;     // 3byte
    uint32_t timeStamp_;    // 3byte
    uint8_t timeStampEx_;  // 1byte
    uint32_t streamId_;     // 3byte
    uint32_t totalTimeStamp_;    // calc
    // tagType不初始化
    TagHeader():dataSize_(0),timeStamp_(0),timeStampEx_(0),streamId_(0),totalTimeStamp_(0)
    {
    }
    ~TagHeader()
    {
    }
};

// flv tag
class Tag
{
    public:
    Tag(): prevTagLen_(0)
    {
    }
    void init(TagHeader* header, unsigned char* data, uint32_t size);
    
    TagHeader tagHeader_;   // tagheader
    BinaryData tagHeaderData_; // tagheader数据
    BinaryData tagData_;       // tagdata
    BinaryData mediaData_;     // mediadata
    uint32_t prevTagLen_;   // 上一个tag的长度
};

class VideoTag:public Tag
{
    public:
    VideoTag(TagHeader* header, unsigned char* data, uint32_t size, FlvParser* parser);
    FrameType frameType_;
    CodecId codecId_;
    // h264
    AVCPacketType avcPacketType_; // 1byte
    uint32_t compositionTime_;  // 3byte

	int ParseH264Tag(FlvParser *pParser);   // 解析h264tag
	int ParseH264Configuration(FlvParser *pParser, unsigned char *pTagData);// 解析h264配置
	int ParseNalu(FlvParser *pParser, unsigned char *pTagData); // 解析h264 NALU
};

class AudioTag:public Tag
{
    public:
    AudioTag(TagHeader* header, unsigned char* data, uint32_t size, FlvParser* parser);
    int parseAACTag(FlvParser* parser);
	int parseAudioSpecificConfig(FlvParser *pParser, unsigned char *pTagData);
	int parseRawAAC(FlvParser *pParser, unsigned char *pTagData);
    SoundFormat soundFormat_;   // 4bit 音频格式
    SoundRate soundRate_;       // 2bit采样率
    SoundSize soundSize_;       // 1bit soundsize
    SoundType soundType_;       // 1bit soundtype

    // aac
    AACPacketType aacPacketType_;
    static int aacProfile_;
    static int sampleRateIndex_;
    static int channelConfig_;
};

class ScriptTag:public Tag
{
    public:
    ScriptTag(TagHeader* header, unsigned char* data, uint32_t size, FlvParser* parser);
};

// flv文件头
class FlvHeader
{
    public:
    char* signature_;    // "FLV" (0X46 0X4C 0X66) 3byte
    uint8_t version_;    // 1byte 
    uint8_t flags_;      // 1byte 前五位保留必须未0 第六位表示是否存在音频tag 第七位保留必须为0 第八为表示是否存在视频tag
    bool haveVideo_;
    bool haveAudio_;
    uint32_t headerSize_;// header的总长度，版本1为9
    unsigned char* data_;
};

// flv文件
class FlvFile
{
    public:
    private:
    FlvHeader* flvHeader_;          // 文件头
    std::vector<Tag*> flvTags_;  // tag
};
#endif // __FLVFILE_H__
