#include <iostream>
#include "SDLPlayer.h"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "FlvFile [input flv]" << std::endl;
        return 0;
    }
    // 播放视频
    SDLPlayer::instance()->init(argv[1]);
    // 主线程
    SDLPlayer::instance()->play();
}
