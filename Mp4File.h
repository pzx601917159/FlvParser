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
#include <string.h>
#include <map>

#define BOX_TYPE(c1, c2, c3, c4) \
    (((static_cast<uint32_t>(c1)) << 24) | \
     ((static_cast<uint32_t>(c2)) << 16) | \
     ((static_cast<uint32_t>(c3)) << 8) | \
     ((static_cast<uint32_t>(c4))))


// box类型
enum BoxType
{
    FTYP = BOX_TYPE('f','t','y','p'),
    MDAT = BOX_TYPE('m','d','a','t'),
    MOOV = BOX_TYPE('m','o','o','v'),
    MVHD = BOX_TYPE('m','v','h','d'),
    TRAK = BOX_TYPE('t','r','a','k'),
    TKHD = BOX_TYPE('t','k','h','d'),
    EDTS = BOX_TYPE('e','d','t','s'),
    MDIA = BOX_TYPE('m','d','i','a'),
    MDHD = BOX_TYPE('m','d','h','d'),
    HDLR = BOX_TYPE('h','d','l','r'),
    MINF = BOX_TYPE('m','i','n','f'),
    VMHD = BOX_TYPE('v','m','h','d'),
    DINF = BOX_TYPE('d','i','n','f'),
    STBL = BOX_TYPE('s','t','b','l'),
    STSD = BOX_TYPE('s','t','s','d'),
    STTS = BOX_TYPE('s','t','t','s'),
    CTSS = BOX_TYPE('c','t','s','s'),
    STSS = BOX_TYPE('s','t','s','s'),
    STSC = BOX_TYPE('s','t','s','c'),
    STSZ = BOX_TYPE('s','t','s','z'),
    STCO = BOX_TYPE('s','t','c','o')
};

static std::string uint32ToString(uint32_t boxType)
{
    return std::string(reinterpret_cast<char*>(&boxType), 4);
}

// box的头部
struct BoxHeader
{
    uint32_t size_; // 如果值为1则包含largesize
    uint32_t type_; // 根据type判断是不是full box
    uint64_t large_size_;
    // 解析header
    void parse()
    {
    }
};

struct FullBoxHeader: public BoxHeader
{
    uint8_t version;    // fullbox才包含version和flags
    uint32_t flasg; // 只用到了三个字节
};

// box的头部
class BoxBody
{
    std::vector<uint8_t> data;
};

class Mp4Box
{
    public:
    Mp4Box(){}
    void registerBox(BoxType box_type)
    {
    }

    Mp4Box(uint32_t box_type, uint32_t box_size, std::vector<uint8_t> data)
    {

    }

    Mp4Box* createBox()
    {
    }

    virtual bool isFullBox()
    {
        return false;
    }

    virtual bool isContainerBox()
    {
        return false;
    }

    BoxHeader* header_;
    BoxBody* body_;
    std::vector<uint8_t> data_;
    char* data()
    {
        return (char*)data_.data();
    }
    Mp4Box* parent_;    // 父节点
    std::map<std::string, Mp4Box*> children_;
};

struct Mp4FullBox : public Mp4Box
{
    bool isFullBox() override
    {
        return true;
    }
};

struct Mp4ContainerBox : public Mp4Box
{
    bool isContainerBox() override
    {
        return true;
    }
};

// file type
struct FtypBox : public Mp4Box
{
    FtypBox(){}
    uint32_t major_band_;
    uint32_t minor_version_;
    uint32_t compatible_brands_;
    // 解析ftype
    void parse()
    { 
        memcpy(&major_band_, data() + 8, sizeof(major_band_));
        memcpy(&minor_version_, data() + 12, sizeof(major_band_));
        memcpy(&compatible_brands_, data() + 16, sizeof(major_band_));
    }
};


// full box
struct MvhdBox:public Mp4Box
{
    // 部分字段version=1为64位，version=1为32位
    uint64_t creation_time_;    // 创建时间，version=1为64为，0为32位
    uint64_t modification_time_;// 修改时间
    uint32_t timescale_;        //  文件媒体在1秒时间内的刻度值，可理解为1秒长度的时间单元数。 
    uint64_t duration_;         // 该track的时间长度，用duration和time scale值可以计算track时长，比如audio track的time scale = 8000, duration = 560128，时长为70.016，video track的time scale = 600, duration = 42000，时长为70。
    int32_t rate_;  // 推荐播放速率，高16位和低16位分别为小数点整数部分和小数部分，即[16.16] 格式，该值为1.0（0x00010000）表示正常前向播放。
    int16_t volume_;    // : 推荐播放音量，[8.8] 格式，1.0（0x0100）表示最大音量。
    uint16_t reserved_; // 为0
    uint32_t reserved[2];   //
    int32_t matrix[9] = { 0x00010000,0,0,0,0x00010000,0,0,0,0x40000000 };
    int32_t pre_defined[6] = {0};
    uint32_t next_track_ID_;
};

// version : box版本，0或1，一般为0。
//flags : 24-bit整数，按位或操作结果值，预定义的值(0x000001 ，track_enabled，表示track是有效的）、(0x000002，track_in_movie，表示该track在播放
struct TkhdBox
{
    // 部分字段version=1为64位，version=1为32位
    uint64_t creation_time_;    // 创建时间，version=1为64为，0为32位
    uint64_t modification_time_;// 修改时间
    uint32_t track_ID_;         //  track id号，不能重复且不能为0。
    const uint32_t reserved_ = 0;
    uint64_t duration_;         // 该track的时间长度，用duration和time scale值可以计算track时长，比如audio track的time scale = 8000, duration = 560128，时长为70.016，video track的time scale = 600, duration = 42000，时长为70。
    uint32_t reserved[2];   //
    int16_t layer_;
    int16_t alternate_group_; 
    int16_t volume_;    //  {if track_is_audio 0x0100 else 0}; : [8.8] 格式，如果为音频track，1.0（0x0100）表示最大音量；否则为0。
    const uint16_t reserved_16_ = 0;
    int32_t matrix[9] = { 0x00010000,0,0,0,0x00010000,0,0,0,0x40000000 };
    uint32_t width_;    //  宽，[16.16] 格式值。
    uint32_t height_;   // 高，[16.16] 格式值，不必与sample的像素尺寸一致，用于播放时的展示宽高。
};

// mdhd 和 tkhd ，内容大致都是一样的。不过tkhd 通常是对指定的 track 设定相关属性和内容。而 mdhd 是针对于独立的 media 来设置的。不过两者一般都是一样的。
// version: box版本，0或1，一般为0。
// timescale: 同mvhd中的timescale。
// duration: track的时间长度。
// language: 媒体语言码。最高位为0，后面15位为3个字符（见ISO 639-2/T标准中定义）。
struct MdhdBox
{
    uint64_t creation_time_;
    uint64_t modification_time_;
    uint64_t timescale_;
    uint64_t duration_;
    // bit(1) pad = 0;
    // char pad_ = 0;
    // unsigned int(5)[3] language; // ISO-639-2/T language code
    uint16_t language_;
    uint16_t pre_defined_ = 0;
};

// “hdlr”解释了媒体的播放过程信息，该box也可以被包含在meta box（meta）中。
// handler type: 在media box中，该值为4个字符，会有以下取值:

//‘vide’ Video track
//‘soun’ Audio track
//‘hint’ Hint track
//‘meta’ Timed Metadata track
//‘auxv’ Auxiliary Video track
// name: human-readable name for the track
// type，以‘\0’结尾的 UTF-8 字符串。用于调试后者检查的目的。
struct HdlrBox:public Mp4Box
{
    uint32_t pre_defined_ = 0;
    uint32_t handler_type_;
    const uint32_t reserved[3] = {0};
    std::string name;
};

// container box
struct MinfBox:public Mp4Box
{
};


// 其子box包括：
// stsd：sample description box，样本的描述信息。
// stts：time to sample box，sample解码时间的压缩表。
// ctts：composition time to sample box，sample的CTS与DTS的时间差的压缩表。
// stss：sync sample box，针对视频，关键帧的序号。
// stsz/stz2：sample size box，每个sample的字节大小。
// CMakeLists.txtstsc：sample to chunk box，sample-chunk映射表。
// stco/co64：chunk offset box，chunk在文件中的偏移。
struct StblBox:public Mp4ContainerBox
{
};

struct SampleEntry
{

};

// 存储了编码类型和初始化解码器需要的信息
struct StsdBox: public Mp4FullBox
{
    int i;
    uint32_t entry_count_; // entry的个数

};


// container box
// 其子box的结构和种类还是比较复杂的。
//“mdia”定义了track媒体类型以及sample数据，描述sample信息。
// 一个“mdia”必须包含如下容器：

// 一个Media Header Atom(mdhd)
// 一个Handler Reference(hdlr)
// 一个media information(minf)和User Data
struct MediaBox:public Mp4Box
{  
};

// full box
struct Vmhd:public Mp4FullBox
{
    // version = 0,1
    uint16_t graphics_mode_ = 0;    // 视频合成模式，为0时拷贝原始图像，否则与opcolor进行合成。
    uint16_t op_color_[3] = {0, 0, 0}; // 一组(red，green，blue)，graphics modes使用。
};

// full box
struct Smhd:public Mp4FullBox
{
    // version = 0,0
    uint16_t balance_ = 0;// 立体声平衡，[8.8] 格式值，一般为0表示中间，-1.0表示全部左声道，1.0表示全部右声道。
    uint16_t reserved_ = 0;
};

struct Dinf:public Mp4ContainerBox
{
    
};

// container box
struct TrackBox:public Mp4ContainerBox
{   
};

// metadata container box
struct MoovBox: public Mp4Box
{
    // 包含一些列box
    std::vector<Mp4Box> boxVec_;
};

// metadata container box
struct MdatBox: public Mp4Box
{
    // 包含一些列box
    std::vector<Mp4Box> boxVec_;
};

// 用于解析mp4文件
class Mp4File
{
    public:
        Mp4File(){
        }
        ~Mp4File(){
        }

        int init(const std::string file_name);

        // 解析文件
        int parse();
    public:
        std::string file_name_;
        std::fstream fs_;
        std::map<std::string, Mp4Box*> boxes_;
        size_t file_size_;
};

#endif  // MP4PARSER_H_
