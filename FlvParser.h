#ifndef FLVPARSER_H
#define FLVPARSER_H
#include <vector>
#include "H264Decoder.h"
#include "OpenH264Decoder.h"
#include "AACDecoder.h"
#include "FlvFile.h"
#include "FdkAACDecoder.h"
using namespace std;
static const unsigned int h264StartCode = 0x01000000;

class FlvParser
{
public:
	FlvParser();
	virtual ~FlvParser();

	int Parse(unsigned char *pBuf, int nBufSize, int &nUsedLen);
	int PrintInfo();
    // 保存h264
	int DumpH264(const std::string &path);
    // 保存aac
	int DumpAAC(const std::string &path);
    // 保存flv
	int DumpFlv(const std::string &path);
    // 解析h264
    int parseH264();
    // 解析aac
    int parseAAC();

private:

	struct FlvStat
	{
		int nMetaNum_;
        int nVideoNum_;
        int nAudioNum_;
		int nMaxTimeStamp_;
		int nLengthSize_;

		FlvStat() : nMetaNum_(0), nVideoNum_(0), nAudioNum_(0), nMaxTimeStamp_(0), nLengthSize_(0)
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
    // 视频包
    vector<AVFrame *> videoFrame_;
    // 音频包
    vector<AVFrame *> audioFrame_;
public:
	// H.264
	int nalUnitLength_;
    //OpenH264Decoder h264Decoder_;
    H264Decoder h264Decoder_;
    AACDecoder aacParser_;
    //FdkAACDecoder aacParser_;
};

#endif // FLVPARSER_H
