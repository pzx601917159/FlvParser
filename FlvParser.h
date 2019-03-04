#ifndef FLVPARSER_H
#define FLVPARSER_H
#include <vector>
#include "H264Decoder.h"
#include "OpenH264Decoder.h"
#include "AACDecoder.h"
#include "FlvFile.h"
#include "FdkAACDecoder.h"
#include <string>
#include <fstream>
using namespace std;
static const unsigned int h264StartCode = 0x01000000;

class FlvParser
{
private:
	FlvParser();
	virtual ~FlvParser();
public:
    static FlvParser* instance()
    {
        static FlvParser flvParser;
        return &flvParser;
    }

    int init(std::string fileName);
    int destory();

	int parseFlv();
	int parse(unsigned char *pBuf, int nBufSize, int &nUsedLen);
	int PrintInfo();
    // 保存h264
	int DumpH264(const std::string &path);
    // 保存aac
	int DumpAAC(const std::string &path);
    // 保存flv
	int DumpFlv(const std::string &path);
    // 解析h264
    int decodeH264();
    // 解析aac
    int decodeAAC();

private:

	struct FlvStat
	{
		int metaNum_;
        int videoNum_;
        int audioNum_;
		int maxTimeStamp_;
		int lengthSize_;

		FlvStat() : metaNum_(0), videoNum_(0), audioNum_(0), maxTimeStamp_(0), lengthSize_(0)
        {
        }
		~FlvStat() 
        {
        }
	};
	
private:
    // 创建flvheader
	FlvHeader *CreateFlvHeader(unsigned char *pBuf);
    // 销毁flvheader
	int DestroyFlvHeader(FlvHeader *pHeader);
    // 创建tag
	Tag *CreateTag(unsigned char *pBuf, int nLeftLen);
    // 销毁tag
	int DestroyTag(Tag *pTag);
	int Stat();
	int StatVideo(Tag *pTag);
	int IsUserDataTag(Tag *pTag);

private:
	FlvHeader* flvHeader_;
	vector<Tag *> vpTag_;
	FlvStat sStat_;
    // 关键帧才能seek
public:
    vector<int> seekPos_;
	// H.264
	int nalUnitLength_;
#ifdef USE_FFMPEG
    H264Decoder h264Decoder_;
    //FdkAACDecoder aacDecoder_;
    AACDecoder aacDecoder_;
    // 解码后的音频帧
    std::vector<AVFrame*> videoFrames_;
    // 解码后的视频帧
    std::vector<AVFrame*> audioFrames_;
#else
    OpenH264Decoder h264Decoder_;
    //H264Decoder h264Decoder_;
    //AACDecoder aacDecoder_;
    FdkAACDecoder aacDecoder_;
    // 解码后的音频帧
    std::vector<char*> videoFrames_;
    // 解码后的视频帧
    std::vector<char*> audioFrames_;
#endif
    // metadata
    ScriptTag* metadata_;
    fstream fs_;
    std::string fileName_;
};

#endif // FLVPARSER_H
