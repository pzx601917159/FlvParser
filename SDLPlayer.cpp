/*************************************************************************
	> File Name: SDLPlayer.h
	> Author: pzx
	> Created Time: 2019年02月22日 星期五 10时44分24秒
************************************************************************/

/**
 * 最简单的SDL2播放视频的例子（SDL2播放RGB/YUV）
 * Simplest Video Play SDL2 (SDL2 play RGB/YUV)
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * 本程序使用SDL2播放RGB/YUV视频像素数据。SDL实际上是对底层绘图
 * API（Direct3D，OpenGL）的封装，使用起来明显简单于直接调用底层
 * API。
 *
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
 * This software plays RGB/YUV raw video data using SDL2.
 * SDL is a wrapper of low-level API (Direct3D, OpenGL).
 * Use SDL is much easier than directly call these low-level API.
 *
 * The process is shown as follows:
 *
 * [Init]
 * SDL_Init(): Init SDL.
 * SDL_CreateWindow(): Create a Window.
 * SDL_CreateRenderer(): Create a Render.
 * SDL_CreateTexture(): Create a Texture.
 *
 * [Loop to Render data]
 * SDL_UpdateTexture(): Set Texture's data.
 * SDL_RenderCopy(): Copy Texture to Render.
 * SDL_RenderPresent(): Show.
 */

#include <stdio.h>
#include "SDLPlayer.h"

int SDLPlayer::thread_exit = 0;
uint32_t SDLPlayer::audioLen_ = 0;
unsigned char* SDLPlayer::audioChrunk_ = NULL;
unsigned char* SDLPlayer::audioPos_ = NULL;

SDLPlayer::SDLPlayer()
{
    //YUV420P
    bpp_ = 12;
    screenWidth_ = 320;
    screenHeight_ = 240;
    pixelWidth_ = 320;
    pixelHeight_ = 240;

    audioSpec_.freq = 44100;
    audioSpec_.format = AUDIO_S16SYS;
    audioSpec_.channels = 2;
    audioSpec_.silence = 0;
    audioSpec_.samples = 1024;
    // 音频的回调函数
    audioSpec_.callback = fillAudio;
    init_ = false;
}

void SDLPlayer::fillAudio(void* data, uint8_t* stream, int len)
{
	//SDL 2.0
	SDL_memset(stream, 0, len);
	if(audioLen_ == 0)
			return; 
	len=(len > audioLen_?audioLen_:len);

	SDL_MixAudio(stream,audioPos_,len,SDL_MIX_MAXVOLUME);
	audioPos_ += len; 
	audioLen_ -= len; 
}

SDLPlayer::~SDLPlayer()
#include "SDLPlayer.h"
{    
}

int SDLPlayer::init()
{
    init_ = true;
    printf("--------------------------\n");
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) 
    {
		printf( "Could not initialize SDL - %s\n", SDL_GetError()); 
		return -1;
	} 

	//SDL 2.0 Support for multiple windows
	screen_ = SDL_CreateWindow("Simplest Video Play SDL2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		screenWidth_, screenHeight_,SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
	if(!screen_) {  
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
}

//Convert RGB24/BGR24 to RGB32/BGR32
//And change Endian if needed
void SDLPlayer::CONVERT_24to32(unsigned char *image_in,unsigned char *image_out,int w,int h)
{
	for(int i =0;i<h;i++)
    {
		for(int j=0;j<w;j++)
        {
			//Big Endian or Small Endian?
			//"ARGB" order:high bit -> low bit.
			//ARGB Format Big Endian (low address save high MSB, here is A) in memory : A|R|G|B
			//ARGB Format Little Endian (low address save low MSB, here is B) in memory : B|G|R|A
			if(SDL_BYTEORDER==SDL_LIL_ENDIAN)
            {
				//Little Endian (x86): R|G|B --> B|G|R|A
				image_out[(i*w+j)*4+0]=image_in[(i*w+j)*3+2];
				image_out[(i*w+j)*4+1]=image_in[(i*w+j)*3+1];
				image_out[(i*w+j)*4+2]=image_in[(i*w+j)*3];
				image_out[(i*w+j)*4+3]='0';
			}
            else
            {
				//Big Endian: R|G|B --> A|R|G|B
				image_out[(i*w+j)*4]='0';
				memcpy(image_out+(i*w+j)*4+1,image_in+(i*w+j)*3,3);
			}
		}
    }
}

int SDLPlayer::refresh_video(void *opaque)
{
	while (thread_exit==0) 
    {
		SDL_Event event;
		event.type = REFRESH_EVENT;
		SDL_PushEvent(&event);
		SDL_Delay(40);
	}
	return 0;
}

int SDLPlayer::play(AVFrame* frame)
{
    printf("%s\n",__FUNCTION__);
    
    SDL_UpdateYUVTexture(sdlTexture_, &sdlRect_,
        frame->data[0], frame->linesize[0],
        frame->data[1], frame->linesize[1],
        frame->data[2], frame->linesize[2]);
    printf(" %d %d %d\n", frame->linesize[0], frame->linesize[1], frame->linesize[2]);
        

    //SDL_UpdateTexture( sdlTexture_, NULL, frame->data[0], frame->linesize[0]);
    //FIX: If window is resize
    sdlRect_.x = 0;
    sdlRect_.y = 0;
    sdlRect_.w = screenWidth_;
    sdlRect_.h = screenHeight_;

    SDL_RenderClear( sdlRenderer_ );
    SDL_RenderCopy( sdlRenderer_, sdlTexture_, NULL, &sdlRect_);
    SDL_RenderPresent( sdlRenderer_ );
    //Delay 40ms
    //SDL_Delay(40);
	return 0;
}

int SDLPlayer::play(unsigned char* buf)
{
    printf("%s\n",__FUNCTION__);
    
    //printf(" %d %d %d\n", frame->linesize[0], frame->linesize[1], frame->linesize[2]);
    SDL_UpdateTexture( sdlTexture_, NULL, buf, screenWidth_);
    //FIX: If window is resize
    sdlRect_.x = 0;
    sdlRect_.y = 0;
    sdlRect_.w = screenWidth_;
    sdlRect_.h = screenHeight_;

    SDL_RenderClear( sdlRenderer_ );
    SDL_RenderCopy( sdlRenderer_, sdlTexture_, NULL, &sdlRect_);
    SDL_RenderPresent( sdlRenderer_ );
    //Delay 40ms
    //SDL_Delay(40);
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
