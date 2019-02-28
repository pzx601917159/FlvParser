#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <iostream>
#include <fstream>

#include "FlvParser.h"
#include "ByteUtil.h"

using namespace std;

#define CheckBuffer(x) { if ((nBufSize-nOffset)<(x)) { nUsedLen = nOffset; return 0;} }
#define TAG_HEADER_SIZE 15
#define FLV_HEADER_SIZE 9

#define VIDEO_CODEC_ID_JPEG 1
#define VIDEO_CODEC_ID_H263 2
#define VIDEO_CODEC_ID_SCREEN 3
#define VIDEO_CODEC_ID_VP6 4
#define VIDEO_CODEC_ID_VPA 5
#define VIDEO_CODEC_ID_SCREEN2 6
#define VIDEO_CODEC_ID_AVC 7 // H264

#define VIDEO_FRAME_TYPE_KEYFRAME 1
#define VIDEO_FRAME_TYPE_INTERFRAME 2

#define AUDIO_CODEC_ID_ADPCM 1
#define AUDIO_CODEC_ID_MP3 2
#define AUDIO_CODEC_ID_LINEPCM 3
#define AUDIO_CODEC_ID_AAC 10

#define AVC_PACKET_TYPE_SEQUENCE_HEADER 0
#define AVC_PACKET_TYPE_AVC_NALU 1
#define AVC_PACKET_TYPE_SEQUENCE_END 2

FlvParser::FlvParser()
{
    flvHeader_ = NULL;
    // h264解析器初始化
    h264Decoder_.init();
    aacParser_.init();
}

FlvParser::~FlvParser()
{
	for (int i = 0; i < vpTag_.size(); i++)
	{
		DestroyTag(vpTag_[i]);
		delete vpTag_[i];
	}
}
/*
 * 参数一：数据
 * 参数二：数据的大小
 * 参数三：已经解析的数据
 */
int FlvParser::Parse(unsigned char *pBuf, int nBufSize, int &nUsedLen)
{
	int nOffset = 0;
    // 初始化flvheader
	if (flvHeader_ == NULL)
	{
		CheckBuffer(FLV_HEADER_SIZE);
        // 保存flvheader
		flvHeader_ = CreateFlvHeader(pBuf+nOffset);
		nOffset += flvHeader_->headerSize_;
	}

	while (1)
	{
		CheckBuffer(TAG_HEADER_SIZE);
		int nPrevSize = ShowU32(pBuf + nOffset);
		nOffset += 4;
		Tag *pTag = CreateTag(pBuf + nOffset, nBufSize-nOffset);
		if (pTag == NULL)
		{
			nOffset -= 4;
			break;
		}
		nOffset += (11 + pTag->tagHeader_.dataSize_);
        // 保存tag
		vpTag_.push_back(pTag);
	}

	nUsedLen = nOffset;
	return 0;
}

int FlvParser::PrintInfo()
{
	Stat();

	cout << "vnum: " << sStat_.nVideoNum_ << " , anum: " << sStat_.nAudioNum_ << " , mnum: " << sStat_.nMetaNum_ << endl;
	cout << "maxTimeStamp: " << sStat_.nMaxTimeStamp_ << " ,nLengthSize: " << sStat_.nLengthSize_ << endl;
    cout << "size:" << vpTag_.size() << endl;
	return 1;
}

int FlvParser::DumpH264(const std::string &path)
{
	fstream f;
	f.open(path.c_str(), ios_base::out|ios_base::binary);

	vector<Tag *>::iterator it_tag;
	for (it_tag = vpTag_.begin(); it_tag != vpTag_.end(); it_tag++)
	{
		if ((*it_tag)->tagHeader_.tagType_ != TAG_TYPE_VIDEO)
			continue;

		f.write((char *)(*it_tag)->mediaData_.data_, (*it_tag)->mediaData_.size_);
	}
	f.close();

	return 1;
}

int FlvParser::DumpAAC(const std::string &path)
{
	fstream f;
	f.open(path.c_str(), ios_base::out | ios_base::binary);

	vector<Tag *>::iterator it_tag;
	for (it_tag = vpTag_.begin(); it_tag != vpTag_.end(); it_tag++)
	{
		if ((*it_tag)->tagHeader_.tagType_ != TAG_TYPE_AUDIO)
			continue;

		AudioTag *pAudioTag = (AudioTag *)(*it_tag);
		if (pAudioTag->soundFormat_ != 10)
			continue;

		if (pAudioTag->mediaData_.size_ != 0)
			f.write((char *)(*it_tag)->mediaData_.data_, (*it_tag)->mediaData_.size_);
	}
	f.close();

	return 1;
}

int FlvParser::DumpFlv(const std::string &path)
{
    fstream f;
    f.open(path.c_str(), ios_base::out | ios_base::binary);

    // write flv-header
    f.write((char *)flvHeader_->data_, flvHeader_->headerSize_);
    unsigned int nLastTagSize = 0;


    // write flv-tag
    vector<Tag *>::iterator it_tag;
    for (it_tag = vpTag_.begin(); it_tag < vpTag_.end(); it_tag++)
    {
        unsigned int nn = WriteU32(nLastTagSize);
        f.write((char *)&nn, 4);

        //check duplicate start code
        if ((*it_tag)->tagHeader_.tagType_ ==TAG_TYPE_AUDIO && *((*it_tag)->tagData_.data_ + 1) == 0x01) {
            bool duplicate = false;
            unsigned char *pStartCode = (*it_tag)->tagData_.data_ + 5 + nalUnitLength_;
            //printf("tagsize=%d\n",(*it_tag)->header_.nDataSize_);
            unsigned nalu_len = 0;
            unsigned char *p_nalu_len=(unsigned char *)&nalu_len;
            switch (nalUnitLength_) 
            {
            case 4:
                nalu_len = ShowU32((*it_tag)->tagData_.data_ + 5);
                break;
            case 3:
                nalu_len = ShowU24((*it_tag)->tagData_.data_ + 5);
                break;
            case 2:
                nalu_len = ShowU16((*it_tag)->tagData_.data_ + 5);
                break;
            default:
                nalu_len = ShowU8((*it_tag)->tagData_.data_ + 5);
                break;
            }
            /*
            printf("nalu_len=%u\n",nalu_len);
            printf("%x,%x,%x,%x,%x,%x,%x,%x,%x\n",(*it_tag)->pTagData_[5],(*it_tag)->pTagData_[6],
                    (*it_tag)->pTagData_[7],(*it_tag)->pTagData_[8],(*it_tag)->_pTagData[9],
                    (*it_tag)->pTagData_[10],(*it_tag)->pTagData_[11],(*it_tag)->_pTagData[12],
                    (*it_tag)->pTagData_[13]);
            */

            unsigned char *pStartCodeRecord = pStartCode;
            int i;
            for (i = 0; i < (*it_tag)->tagHeader_.dataSize_ - 5 - nalUnitLength_ - 4; ++i) {
                if (pStartCode[i] == 0x00 && pStartCode[i+1] == 0x00 && pStartCode[i+2] == 0x00 &&
                        pStartCode[i+3] == 0x01) {
                    if (pStartCode[i+4] == 0x67) {
                        //printf("duplicate sps found!\n");
                        i += 4;
                        continue;
                    }
                    else if (pStartCode[i+4] == 0x68) {
                        //printf("duplicate pps found!\n");
                        i += 4;
                        continue;
                    }
                    else if (pStartCode[i+4] == 0x06) {
                        //printf("duplicate sei found!\n");
                        i += 4;
                        continue;
                    }
                    else {
                        i += 4;
                        //printf("offset=%d\n",i);
                        duplicate = true;
                        break;
                    }
                }
            }

            if (duplicate) {
                nalu_len -= i;
                (*it_tag)->tagHeader_.dataSize_ -= i;
                unsigned char *p = (unsigned char *)&((*it_tag)->tagHeader_.dataSize_);
                (*it_tag)->tagHeaderData_.data_[1] = p[2];
                (*it_tag)->tagHeaderData_.data_[2] = p[1];
                (*it_tag)->tagHeaderData_.data_[3] = p[0];
                //printf("after,tagsize=%d\n",(int)ShowU24((*it_tag)->pTagHeader_ + 1));
                //printf("%x,%x,%x\n",(*it_tag)->pTagHeader_[1],(*it_tag)->_pTagHeader[2],(*it_tag)->_pTagHeader[3]);

                f.write((char *)(*it_tag)->tagHeaderData_.data_, 11);
                switch (nalUnitLength_) {
                case 4:
                    *((*it_tag)->tagData_.data_ + 5) = p_nalu_len[3];
                    *((*it_tag)->tagData_.data_ + 6) = p_nalu_len[2];
                    *((*it_tag)->tagData_.data_ + 7) = p_nalu_len[1];
                    *((*it_tag)->tagData_.data_ + 8) = p_nalu_len[0];
                    break;
                case 3:
                    *((*it_tag)->tagData_.data_ + 5) = p_nalu_len[2];
                    *((*it_tag)->tagData_.data_ + 6) = p_nalu_len[1];
                    *((*it_tag)->tagData_.data_ + 7) = p_nalu_len[0];
                    break;
                case 2:
                    *((*it_tag)->tagData_.data_ + 5) = p_nalu_len[1];
                    *((*it_tag)->tagData_.data_ + 6) = p_nalu_len[0];
                    break;
                default:
                    *((*it_tag)->tagData_.data_ + 5) = p_nalu_len[0];
                    break;
                }
                //printf("after,nalu_len=%d\n",(int)ShowU32((*it_tag)->pTagData_ + 5));
                f.write((char *)(*it_tag)->tagData_.data_, pStartCode - (*it_tag)->tagData_.data_);
                /*
                printf("%x,%x,%x,%x,%x,%x,%x,%x,%x\n",(*it_tag)->pTagData_[0],(*it_tag)->pTagData_[1],(*it_tag)->_pTagData[2],
                        (*it_tag)->pTagData_[3],(*it_tag)->pTagData_[4],(*it_tag)->_pTagData[5],(*it_tag)->_pTagData[6],
                        (*it_tag)->pTagData_[7],(*it_tag)->pTagData_[8]);
                */
                f.write((char *)pStartCode + i, (*it_tag)->tagHeader_.dataSize_ - (pStartCode - (*it_tag)->tagData_.data_));
                /*
                printf("write size:%d\n", (pStartCode - (*it_tag)->pTagData_) +
                        ((*it_tag)->header_.nDataSize_ - (pStartCode - (*it_tag)->pTagData_)));
                */
            } else {
                f.write((char *)(*it_tag)->tagHeaderData_.data_, 11);
                f.write((char *)(*it_tag)->tagData_.data_, (*it_tag)->tagHeader_.dataSize_);
            }
        } else {
            f.write((char *)(*it_tag)->tagHeaderData_.data_, 11);
            f.write((char *)(*it_tag)->tagData_.data_, (*it_tag)->tagHeader_.dataSize_);
        }

        nLastTagSize = 11 + (*it_tag)->tagHeader_.dataSize_;
    }
    unsigned int nn = WriteU32(nLastTagSize);
    f.write((char *)&nn, 4);

    f.close();

    return 1;
}

//解析h264
int FlvParser::parseH264()
{
    for(auto tag:vpTag_)
    {
        if(tag->tagHeader_.tagType_ == TAG_TYPE_VIDEO)
        {
            int width;
            int height;
            int pixFmt;
            h264Decoder_.decodeFrame(tag->mediaData_.data_, tag->mediaData_.size_, &width, &height, &pixFmt, tag->pts_);
            //printf("width:%d, height:%d, pixFmt:%d\n",width,height,pixFmt);
        }    
    }
}

//解析aac
int FlvParser::parseAAC()
{
    for(auto tag:vpTag_)
    {
        if(tag->tagHeader_.tagType_ == TAG_TYPE_AUDIO)
        {
            int width;
            int height;
            int pixFmt;
            //printf("======decode audio frame");
            aacParser_.decodeFrame(tag->mediaData_.data_, tag->mediaData_.size_, &width, &height, &pixFmt, tag->pts_);
            //printf("width:%d, height:%d, pixFmt:%d\n",width,height,pixFmt);
        }    
    }
}

int FlvParser::Stat()
{
	for (int i = 0; i < vpTag_.size(); i++)
	{
		switch (vpTag_[i]->tagHeader_.tagType_)
		{
		case 0x08:
			sStat_.nAudioNum_++;
			break;
		case 0x09:
			StatVideo(vpTag_[i]);
			break;
		case 0x12:
			sStat_.nMetaNum_++;
			break;
		default:
			;
		}
	}
	return 1;
}

int FlvParser::StatVideo(Tag *pTag)
{
	sStat_.nVideoNum_++;
	sStat_.nMaxTimeStamp_ = pTag->tagHeader_.timeStamp_;

	if (pTag->tagData_.data_[0] == 0x17 && pTag->tagData_.data_[1] == 0x00)
	{
		sStat_.nLengthSize_ = (pTag->tagData_.data_[9] & 0x03) + 1;
	}

	return 1;
}

// 文件头
FlvHeader *FlvParser::CreateFlvHeader(unsigned char *pBuf)
{
	FlvHeader *pHeader = new FlvHeader;
	pHeader->version_ = pBuf[3];
	pHeader->haveAudio_ = (pBuf[4] >> 2) & 0x01;
	pHeader->haveVideo_ = (pBuf[4] >> 0) & 0x01;
	pHeader->headerSize_ = ShowU32(pBuf + 5);

	pHeader->data_ = new unsigned char[pHeader->headerSize_];
    //拷贝包头的数据
	memcpy(pHeader->data_, pBuf, pHeader->headerSize_);
	return pHeader;
}

int FlvParser::DestroyFlvHeader(FlvHeader *header)
{
	if (header == NULL)
		return 0;

	delete header->data_;
	return 1;
}

Tag *FlvParser::CreateTag(unsigned char *pBuf, int nLeftLen)
{
	TagHeader header;
	header.tagType_ = (TagType)ShowU8(pBuf+0);
	header.dataSize_ = ShowU24(pBuf + 1);
	header.timeStamp_ = ShowU24(pBuf + 4);
	header.timeStampEx_ = ShowU8(pBuf + 7);
	header.streamId_ = ShowU24(pBuf + 8);
	header.totalTimeStamp_ = (unsigned int)((header.timeStampEx_ << 24)) + header.timeStamp_;
	//cout << "total TS : " << header.nTotalTimeStamp_ << endl;
	//cout << "nLeftLen : " << nLeftLen << " , nDataSize_ : " << pTag->header.nDataSize << endl;
	if ((header.dataSize_ + 11) > nLeftLen)
	{
		return NULL;
	} 

	Tag *pTag;
	switch (header.tagType_) {
    case TAG_TYPE_VIDEO:
		pTag = new VideoTag(&header, pBuf, nLeftLen, this);
        break;
	case TAG_TYPE_AUDIO:
		pTag = new AudioTag(&header, pBuf, nLeftLen, this);
		break;
    case TAG_TYPE_SCRIPT:
        pTag = new ScriptTag(&header, pBuf, nLeftLen, this);
        break;
	default:
		pTag = new Tag();
		pTag->init(&header, pBuf, nLeftLen);
	}
	
	return pTag;
}

int FlvParser::DestroyTag(Tag *pTag)
{
    /*
	if (pTag->mediaData_.data_ != NULL)
		delete []pTag->mediaData_.data_;
	if (pTag->tagData_.data_!=NULL)
		delete []pTag->tagData_.data_;
	if (pTag->tagHeaderData_.data_ != NULL)
		delete []pTag->tagHeaderData_.data_;
        */

	return 1;
}

