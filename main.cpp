#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "FlvParser.h"
#include "SDLPlayer.h"
#include <thread>

#define USE_FFMPEG 0    // 是否使用ffmpeg解码
using namespace std;

void videoThread(int n)
{
    FlvParser::instance()->decodeH264();
}

void audioThread(int n)
{
    FlvParser::instance()->decodeAAC();
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		cout << "FlvParser [input flv]" << endl;
		return 0;
	}

    int ret = FlvParser::instance()->init(argv[1]);
    if(ret < 0)
    {
        printf("flvParser init error\n");
        return -1;
    }
    // TODO 如果加载整个文件则内存太大了
    FlvParser::instance()->parseFlv();
    SDLPlayer::instance()->init();
    
    // 解析音频线程
    std::thread threadAudio(audioThread, NULL);
    // 解析视频线程
    std::thread threadVideo(videoThread, NULL);
    threadAudio.join();
    threadVideo.join();

}
