/*************************************************************************
	> File Name: SDLPlayer.h
	> Author: pzx
	> Created Time: 2019年02月22日 星期五 10时44分24秒
************************************************************************/

/**
 * 函数调用步骤如下:
 *
 * [初始化]
 * SDL_Init(): 初始化SDL。
 * SDL_CreateWindow(): 创建窗口（Window）。
 * SDL_CreateRenderer(): 基于窗口创建渲染器（Render）。
 * SDL_CreateTexture(): 创建纹理（Texture）。
 *
 * [循环渲染数据]
 * SDL_UpdateTexture(): 设置纹理的数据。
 * SDL_RenderCopy(): 纹理复制给渲染器。
 * SDL_RenderPresent(): 显示。
 *
 */

#include <stdio.h>
#include "SDLPlayer.h"
#include "Clock.h"
#include <time.h>
#include <pthread.h>
#include "FlvParser.h"

uint32_t SDLPlayer::audioLen_ = 0;
unsigned char* SDLPlayer::audioChrunk_ = NULL;
unsigned char* SDLPlayer::audioPos_ = NULL;
uint32_t SDLPlayer::serial_ = 0;
double SDLPlayer::videoTime_ = 0;
double SDLPlayer::audioTime_ = 0;
uint64_t SDLPlayer::ptsDrift_ = 0;
double SDLPlayer::audioClock_ = 0;
double SDLPlayer::videoClock_ = 0;
u_int64_t SDLPlayer::audioDataCount_ = 0;

SDLPlayer::SDLPlayer()
{
    //YUV420P
    bpp_ = 12;
    // TODO 通过metadata初始化
    screenWidth_ = FlvParser::instance()->metadata_->width_;
    screenHeight_ = FlvParser::instance()->metadata_->height_;
    pixelWidth_ = screenWidth_;
    pixelHeight_ = screenHeight_;

    audioSpec_.freq = 44100;
    audioSpec_.format = AUDIO_S16SYS;
    audioSpec_.channels = 2;
    audioSpec_.silence = 0;
    audioSpec_.samples = 1024;
    // 音频的回调函数
    audioSpec_.callback = fillAudio;
    // 用户传递的数据
    //audioSpec_.userdata = ;
}

// 音频的回调函数
void SDLPlayer::fillAudio(void* data, uint8_t* stream, int len)
{
	//SDL 2.0
	SDL_memset(stream, 0, len);
	if(audioLen_ == 0)
    {
			return; 
    }
	len = (len > audioLen_ ? audioLen_ : len);

	SDL_MixAudio(stream, audioPos_, len, SDL_MIX_MAXVOLUME);
	audioPos_ += len; 
	audioLen_ -= len;
    ++serial_;
    audioDataCount_ += len;

    // 更新视频时间
    // 当前的时间
    if(ptsDrift_ == 0)
    {
        // 当前时间
        ptsDrift_ = getTime();
    }
    //音频播放了多长时间
    audioClock_ = (double)(audioDataCount_) / (double)getBytesPersecond(44100, 2);
    printf("audio serial_= %d time = %lf timesys:%ld len:%d threadid:%d audioLen_:%d\n",serial_, audioClock_, getTime(),len, pthread_self(), audioLen_);
}

SDLPlayer::~SDLPlayer()
{    
}

int SDLPlayer::init()
{
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) 
    {
		printf( "Could not initialize SDL - %s\n", SDL_GetError()); 
		return -1;
	} 

	//SDL 2.0 Support for multiple windows
	screen_ = SDL_CreateWindow("Simplest Video Play SDL2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		screenWidth_, screenHeight_, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
	if(!screen_)
    {
		printf("SDL: could not create window - exiting:%s\n",SDL_GetError());  
		return -1;
	}
	sdlRenderer_ = SDL_CreateRenderer(screen_, -1, 0);  

	//IYUV: Y + U + V  (3 planes)
	//YV12: Y + V + U  (3 planes)
	pixformat_= SDL_PIXELFORMAT_IYUV;  

	sdlTexture_ = SDL_CreateTexture(sdlRenderer_,pixformat_, SDL_TEXTUREACCESS_STREAMING,pixelWidth_,pixelHeight_);

    // audio
    if(SDL_OpenAudio(&audioSpec_, NULL) < 0)
    {
        printf("SDL_OpenAudio error\n");
        return -1;
    }
    SDL_PauseAudio(0);
    return 0;
}

int SDLPlayer::destory()
{
    // TODO SLD_CLOSE.....
}

int SDLPlayer::play(AVFrame* frame, int pts, int width, int height)
{
    // 重点：计算当前正在播放的音频的pts,audioClock_包含了缓冲区的数据
    // 当前播放的音频的时间为
    // 视频和音频的时间差
    int delay = pts * 1000 - audioClock_*1000000;
    //int deley = getTime() - ptsDrift_;
    if(delay > 0)
    {
        struct timespec time;
        struct timespec remain;
        time.tv_sec = delay / 1000000;
        time.tv_nsec = (delay % 1000000) * 1000;
        int ret = nanosleep(&time, &remain);
    }
    printf("video pts:%d audioclock:%lf deleay:%d thread_id:%d\n", pts, audioClock_,delay, pthread_self());
    
    SDL_UpdateYUVTexture(sdlTexture_, &sdlRect_,
        frame->data[0], frame->linesize[0],
        frame->data[1], frame->linesize[1],
        frame->data[2], frame->linesize[2]);
        

    //FIX: If window is resize
    sdlRect_.x = 0;
    sdlRect_.y = 0;
    sdlRect_.w = screenWidth_;
    sdlRect_.h = screenHeight_;

    SDL_RenderClear( sdlRenderer_ );
    SDL_RenderCopy( sdlRenderer_, sdlTexture_, NULL, &sdlRect_);
    SDL_RenderPresent( sdlRenderer_ );
    //Delay 40ms
	return 0;
}

int SDLPlayer::play(unsigned char* buf, int pts, int width, int height)
{
    // 当前播放的音频的时间为
    int delay = pts * 1000 - audioClock_*1000000;
    //int deley = getTime() - ptsDrift_;
    if(delay > 0)
    {
        struct timespec time;
        struct timespec remain;
        time.tv_sec = delay / 1000000;
        time.tv_nsec = (delay % 1000000) * 1000;
        int ret = nanosleep(&time, &remain);
    }
    SDL_UpdateTexture( sdlTexture_, NULL, buf, screenWidth_);
    // FIX: If window is resize
    // 测试linux下并没有卵用
    sdlRect_.x = 0;
    sdlRect_.y = 0;
    sdlRect_.w = width;
    sdlRect_.h = height;

    SDL_RenderClear(sdlRenderer_);
    SDL_RenderCopy(sdlRenderer_, sdlTexture_, NULL, &sdlRect_);
    SDL_RenderPresent(sdlRenderer_);
	return 0;
}


int SDLPlayer::playAudio(unsigned char* data, uint32_t size)
{
    audioChrunk_ = data;
    audioLen_ = size;
    audioPos_ = audioChrunk_;
    while(audioLen_ > 0)
        SDL_Delay(1);
    return 0;
}
