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

#include <string>
#include <cstdio>
#include <ctime>
#include <thread>
#include "FlvFile.h"
#include "SDLPlayer.h"
#include "Clock.h"
#include "VideoState.h"
#include "DecoderFactory.h"

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
bool SDLPlayer::terminate_ = false;
FlvFile SDLPlayer::file_;
std::mutex SDLPlayer::videoTagMutex_;
std::mutex SDLPlayer::videoFrameMutex_;
std::mutex SDLPlayer::audioTagMutex_;
std::mutex SDLPlayer::audioFrameMutex_;

SDLPlayer::SDLPlayer()
{
    // YUV420P
    bpp_ = 12;
    // TODO 通过metadata初始化
    //screenWidth_ = VideoState::videoWidth_;
    //screenHeight_ = VideoState::videoHeight_;
    screenWidth_ = 320;
    screenHeight_ = 240;
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
    // audioSpec_.userdata = ;
    terminate_ = false;

    videoThread_ = nullptr;
    audioThread_ = nullptr;
    fileThread_ = nullptr;
}

// 音频的回调函数
void SDLPlayer::fillAudio(void* data, uint8_t* stream, int len)
{
    // SDL 2.0
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
    // 音频播放了多长时间
    audioClock_ = static_cast<double>(audioDataCount_) /
        static_cast<double>(getBytesPersecond(44100, 2));
    printf("audio serial_= %d time = %lf timesys:%ld len:%d audioLen_:%d\n",
            serial_, audioClock_, getTime(), len, audioLen_);
}

SDLPlayer::~SDLPlayer()
{
}

int SDLPlayer::init(std::string fileName_)
{
    file_.init(fileName_);
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
    {
        printf("Could not initialize SDL - %s\n", SDL_GetError());
        return -1;
    }

    // SDL 2.0 Support for multiple windows
    screen_ = SDL_CreateWindow("Simplest Video Play SDL2",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            screenWidth_, screenHeight_,
            SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
    if(!screen_)
    {
        printf("SDL: could not create window - exiting:%s\n", SDL_GetError());
        return -1;
    }
    sdlRenderer_ = SDL_CreateRenderer(screen_, -1, 0);

    // IYUV: Y + U + V  (3 planes)
    // YV12: Y + V + U  (3 planes)
    pixformat_ = SDL_PIXELFORMAT_IYUV;

    sdlTexture_ = SDL_CreateTexture(sdlRenderer_, pixformat_,
            SDL_TEXTUREACCESS_STREAMING, pixelWidth_, pixelHeight_);

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
    terminate_ = true;
    if(videoThread_)
    {
        videoThread_->join();
        delete videoThread_;
        videoThread_ = NULL;
    }
    if(audioThread_)
    {
        audioThread_->join();
        delete audioThread_;
        videoThread_ = NULL;
    }
    if(videoThread_)
    {
        fileThread_->join();
        delete fileThread_;
        fileThread_ = NULL;
    }
    SDL_Quit();
}

int SDLPlayer::play(AVFrame* frame, int pts, int width, int height)
{
    // 重点：计算当前正在播放的音频的pts,audioClock_包含了缓冲区的数据
    // 当前播放的音频的时间为
    // 视频和音频的时间差
    int delay = pts * 1000 - audioClock_*1000000;
    // int deley = getTime() - ptsDrift_;
    if(delay > 0)
    {
        /*
        struct timespec time;
        struct timespec remain;
        time.tv_sec = delay / 1000000;
        time.tv_nsec = (delay % 1000000) * 1000;
        int ret = nanosleep(&time, &remain);
        */
    }
    SDL_Delay(40);
    printf("video pts:%d audioclock:%lf deleay:%d thread_id:%d\n",
            pts, audioClock_, delay, pthread_self());

    SDL_UpdateYUVTexture(sdlTexture_, &sdlRect_,
        frame->data[0], frame->linesize[0],
        frame->data[1], frame->linesize[1],
        frame->data[2], frame->linesize[2]);


    // FIX: If window is resize
    sdlRect_.x = 0;
    sdlRect_.y = 0;
    sdlRect_.w = screenWidth_;
    sdlRect_.h = screenHeight_;

    SDL_RenderClear(sdlRenderer_);
    SDL_RenderCopy(sdlRenderer_, sdlTexture_, NULL, &sdlRect_);
    SDL_RenderPresent(sdlRenderer_);
    return 0;
}

int SDLPlayer::play(unsigned char* buf, int pts, int width, int height)
{
    printf("_____________________________\n");
    // 当前播放的音频的时间为
    int delay = pts * 1000 - audioClock_*1000000;
    // int deley = getTime() - ptsDrift_;
    /*
    if(delay > 0)
    {
        struct timespec time;
        struct timespec remain;
        time.tv_sec = delay / 1000000;
        time.tv_nsec = (delay % 1000000) * 1000;
        int ret = nanosleep(&time, &remain);
    }
    */
    SDL_UpdateTexture(sdlTexture_, NULL, buf, screenWidth_);
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


int SDLPlayer::playAudio2(unsigned char* data, uint32_t size)
{
    printf("===========================\n");
    audioChrunk_ = data;
    audioLen_ = size;
    audioPos_ = audioChrunk_;
    /*
    while(audioLen_ > 0)
    {
        printf("sdl delay\n");
        SDL_Delay(1);
    }
    */
    return 0;
}


// 播放音频
int SDLPlayer::playAudio()
{
    while(!terminate_)
    {
        //std::unique_lock<std::mutex> lock(audioFrameMutex_);
        if(!VideoState::audioFrames_.empty())
        {
            AVFrame* frame = VideoState::audioFrames_.front();
            //SDLPlayer::instance()->playAudio2(data->data_, data->size_);
            // free(data->data_);
            VideoState::audioFrames_.pop();
        }
        else
        {
            //lock.unlock();
            printf("decode audio sleep\n");
            struct timespec spec;
            struct timespec rem;
            spec.tv_sec = 1;
            spec.tv_nsec = 100000;
            nanosleep(&spec, &rem);
            // TODO cond signal
        }
    }
    return 0;
}

// 播放视频
int SDLPlayer::playVideo()
{
    while(!terminate_)
    {
        std::unique_lock<std::mutex> lock(videoFrameMutex_);
        if(!VideoState::videoFrames_.empty())
        {
            AVFrame* frame = VideoState::videoFrames_.front();
            SDLPlayer::instance()->play(frame, 0, 320, 240);
            //free(data->data_);
            VideoState::videoFrames_.pop();
        }
        else
        {
            lock.unlock();
            printf("play video sleep\n");
            struct timespec spec;
            struct timespec rem;
            spec.tv_sec = 1;
            spec.tv_nsec = 100000;
            nanosleep(&spec, &rem);
            // TODO cond signal
        }
    }
    return 0;
}

// 播放音视频
int SDLPlayer::play()
{
    // 初始化解码器
    DecoderFactory::init();

    // 开启文件读取线程
    startFileThread();
    // 开启视频解码线程
    startVideoThread();
    // 开启音频解码线程
    startAudioThread();

    // 播放音视频
    std::thread* audioPlayThread = new std::thread(playAudio);
    std::thread* videoPlayThread = new std::thread(playVideo);
    audioPlayThread->join();
    videoPlayThread->join();
    return 0;
}

int SDLPlayer::startVideoThread()
{
    if(videoThread_ == nullptr)
    {
        videoThread_ = new std::thread(decodeVideo);
    }
}

int SDLPlayer::startAudioThread()
{
    printf("%s\n", __FUNCTION__);
    if(audioThread_ == nullptr)
    {
        audioThread_ = new std::thread(decodeAudio);
    }
}

int SDLPlayer::startFileThread()
{
    if(fileThread_ == nullptr)
    {
        fileThread_ = new std::thread(parseFile);
    }
}

int SDLPlayer::decodeVideo()
{
    int width = 0;
    int height = 0;
    int pixFmt = 0;
    while(!terminate_)
    {
        printf("video decode thread video frame size:%d videotag size:%d\n", VideoState::videoFrames_.size(), VideoState::videoTags_.size());
        std::unique_lock<std::mutex> lock(videoTagMutex_);
        std::unique_lock<std::mutex> lock2(videoFrameMutex_);
        if(VideoState::videoFrames_.size() < MAX_FRAME_SIZE && !VideoState::videoTags_.empty())
        {
            VideoTag* tag = reinterpret_cast<VideoTag*>(VideoState::videoTags_.front());
            // TODO
            DecoderFactory::getDecoder(DECODER_TYPE_FFMPEG_VIDEO)->decodeFrame(tag->mediaData_.data_, tag->mediaData_.size_, &width, &height, &pixFmt, tag->pts_);
            VideoState::videoTags_.pop();
            delete tag;
        }
        else
        {
            lock.unlock();
            lock2.unlock();
            printf("decode video sleep\n");
            struct timespec spec;
            struct timespec rem;
            spec.tv_sec = 1;
            spec.tv_nsec = 100000;
            nanosleep(&spec, &rem);
            // TODO cond signal
        }
    }
    return 0;
}

int SDLPlayer::decodeAudio()
{
    printf("decode audio\n");
    int width = 0;
    int height = 0;
    int pixFmt = 0;
    while(!terminate_)
    {
        printf("audio decode thread audio frame size:%d audiotag size:%d\n", VideoState::audioFrames_.size(), VideoState::audioTags_.size());
        std::unique_lock<std::mutex> lock(audioTagMutex_);
        std::unique_lock<std::mutex> lock2(audioFrameMutex_);
        if(VideoState::audioFrames_.size() < MAX_FRAME_SIZE && !VideoState::audioTags_.empty())
        {
            AudioTag* tag = reinterpret_cast<AudioTag*>(VideoState::audioTags_.front());
            // TODO
            DecoderFactory::getDecoder(DECODER_TYPE_FFMPEG_AUDIO)->decodeFrame(tag->mediaData_.data_, tag->mediaData_.size_, &width, &height, &pixFmt, tag->pts_);
            VideoState::audioTags_.pop();
            delete tag;
        }
        else
        {
            lock.unlock();
            lock2.unlock();
            printf("decode audio sleep\n");
            struct timespec spec;
            struct timespec rem;
            spec.tv_sec = 1;
            spec.tv_nsec = 100000;
            nanosleep(&spec, &rem);
            // TODO cond signal
        }
    }
    return 0;
}


int SDLPlayer::parseFile()
{
    while(!terminate_)
    {
        printf("video tag size:%d audio tags size:%d\n",
                VideoState::videoTags_.size(),
                VideoState::audioTags_.size());
        std::unique_lock<std::mutex> lock(videoTagMutex_);
        // TODO 考虑只存在音频或者视频的情况,可以根据flvheader判断
        if(VideoState::videoTags_.size() < MAX_TAG_SIZE)
        {
            Tag* tag = file_.parseFlv();
            if(tag != nullptr)
            {
                if(tag->tagHeader_.tagType_ == TAG_TYPE_VIDEO)
                {
                    VideoState::videoTags_.push(tag);
                }
                else if(tag->tagHeader_.tagType_ == TAG_TYPE_AUDIO)
                {
                    lock.unlock();
                    std::unique_lock<std::mutex> lock2(audioTagMutex_);
                    VideoState::audioTags_.push(tag);
                }
            }
            else
            {
                printf("tag is null\n");
            }
        }
        else
        {
            lock.unlock();
            printf("parse sleep\n");
            struct timespec spec;
            struct timespec rem;
            spec.tv_sec = 1;
            spec.tv_nsec = 0;
            nanosleep(&spec, &rem);
            // TODO cond signal
        }
    }
    return 0;
}

