/*************************************************************************
	> File Name: Mp4Parser.h
	> Author: pzx
	> Created Time: 2019年03月04日 星期一 14时09分32秒
************************************************************************/
#ifndef MP4PARSER_H_
#define MP4PARSER_H_
#include <cinttypes>
#include <string>
#include <vector>
#include <fstream>
// box的头部
struct BoxHeader
{
    uint32_t size_; // 如果值为1则包含largesize
    uint32_t type_; // 根据type判断是不是full box
    uint64_t large_size_;
    uint8_t version;    // fullbox才包含version和flags
    uint32_t flasg; // 只用到了三个字节
};

// box的头部
class BoxBody
{
};

struct Mp4Box
{
    BoxHeader header_;
    BoxBody body_;
};

// file type
struct FtypBox : public Mp4Box
{
    uint32_t major_band_;
    uint32_t minor_version_;
    uint32_t compatible_brands_;
};

struct MvhdBox:public Mp4Box
{
};

// metadata
struct MoovBox: public Mp4Box
{
    
};

// 用于解析mp4文件
class Mp4File
{
    public:
        Mp4File(){}
        ~Mp4File(){}

        int init(const std::string file_name);

        // 解析文件
        int parse();
    public:
        std::string file_name_;
        std::vector<Mp4Box> boxes_;
        std::fstream fs_;
};

#endif  // MP4PARSER_H_
