/*************************************************************************
	> File Name: FlvFile.cpp
	> Author: pzx
	> Created Time: 2019年02月21日 星期四 15时42分31秒
************************************************************************/
#include "FlvFile.h"
#include "FlvParser.h"
#include "ByteUtil.h"
#include "Amf.h"
#include <iostream>
using namespace std;

int AudioTag::aacProfile_;
int AudioTag::sampleRateIndex_;
int AudioTag::channelConfig_;

void Tag::init(TagHeader *pHeader, unsigned char *pBuf,uint32_t nLeftLen)
{
	memcpy(&tagHeader_, pHeader, sizeof(TagHeader));

	tagHeaderData_.data_ = new unsigned char[11];
	memcpy(tagHeaderData_.data_, pBuf, 11);

	tagData_.data_ = new unsigned char[tagHeader_.dataSize_];
	memcpy(tagData_.data_, pBuf + 11, tagHeader_.dataSize_);
}

AudioTag::AudioTag(TagHeader *pHeader, unsigned char *pBuf, uint32_t nLeftLen, FlvParser *pParser)
{
    //printf("audiotag\n");
	init(pHeader, pBuf, nLeftLen);

	unsigned char *pd = tagData_.data_;
	soundFormat_ = (SoundFormat)((pd[0] & 0xf0) >> 4);
	soundRate_ = (SoundRate)((pd[0] & 0x0c) >> 2);
	soundSize_ = (SoundSize)((pd[0] & 0x02) >> 1);
	soundType_ = (SoundType)(pd[0] & 0x01);
	if (soundFormat_ == 10) // AAC
	{
        //解析aactag
		parseAACTag(pParser);
	}
}

int AudioTag::parseAACTag(FlvParser *pParser)
{
	unsigned char *pd = tagData_.data_;
	int nAACPacketType = pd[1];
    dts_ = tagHeader_.totalTimeStamp_;
    pts_ = dts_;

	if (nAACPacketType == 0)
	{
        //解析aac header
		parseAudioSpecificConfig(pParser, pd);
	}
	else if (nAACPacketType == 1)
	{
		parseRawAAC(pParser, pd);
	}
	else
	{

	}

	return 1;
}

int AudioTag::parseAudioSpecificConfig(FlvParser *pParser, unsigned char *pTagData)
{
	unsigned char *pd = tagData_.data_;

    dts_ = tagHeader_.totalTimeStamp_;
    pts_ = dts_;

	aacProfile_ = ((pd[2]&0xf8)>>3) - 1;
	sampleRateIndex_ = ((pd[2]&0x07)<<1) | (pd[3]>>7);
	channelConfig_ = (pd[3]>>3) & 0x0f;

	mediaData_.data_ = NULL;
	mediaData_.size_ = 0;

	return 1;
}

int AudioTag::parseRawAAC(FlvParser *pParser, unsigned char *pTagData)
{
	uint64_t bits = 0;
	int dataSize = tagHeader_.dataSize_ - 2;

	WriteU64(bits, 12, 0xFFF);
	WriteU64(bits, 1, 0);
	WriteU64(bits, 2, 0);
	WriteU64(bits, 1, 1);
	WriteU64(bits, 2, aacProfile_);
	WriteU64(bits, 4, sampleRateIndex_);
	WriteU64(bits, 1, 0);
	WriteU64(bits, 3, channelConfig_);
	WriteU64(bits, 1, 0);
	WriteU64(bits, 1, 0);
	WriteU64(bits, 1, 0);
	WriteU64(bits, 1, 0);
	WriteU64(bits, 13, 7 + dataSize);
	WriteU64(bits, 11, 0x7FF);
	WriteU64(bits, 2, 0);

	mediaData_.size_ = 7 + dataSize;
	mediaData_.data_ = new unsigned char[mediaData_.size_];
	unsigned char p64[8];
	p64[0] = (unsigned char)(bits >> 56);
	p64[1] = (unsigned char)(bits >> 48);
	p64[2] = (unsigned char)(bits >> 40);
	p64[3] = (unsigned char)(bits >> 32);
	p64[4] = (unsigned char)(bits >> 24);
	p64[5] = (unsigned char)(bits >> 16);
	p64[6] = (unsigned char)(bits >> 8);
	p64[7] = (unsigned char)(bits);

	memcpy(mediaData_.data_, p64+1, 7);
	memcpy(mediaData_.data_ + 7, pTagData + 2, dataSize);

	return 1;
}

int VideoTag::ParseH264Tag(FlvParser *pParser)
{
    //VIDEO TAG:
    //  frametype:4bit codecid:4bit avcpackettype:1byte compositiontime:3byte
    //  总共 5byte
	unsigned char *pd = tagData_.data_;
    // 3字节
	avcPacketType_ = (AVCPacketType)pd[1];
    // 占三个字节
	compositionTime_ = ShowU24(pd + 2);

    dts_ = tagHeader_.totalTimeStamp_;
    pts_ = dts_ + compositionTime_;
    if(frameType_ == FRAME_TYPE_KEY)
    {
        FlvParser::instance()->seekPos_.push_back(pts_);
    }

	if (avcPacketType_ == AVC_PACKET_TYPE_SEQUENCE_HEADER)
	{
		ParseH264Configuration(pParser, pd);
	}
	else if (avcPacketType_ == AVC_PACKET_TYPE_NALU)
	{
        //printf("parsenalulen\n");
		ParseNalu(pParser, pd);
	}
	else if(avcPacketType_ == AVC_PACKET_TYPE_SEQUENCE_END)
	{
        //printf("xxxxxxavcpacket_type:%d\n", avcPacketType_);
	}
	return 1;
}

int VideoTag::ParseH264Configuration(FlvParser *pParser, unsigned char *pTagData)
{
    // 从6byte开始为avcheader的数据
	unsigned char *pd = pTagData;
    // 表示NALU length的长度所占的字节数
	pParser->nalUnitLength_ = (pd[9] & 0x03) + 1;
    printf("nalu length:%d\n", pParser->nalUnitLength_);

	int sps_size = 0;
    int pps_size = 0;
    int sps_num = 0;
    int pps_num = 0;
    //pps sps都只有一个
    sps_num = ShowU8(pd + 10) & 0x1f;
    cout << "sps_num:" << sps_num << endl;
	sps_size = ShowU16(pd + 11);
    //sps_size = FlvParser::show()
    printf("sps_size:%d\n",sps_size);
    pps_num = ShowU8(pd + 11 + (2 + sps_size));
    printf("pps num:%d\n", pps_num);
	pps_size = ShowU16(pd + 11 + (2 + sps_size) + 1);
    printf("pps size:%d\n",pps_size);
    // 4字节的startcode	
	mediaData_.size_ = 4 + sps_size + 4 + pps_size;
	mediaData_.data_ = new unsigned char[mediaData_.size_];
	memcpy(mediaData_.data_, &h264StartCode, 4);
	memcpy(mediaData_.data_ + 4, pd + 11 + 2, sps_size);
	memcpy(mediaData_.data_ + 4 + sps_size, &h264StartCode, 4);
	memcpy(mediaData_.data_ + 4 + sps_size + 4, pd + 11 + 2 + sps_size + 2 + 1, pps_size);
	return 1;
}

// videotag
VideoTag::VideoTag(TagHeader *pHeader, unsigned char *pBuf, uint32_t nLeftLen, FlvParser *pParser)
{
    //printf("videotag\n");
	init(pHeader, pBuf, nLeftLen);

	unsigned char *pd = tagData_.data_;
    // 帧类型
	frameType_ = FrameType((pd[0] & 0xf0) >> 4);
    // 编码id
	codecId_ = CodecId(pd[0] & 0x0f);
	if (tagHeader_.tagType_ == TAG_TYPE_VIDEO && codecId_ == CODEC_ID_AVC)
	{
		ParseH264Tag(pParser);
	}
}

// 解析NALU
int VideoTag::ParseNalu(FlvParser *pParser, unsigned char *pTagData)
{
	unsigned char *pd = pTagData;
	int nOffset = 0;

	mediaData_.data_ = new unsigned char[tagHeader_.dataSize_+10];
	mediaData_.size_ = 0;

	nOffset = 5;
	while (1)
	{
		if (nOffset >= tagHeader_.dataSize_)
			break;

		int nNaluLen;
        //4个字节
		switch (pParser->nalUnitLength_)
		{
		case 4:
			nNaluLen = ShowU32(pd + nOffset);
			break;
		case 3:
			nNaluLen = ShowU24(pd + nOffset);
			break;
		case 2:
			nNaluLen = ShowU16(pd + nOffset);
			break;
		default:
			nNaluLen = ShowU8(pd + nOffset);
		}
		memcpy(mediaData_.data_ + mediaData_.size_, &h264StartCode, 4);
		memcpy(mediaData_.data_ + mediaData_.size_ + 4, pd + nOffset + pParser->nalUnitLength_, nNaluLen);
		mediaData_.size_ += (4 + nNaluLen);
		nOffset += (pParser->nalUnitLength_ + nNaluLen);
	}

	return 1;
}

ScriptTag::ScriptTag(TagHeader *pHeader, unsigned char *pBuf, uint32_t nLeftLen, FlvParser *pParser)
{
    printf("script tag\n");
	init(pHeader, pBuf, nLeftLen);

	unsigned char *pd = tagData_.data_;
    // 开始解析tag
    int i=0;
    int type = pd[i++];
    printf("amf type:%d\n",type);
    std::string key;
    std::string value;
    if(type == AMF_DATA_TYPE_STRING)
    {
        key = amfGetString(pd + i,nLeftLen-i);
    }
    else
    {
        return;
    }
    i += 2;
    i += key.length();
    std::cout << "str:" << key << std::endl;

    // 解析metadata
    if(key == "onMetaData")
    {
        parseMetadata(pd + i, nLeftLen -i, "onMetadata");
    }
    printf("Metdata:width:%lf, height:%lf\n",width_, height_);
}

// 解析metadata
int ScriptTag::parseMetadata(unsigned char* data, uint32_t size, std::string key)
{
    std::string valueString;
    double valueNum;
    int i = 0;
    int valueType = data[i++];
    int valueLen = 0;
    switch(valueType)
    {
        case AMF_DATA_TYPE_MIXEDARRAY:
        {
            int arrLen = ShowU32(data + i);
            i += 4;
            for(int j = 0; j < arrLen; ++j)
            {
                if(j != 0)
                {
                    ++i;
                }
                key = amfGetString(data + i, size-i);
                i += 2;
                i += key.length();
                valueLen = parseMetadata(data + i, size - i, key);
                i += valueLen;
            }
            break;
        }
        case AMF_DATA_TYPE_STRING:
        {
            valueString = amfGetString(data + i, size -i);
            i += 2;
            i += valueString.length();
            valueLen = valueString.length() + 2;
            break;
        }
        case AMF_DATA_TYPE_BOOL:
        {
            valueNum = data[i++];
            valueLen = 1;
            break;
        }
        case AMF_DATA_TYPE_NUMBER:
        {
            valueNum = ShowDouble(data + i);
            cout << "value:" << valueNum << endl;
            valueLen = 8;
            i += 8;
            break;
        }
        default:
        break;
    }
    if(key == "duration")
    {
        duration_ = valueNum;
    }
    else if(key == "width")
    {
        width_ = valueNum;
    }
    else if(key == "height")
    {
        height_ = valueNum;
    }
    else if(key == "videodatarate")
    {
        videoDataRate_ = valueNum;
    }
    else if(key == "framerate")
    {
        framerate_ = valueNum;
    }
    else if(key == "videocodecid")
    {
        videoCodecId_ = valueNum;
    }
    else if(key == "audiosamplerate")
    {
        audioSampleRate_ = valueNum;
    }
    else if(key == "audiosamplesize")
    {
        audioSampleSize_ = valueNum;
    }
    else if(key == "stereo")
    {
        stereo_ = valueNum;
    }
    else if(key == "audiocodecid")
    {
        audioCodecId_ = valueNum;
    }
    else if(key == "filesize")
    {
        fileSize_ = valueNum;
    }
    return valueLen;
}


