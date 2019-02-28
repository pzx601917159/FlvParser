#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include "FlvParser.h"
#include "SDLPlayer.h"
#include <thread>
//全局播放器
SDLPlayer g_sdlPlayer;

using namespace std;

void Process(fstream &fin, const char *filename);

void videoThread(void* args)
{
    FlvParser* parser = (FlvParser*)args;
    parser->parseH264();
}

void audioThread(void* args)
{
    FlvParser* parser = (FlvParser*)args;
    parser->parseAAC();
}

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		cout << "FlvParser [input flv] [output flv]" << endl;
		return 0;
	}

	fstream fin;
	fin.open(argv[1], ios_base::in | ios_base::binary);
	if (!fin)
		return 0;

	Process(fin, argv[2]);

	fin.close();

	return 1;
}

void Process(fstream &fin, const char *filename)
{
	FlvParser parser;
    g_sdlPlayer.init();

    // 4M的buf
	int nBufSize = 4 * 1024 * 1024;
	int nFlvPos = 0;
	unsigned char *pBuf, *pBak;
	pBuf = new unsigned char[nBufSize];
	pBak = new unsigned char[nBufSize];

	while (1)
	{
		int nReadNum = 0;
		int nUsedLen = 0;
        // 读数据
		fin.read((char *)pBuf + nFlvPos, nBufSize - nFlvPos);
		nReadNum = fin.gcount();
		if (nReadNum == 0)
			break;
		nFlvPos += nReadNum;

		parser.Parse(pBuf, nFlvPos, nUsedLen);
		if (nFlvPos != nUsedLen)
		{
			memcpy(pBak, pBuf + nUsedLen, nFlvPos - nUsedLen);
			memcpy(pBuf, pBak, nFlvPos - nUsedLen);
		}
		nFlvPos -= nUsedLen;
	}
	//parser.PrintInfo();
	//parser.DumpH264("parser.264");
	//parser.DumpAAC("parser.aac");

	//dump into flv
	//parser.DumpFlv(filename);
    //parser.parseH264();
    //parser.parseAAC();
    //parser.PrintInfo();
    std::thread threadAudio(audioThread, &parser);
    std::thread threadVideo(videoThread, &parser);
    threadAudio.join();
    threadVideo.join();

	delete []pBak;
	//delete []pBuf;
}
