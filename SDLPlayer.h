/*************************************************************************
    > File Name: SDLPlayer.h
    > Author: pzx
    > Created Time: 2019年02月22日 星期五 10时44分24秒
************************************************************************/
#ifndef SDLPLAYER_H_
#define SDLPLAYER_H_
#include <cinttypes>
#include <thread>
#include <string>
extern "C"
{
#include "libavutil/avutil.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "SDL2/SDL.h"
}
#include "FlvFile.h"
#include "VideoState.h"
#include <mutex>

// 暂时只维护ffmpeg版本
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
    int init(std::string fileName);
    int destory();
    // for ffmpeg
    int play(AVFrame* frame, int pts, int width, int height);
    int play(unsigned char* frame, int pts, int width, int height);
    int playAudio2(unsigned char* data, uint32_t size);
    static int refresh_video(void *opaque);
    static void fillAudio(void* data, uint8_t* stream, int len);

    // 播放音视频
    int play();
    // 播放视频
    static int playVideo();
    // 播放音频
    static int playAudio();
    int startVideoThread();
    int startAudioThread();
    int startFileThread();

    static int decodeVideo();
    static int decodeAudio();
    static int parseFile();

    private:
    // Bit per Pixel
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
    static bool terminate_;    // 结束播放
    std::thread* videoThread_;
    std::thread* audioThread_;
    std::thread* fileThread_;
    static FlvFile file_;
    static std::mutex videoTagMutex_;
    static std::mutex videoFrameMutex_;
    static std::mutex audioTagMutex_;
    static std::mutex audioFrameMutex_;
};
#endif  // SDLPLAYER_H_
