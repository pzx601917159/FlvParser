/*************************************************************************
	> File Name: SDLPlayer.h
	> Author: pzx
	> Created Time: 2019年02月22日 星期五 10时44分24秒
************************************************************************/
#ifndef __SDL_PLAYER_H__
#define __SDL_PLAYER_H__
//Refresh Event
#define REFRESH_EVENT  (SDL_USEREVENT + 1)
#include <inttypes.h>
extern "C"
{
#include "libavutil/avutil.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "SDL2/SDL.h"
}
class SDLPlayer
{
    public:
    SDLPlayer();
    ~SDLPlayer();
    int init();
    int destory();
    int play(AVFrame* frame);
    int play(unsigned char* frame);
    int playAudio(unsigned char* data,uint32_t size);
    private:
    //Bit per Pixel
    int bpp_;
    int screenWidth_;
    int screenHeight_;
    int pixelWidth_;
    int pixelHeight_;
    //unsigned char buffer[pixelHeight_*pixelHeight_*bpp/8];
    //BPP=32
    //unsigned char buffer_convert[pixelWidth_*pixelHeight_*4];
    static int thread_exit;
    void CONVERT_24to32(unsigned char *image_in,unsigned char *image_out,int w,int h);
    static int refresh_video(void *opaque);
    static void fillAudio(void* data, uint8_t* stream, int len);
	SDL_Window *screen_;
	SDL_Renderer* sdlRenderer_;
    SDL_Texture* sdlTexture_;
	Uint32 pixformat_;
	SDL_Rect sdlRect_;
	SDL_Thread *refresh_thread_; 
	SDL_Event event_;
    SDL_AudioSpec audioSpec_;
    static uint32_t audioLen_;
    static unsigned char* audioChrunk_;
    static unsigned char* audioPos_;
    bool init_;
};
extern SDLPlayer g_sdlPlayer;
#endif //__SDL_PLAYER_H__
