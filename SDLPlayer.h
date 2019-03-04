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
    private:
    SDLPlayer();
    ~SDLPlayer();
    public:
    // thread safe in c++11
    static SDLPlayer* instance()
    {
        static SDLPlayer player;
        return &player;
    }
    int init();
    int destory();
    //for ffmpeg
    int play(AVFrame* frame, int pts, int width, int height);
    int play(unsigned char* frame, int pts, int width, int height);
    int playAudio(unsigned char* data,uint32_t size);
    static int refresh_video(void *opaque);
    static void fillAudio(void* data, uint8_t* stream, int len);

    private:
    //Bit per Pixel
    int bpp_;
    int screenWidth_;
    int screenHeight_;
    int pixelWidth_;
    int pixelHeight_;

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
    static uint32_t serial_;
    static double audioTime_;
    static double videoTime_;
    static uint64_t ptsDrift_;    
    static double audioClock_;
    static double videoClock_;
    static uint64_t audioDataCount_;
};
extern SDLPlayer g_sdlPlayer;
#endif //__SDL_PLAYER_H__
