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

#include "ByteUtil.h"
#include "log.h"

#define BOX_TYPE(c1, c2, c3, c4) \
    (((static_cast<uint32_t>(c1))) | \
     ((static_cast<uint32_t>(c2)) << 8) | \
     ((static_cast<uint32_t>(c3)) << 16) | \
     ((static_cast<uint32_t>(c4)) << 24))


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
    STSH = BOX_TYPE('s','t','s','h'),
    STDP = BOX_TYPE('s','t','d','p'),
    PADB = BOX_TYPE('p','a','d','b'),
    STSC = BOX_TYPE('s','t','s','c'),
    STSZ = BOX_TYPE('s','t','s','z'),
    STZ2 = BOX_TYPE('s','t','z','2'),
    STCO = BOX_TYPE('s','t','c','o'),
    CO64 = BOX_TYPE('c','o','6','4'),
    TREF = BOX_TYPE('t','r','e','f'),
    SMHD = BOX_TYPE('s','m','h','d'),
    HMHD = BOX_TYPE('h','m','h','d'),
    FREE = BOX_TYPE('f','r','e','e'),
    PDIN = BOX_TYPE('p','d','i','n'),
    MOOF = BOX_TYPE('m','o','o','f'),
    MFRA = BOX_TYPE('m','f','r','a'),
    SKIP = BOX_TYPE('s','k','i','p'),
    UUID = BOX_TYPE('u','u','i','d'),
};

static std::string uint32ToString(uint32_t boxType)
{
    return std::string(reinterpret_cast<char*>(&boxType), 4);
}

// 按照mp4 pdf文件时先
struct Box
{
    Box(uint32_t box_type, uint32_t box_size, uint8_t extend_type[16] = {})
    {
        parent_ = nullptr;
        size_ = box_size;
        type_ = box_type;
        parse_len_ = 0;
    }
    Box(uint32_t box_type, uint32_t box_size, std::vector<uint8_t>& data)
    {
        size_ = box_size;
        type_ = box_type;
        parse_len_ = 0;
        parse(data);
    }

    // 解析并释放数据
    void parse(std::vector<uint8_t>& data)
    {
        if(size_ == 1)
        {
            memcpy(&large_size_, data.data() + parse_len_, sizeof(large_size_));
            parse_len_ += sizeof(large_size_);
        }
        if(type_ == BoxType::UUID)
        {
            memcpy(extend_type_, data.data() + parse_len_, sizeof(extend_type_));
            parse_len_ += sizeof(extend_type_);
        }
    }
    uint32_t size_;
    uint32_t type_;
    uint32_t large_size_;   // if size==1,large_size存在，if size == 0,box extends to end of file,只在mdat中可能用到
    uint8_t extend_type_[16];// if box_type == 'uuid'
    uint32_t parse_len_;
    // 子box
    std::map<uint32_t, Box*> children_;
    // 父box
    Box* parent_;
};

struct FullBox:public Box
{
    FullBox(uint32_t box_type, uint32_t box_size, uint8_t v, int8_t f[3]):Box(box_type, box_size)
    {
    }
    FullBox(uint32_t box_type, uint32_t box_size, uint32_t version_flags):Box(box_type, box_size)
    {
        version_ = version_ >> 24;
        memcpy(flags_, (uint8_t*)&version_flags + 1, 3);
    }
    FullBox(uint32_t box_type, uint32_t box_size, std::vector<uint8_t>& data):
        Box(box_type, box_size, data)
    {
        parse(data);
    }
    void parse(std::vector<uint8_t>& data)
    {
        version_ = data[parse_len_++];
        memcpy(flags_, data.data() + parse_len_, sizeof(flags_));
        parse_len_ += sizeof(flags_);
        LOG_DEBUG("version:{} flags:{}", version_, flags_);
    }
    uint8_t version_;
    int8_t flags_[3];
    //virtual void parse()
};

// file type
struct FtypBox : public Box
{
    FtypBox(uint32_t box_size):Box(BoxType::FTYP, box_size)
    {
    }
    uint32_t major_band_;
    uint32_t minor_version_;
    uint32_t compatible_brands_;

    void set_data()
    {

    }

    // 解析ftype
    void parse(std::vector<uint8_t>& box_data)
    {
        memcpy(&major_band_, box_data.data(), sizeof(major_band_));
        memcpy(&minor_version_, box_data.data() + 4, sizeof(major_band_));
        memcpy(&compatible_brands_, box_data.data() + 8, sizeof(major_band_));
    }
};

// metadata container box
struct MoovBox: public Box
{
    MoovBox(uint32_t box_size):Box(BoxType::MOOV, box_size)
    {
        // std::cout << "box type:" << box_type << std::endl;
    }
};

// 音视频数据
struct MdatBox: public Box
{
    // 包含具体的媒体数据
    std::vector<uint8_t> data_;
};

// full box
struct MvhdBox:public FullBox
{
    MvhdBox(uint8_t version, uint32_t box_size, int8_t flags[3]):FullBox(BoxType::MVHD, box_size, version, flags)
    {
    }
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

// container:moov box
struct TrakBox:public Box
{
    TrakBox(uint32_t box_size):Box(BoxType::TRAK, box_size)
    {
    }
};

// version : box版本，0或1，一般为0。
//flags : 24-bit整数，按位或操作结果值，预定义的值(0x000001 ，track_enabled，表示track是有效的）、(0x000002，track_in_movie，表示该track在播放
struct TkhdBox : public Box
{
    TkhdBox(uint32_t box_size):Box(BoxType::TKHD, box_size)
    {
    }
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

struct TrefBox:public Box
{
    TrefBox(uint32_t box_size):Box(BoxType::TREF, box_size)
    {
    }
};

struct TrefTypeBox:public Box
{
    TrefTypeBox(uint32_t reference_type, uint32_t box_size):Box(reference_type, box_size)
    {
    }
    uint32_t trak_ids_; // trak_ids_[]数组，这里怎么表示呢？
};

// container box
// 其子box的结构和种类还是比较复杂的。
//“mdia”定义了track媒体类型以及sample数据，描述sample信息。
// 一个“mdia”必须包含如下容器：

// 一个Media Header Atom(mdhd)
// 一个Handler Reference(hdlr)
// 一个media information(minf)和User Data
// container:trak
struct MdiaBox:public Box
{
    MdiaBox(uint32_t box_size):Box(BoxType::MDIA, box_size)
    {
    }
};



// mdhd 和 tkhd ，内容大致都是一样的。不过tkhd 通常是对指定的 track 设定相关属性和内容。而 mdhd 是针对于独立的 media 来设置的。不过两者一般都是一样的。
// version: box版本，0或1，一般为0。
// timescale: 同mvhd中的timescale。
// duration: track的时间长度。
// language: 媒体语言码。最高位为0，后面15位为3个字符（见ISO 639-2/T标准中定义）。
// container:mdia
struct MdhdBox:public FullBox
{
    MdhdBox(uint32_t box_size, std::vector<uint8_t>& data):
        FullBox(BoxType::MDHD, box_size, data)
    {
        parse(data);
    }
    MdhdBox(uint32_t box_size, uint8_t version, int8_t flags[3] = {0}):FullBox(BoxType::MDHD, box_size, version, flags)
    {
    }
    // 解析ftype
    void parse(std::vector<uint8_t>& data)
    {
        if(version_ == 1)
        {
            // seek需要timescale_
            memcpy(&creation_time_, data.data() + parse_len_, sizeof(creation_time_));
            parse_len_ += sizeof(creation_time_);
            memcpy(&modification_time_, data.data() + parse_len_, sizeof(modification_time_));
            parse_len_ += sizeof(modification_time_);
            memcpy(&timescale_, data.data() + parse_len_, sizeof(timescale_));
            parse_len_ += sizeof(timescale_);
            memcpy(&duration_, data.data() + parse_len_, sizeof(duration_));
            parse_len_ += sizeof(duration_);
        }
        // 没有其他值
        else // if(version_ == 0)
        {
            // seek需要timescale_
            creation_time_ = ShowU32(data.data() + parse_len_);
            parse_len_ += 4;
            modification_time_ = ShowU32(data.data() + parse_len_);
            parse_len_ += 4;
            timescale_ = ShowU32(data.data() + parse_len_);
            parse_len_ += sizeof(timescale_);
            duration_ = ShowU32(data.data() + parse_len_);
            parse_len_ += 4;
        }
        memcpy(&language_, data.data() + parse_len_, sizeof(language_));
        parse_len_ += sizeof(language_);
        memcpy(&pre_defined_, data.data() + parse_len_, sizeof(pre_defined_));
        parse_len_ += sizeof(pre_defined_);
        LOG_DEBUG("create_time:{} modification_time:{} timescale:{} duration:{}", creation_time_, modification_time_, timescale_, duration_);
        LOG_DEBUG("real duration:{}", duration_/timescale_);
    }
    // version == 1
    uint64_t creation_time_;
    uint64_t modification_time_;
    uint32_t timescale_;
    uint64_t duration_;
    /*
     * version ==0
     * uint32_t creation_time_
     * uint32_t modification_time_
     * uint32_t timescale_
     * uint32_t duration_
     */
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
// container:mdia or meta
struct HdlrBox:public FullBox
{
    HdlrBox(uint32_t box_size):FullBox(BoxType::HDLR, box_size, 0)
    {

    }
    uint32_t pre_defined_ = 0;
    uint32_t handler_type_;
    const uint32_t reserved[3] = {0};
    std::string name;
    void parse(std::vector<uint8_t>& data)
    {
        // 这个时fullbox
        memcpy(&handler_type_,data.data() + 8, 4);
    }
};

// container box
// container:mdia
struct MinfBox:public Box
{
    MinfBox(uint32_t box_size):Box(BoxType::MINF, box_size)
    {
    }
};


// full box
// container:minf
struct VmhdBox:public FullBox
{
    //  0 , 1
    VmhdBox(uint32_t box_size):FullBox(BoxType::VMHD, box_size, 1)
    {
    }
    // version = 0,1
    uint16_t graphics_mode_ = 0;    // 视频合成模式，为0时拷贝原始图像，否则与opcolor进行合成。
    uint16_t op_color_[3] = {0, 0, 0}; // 一组(red，green，blue)，graphics modes使用。
};

// full box
struct SmhdBox:public FullBox
{
    SmhdBox(uint32_t box_size):FullBox(BoxType::SMHD, box_size, 0)
    {

    }
    // version = 0,0
    uint16_t balance_ = 0;// 立体声平衡，[8.8] 格式值，一般为0表示中间，-1.0表示全部左声道，1.0表示全部右声道。
    uint16_t reserved_ = 0;
};

struct HmhdBox : public FullBox
{
    HmhdBox(uint32_t box_size):FullBox(BoxType::HMHD, box_size, 0)
    {
    }
    uint16_t max_pdu_size_;
    uint16_t avg_pdu_size_;
    uint32_t max_bitrate_;
    uint32_t avg_bitrate_;
    uint32_t reserved_;
};


// 其子box包括：
// stsd：sample description box，样本的描述信息。
// stts：time to sample box，sample解码时间的压缩表。
// ctts：composition time to sample box，sample的CTS与DTS的时间差的压缩表。
// stss：sync sample box，针对视频，关键帧的序号。
// stsz/stz2：sample size box，每个sample的字节大小。
// CMakeLists.txtstsc：sample to chunk box，sample-chunk映射表。
// stco/co64：chunk offset box，chunk在文件中的偏移。
// container:minf
struct StblBox:public Box
{
    StblBox(uint32_t box_size):Box(BoxType::STBL, box_size)
    {

    }
};

struct Entry
{
    uint32_t sample_count_;
    uint32_t sample_delta_;
};

// container:stbl
struct SttsBox:public FullBox
{
    SttsBox(uint32_t box_size):FullBox(BoxType::STTS, box_size, 0)
    {
    }
    SttsBox(uint32_t box_size, std::vector<uint8_t> data): FullBox(BoxType::STTS, box_size, data)
    {
        //entry_count_ = ShowU32(data.data() + parse_len_);
        //LOG_DEBUG("stts entry_count_:{}", entry_count_);
        parse_len_ += 4;
        /*
        for(auto i = 0; i < entry_count_; ++i)
        {
            Entry* entry = new Entry();
            entry->sample_count_ = ShowU32(data.data() + parse_len_);
            parse_len_ += 4;
            entry->sample_delta_ = ShowU32(data.data() + parse_len_);
            parse_len_ += 4;
            entrys_.push_back(entry);
        }
        */
    }
    uint32_t entry_count_;
    std::vector<Entry*> entrys_;
};

// container:stbl
struct Ctts:public FullBox
{
    Ctts(uint32_t box_size):FullBox(BoxType::CTSS, box_size, 0)
    {
    }
    uint32_t entry_count_;
    std::vector<Entry> entrys_;
};

struct SampleEntry : public Box
{
    SampleEntry(uint32_t format, uint32_t box_size):Box(format, box_size)
    {
    }
    uint8_t reserved_[6] = {0};
    uint16_t data_reference_index_;
};

struct HintSampleEntry : public SampleEntry
{
    HintSampleEntry(uint32_t protocol, uint32_t box_size) : SampleEntry(protocol, box_size)
    {

    }
    uint8_t * data;
};

struct VisaulSampleEntry: public SampleEntry
{
    VisaulSampleEntry(uint32_t codingname, uint32_t box_size):SampleEntry(codingname, box_size)
    {

    }
    uint16_t pre_defined_ = 0;
    const uint16_t reserved_ = 0;
    uint32_t pre_defined2_[3] = {0};
    uint16_t width_;
    uint16_t height_;
    uint32_t horizresolution = 0x00480000; // 72dpi
    uint32_t vertresolution = 0x00480000; // 72dpi
    uint32_t reserved2_ = 0;
    const uint16_t  frame_count_ = 1;
    char compressorname[32];
    uint16_t depth = 0x0018;
    int16_t pre_defined3_ = -1;
};

class AudioSampleEntry:public SampleEntry
{
    AudioSampleEntry(uint32_t codingname, uint32_t box_size) : SampleEntry(codingname, box_size)
    {
    }
    uint32_t reserved_[2] = {0};
    uint16_t channel_count_ = 2;
    uint16_t sample_size_ = 16;
    uint16_t pre_defined_ = 0;
    uint16_t reserved2_ = 0;
    // timescale of media << 16
    uint32_t samplerate_;
};

// 存储了编码类型和初始化解码器需要的信息
// container:stbl
struct StsdBox: public FullBox
{
    StsdBox(uint32_t box_size):FullBox(BoxType::STSD, box_size, 0)
    {

    }
    int i;
    uint32_t entry_count_; // entry的个数
    // 根据handler_type确定时哪种entry
    SampleEntry** entrys_;
};


// container : stbl
struct StszBox:public FullBox
{
    StszBox(uint32_t box_size):FullBox(BoxType::STSZ, box_size, 0)
    {

    }
    uint32_t sample_size_;
    uint32_t sample_count_;
    // if sample_size_ == 0;
    // for(i =0; i< sample_count_; ++i)
    // {
    //  uint32_t entry_size;
    // }
    uint32_t* entry_size_;
};

// container : stbl
struct Stz2Box:public FullBox
{
    Stz2Box(uint32_t box_size):FullBox(BoxType::STZ2, box_size, 0)
    {

    }
    uint8_t reserved_[3] = {0};
    uint8_t file_size_;
    uint32_t sample_count_;
    // for(i =0; i< sample_count_; ++i)
    // {
    //      unsigned int(field_size) entry_size;
    // }
    uint32_t* entry_size_;
};

struct SampleToChunk
{
    uint32_t first_chunk_;
    uint32_t samples_per_chunk_;
    uint32_t samples_description_index_;
};

// container : stbl
// sample to chunk映射表
struct StscBox:public FullBox
{
    StscBox(uint32_t box_size):FullBox(BoxType::STSC, box_size, 0)
    {
    }
    uint32_t entry_count_;
    // for(i =1; i<= entry_count_; ++i)
    // {
    //      SampleToChunk;
    // }
    SampleToChunk * entry_;
};

// container:stbl
// chunk offset box
struct StcoBox:public FullBox
{
    StcoBox(uint32_t box_size):FullBox(BoxType::STCO, box_size, 0)
    {

    }
    uint32_t entry_count_;
    // for(i =1; i<= entry_count_; ++i)
    // {
    //      chunk_offset_;
    // }
    uint32_t* chunk_offset_;
};

// container:stbl
// chunk offset box
struct Co64Box:public FullBox
{
    Co64Box(uint32_t box_size):FullBox(BoxType::CO64, box_size, 0)
    {

    }
    uint32_t entry_count_;
    // for(i =1; i<= entry_count_; ++i)
    // {
    //      chunk_offset_;
    // }
    uint64_t* chunk_offset_;
};

// container:stbl
struct StssBox:public FullBox
{
    StssBox(uint32_t box_size):FullBox(BoxType::STSS, box_size, 0)
    {
    };
    StssBox(uint32_t box_size, std::vector<uint8_t> data): FullBox(BoxType::STSS, box_size, data)
    {
        entry_count_ = ShowU32(data.data() + parse_len_);
        parse_len_ += 4;
        for(auto i = 0; i < entry_count_; ++i)
        { 
        }
    }
    uint32_t entry_count_;
    // for(i =1; i<= entry_count_; ++i)
    // {
    //      chunk_offset_;
    // }
    uint32_t* sample_number_;
};

struct StshEntry
{
    uint32_t shadowed_sample_number_;
    uint32_t sync_sample_number_;
};

// container:stbl
struct StshBox:public FullBox
{
    StshBox(uint32_t box_size):FullBox(BoxType::STSH, box_size, 0)
    {

    };
    uint32_t entry_count_;
    // for(i =1; i<= entry_count_; ++i)
    // {
    //      StshEntry;
    // }
    StshEntry* entrys_;
};

// container:stbl
struct StdpBox:public FullBox
{
    StdpBox(uint32_t box_size):FullBox(BoxType::STDP, box_size, 0)
    {

    };
    int i;
    // for(i =0; i<sample_count_; ++i)
    // {
    //      StshEntry;
    // }
    uint16_t prioritys_;
};

struct PaddingBits
{
    unsigned reserved_:1;   // 0
    unsigned pad1_:3;
    unsigned reserved2_:1;
    unsigned pad2_:3;
};

// container:stbl
struct PadbBox:public FullBox
{
    PadbBox(uint32_t box_size):FullBox(BoxType::PADB, box_size, 0)
    {

    };
    uint32_t sample_count_;
    // for(i =0; i<((sample_count_+1)/2); ++i)
    // {
    //      PaddingBits;
    // }
    PaddingBits padding_bits_;
};

// container：file or other box
struct FreeBox: public Box
{
    FreeBox(uint32_t box_size):Box(BoxType::FREE, box_size)
    {
    }
    uint8_t * data_;
};

// 解析到elst，之后的暂时不需要，先不解析了
//
struct Dinf:public Box
{
};

// container box
struct TrackBox:public Box
{
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

        // 获取box type和size
        bool get_box_size_type(uint32_t& box_size, uint32_t& box_type);

        // 获取box large size
        bool get_box_large_size(uint64_t& large_size);

        // 读取box的数据，不包含box_type和box_size
        bool read_box_data(uint32_t box_size, std::vector<uint8_t>& data);

    public:
        std::string file_name_;
        std::fstream fs_;
        std::map<int, Box*> boxes_;
        size_t file_size_;
};

#endif  // MP4PARSER_H_
