#include <iostream>
#include "SDLPlayer.h"
#include "Mp4File.h"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << argv[0] <<" [file name]" << std::endl;
        return 0;
    }
    // 播放视频
    // SDLPlayer::instance()->init(argv[1]);
    // 主线程
    // SDLPlayer::instance()->play();
    Mp4File file;
    file.init(argv[1]);
    int ret = file.parse();
    file.show_file();
    uint32_t offset = file.get_offset(10);
    LOG_DEBUG("offset 10:{}", offset);
    return ret;
}
