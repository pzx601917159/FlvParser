/*************************************************************************
	> File Name: Mp4Parser.h
	> Author: pzx
	> Created Time: 2019年03月04日 星期一 14时09分32秒
************************************************************************/
#include "Mp4File.h"
#include <iostream>
#include "ByteUtil.h"
#include "log.h"

int Mp4File::init(const std::string file_name)
{
    file_name_ = file_name;
    if(file_name_.empty())
    {
        return -1;
    }
    fs_.open(file_name_, std::ios_base::in | std::ios_base::binary);
    if(!fs_ || !fs_.is_open() || !fs_.good())
    {
        return -1;
    }
    fs_.seekg(0, fs_.end);
    file_size_ = fs_.tellg();
    spdlog::set_level(spdlog::level::debug);
    LOG_DEBUG("file size222:{}", file_size_);
    fs_.seekg(0);
    return 0;
}

bool Mp4File::get_box_size_type(uint32_t& box_size, uint32_t& box_type)
{
    fs_.read((char*)&box_size, sizeof(uint32_t));
    box_size = ShowU32((unsigned char*)&box_size);
    int read_len = fs_.gcount();
    if(read_len < sizeof(uint32_t))
    {
        return false;
    }
    if(box_size > file_size_)
    {
        return false;
    }
    fs_.read((char*)&box_type, sizeof(uint32_t));
    read_len = fs_.gcount();
    if(read_len < sizeof(uint32_t))
    {
        return false;
    }
    return true;
}

bool Mp4File::get_box_large_size(uint64_t& large_size)
{
    // TODO IMP

}

// 读取box的数据，不包含box_type和box_size
bool Mp4File::read_box_data(uint32_t box_size, std::vector<uint8_t>& data)
{
    if(box_size <= 8)
    {
        return true;
    }
    data.reserve(box_size - 8);
    fs_.read((char*)data.data(), box_size - 8);
    if(fs_.gcount() != box_size -8)
    {
        return false;
    }
    LOG_DEBUG("read success, box_size:{} data size:{}", box_size, data.size());
    return true;
}

// 解析文件
int Mp4File::parse()
{
    parse(nullptr, file_size_);
}

// 解析文件
int Mp4File::parse(Box* parent, uint32_t parent_size)
{
    uint32_t left_size = 0;
    uint32_t parse_len = 0;
    uint32_t box_size = 0;
    uint32_t box_type = 0;
    int depth = 0;
    bool read_res = true;
    uint32_t read_parent_size = 0;
    std::vector<uint8_t> data;
    while(!fs_.eof())
    {
        read_parent_size += box_size;
        LOG_DEBUG("read_parent_size:{} parent_size:{}", read_parent_size, parent_size);
        if(read_parent_size == parent_size)
        {
            if(parent != nullptr)
            {
                LOG_DEBUG("====read_box complete parent:{} child:{}", uint32ToString(parent->type_), uint32ToString(box_type));
            }
            break;
        }
        get_box_size_type(box_size, box_type);
        LOG_DEBUG("boxtype:{}",uint32ToString(box_type));
        parse_len += 8;
        // 则包含large size
        if(box_size == 1)
        {
            //get_box_large_size();
        }
        else
        {
            switch(box_type)
            {
                // todo switch case int
                case BoxType::FTYP:
                {
                    read_box_data(box_size, data);
                    // 解析完header
                    FtypBox* box = new FtypBox(box_size, data);
                    box->set_parent(parent);
                    boxes_[box_type] = box;
                    break;
                }
                case BoxType::FREE:
                {
                    read_box_data(box_size, data);
                    FreeBox* box = new FreeBox(box_size, data);
                    box->set_parent(parent);
                    boxes_[box_type] = box;
                    break;
                }
                case BoxType::MDAT:
                {
                    read_box_data(box_size, data);
                    break;
                }
                case BoxType::PDIN:
                {
                    read_box_data(box_size, data);
                    break;
                }
                case BoxType::MOOF:
                {
                    read_box_data(box_size, data);
                    break;
                }
                case BoxType::MFRA:
                {
                    read_box_data(box_size, data);
                    break;
                }
                case BoxType::SKIP:
                {
                    read_box_data(box_size, data);
                    break;
                }
                case BoxType::MOOV:
                {
                    MoovBox* moov_box = new MoovBox(box_size);
                    moov_box->set_parent(nullptr);
                    boxes_[box_type] = moov_box;
                    parse(moov_box, box_size - 8);
                    break;
                }
                case BoxType::MVHD:
                {
                    read_box_data(box_size, data);
                    MvhdBox* mvhd_box = new MvhdBox(box_size, data);
                    mvhd_box->set_parent(parent);
                    break;
                }
                case BoxType::TRAK:
                {
                    // 不读取数据，container box
                    TrakBox* box = new TrakBox(box_size);
                    box->set_parent(parent);
                    parse(box, box_size - 8);
                    break;
                }
                case BoxType::TKHD:
                {
                    read_box_data(box_size, data);
                    TkhdBox* box = new TkhdBox(box_size, data);
                    box->set_parent(parent);
                    break;
                }
                // 这里区分时音频还是视频
                case BoxType::HDLR:
                {
                    read_box_data(box_size, data);
                    HdlrBox* box = new HdlrBox(box_size, data);
                    box->set_parent(parent);
                    break;
                }
                case BoxType::MDIA:
                {
                    MdiaBox *mdia_box = new MdiaBox(box_size);
                    mdia_box->set_parent(parent);
                    parse(mdia_box, box_size - 8);
                    break;
                }
                case BoxType::MINF:
                {
                    MinfBox *box = new MinfBox(box_size);
                    box->set_parent(parent);
                    parse(box, box_size - 8);
                    break;
                }
                case BoxType::STBL:
                {
                    StblBox * box = new StblBox(box_size);
                    box->set_parent(parent);
                    parse(box, box_size - 8);
                    break;
                }
                case BoxType::MDHD:
                {
                    read_box_data(box_size, data);
                    MdhdBox* box = new MdhdBox(box_size, data);
                    box->set_parent(parent);
                    break;
                }
                case BoxType::STSD:
                {
                    read_box_data(box_size, data);
                    StsdBox* box = new StsdBox(box_size, data, parent);
                    break;
                }
                case BoxType::STTS:
                {
                    read_box_data(box_size, data);
                    SttsBox* box = new SttsBox(box_size, data);
                    box->set_parent(parent);
                    break;
                }
                case BoxType::STSS:
                {
                    read_box_data(box_size, data);
                    StssBox* box = new StssBox(box_size, data);
                    box->set_parent(parent);
                    break;
                }
                case BoxType::CTSS:
                {
                    read_box_data(box_size, data);
                    CtssBox* box = new CtssBox(box_size, data);
                    box->set_parent(parent);
                    break;
                }
                case BoxType::STSC:
                {
                    read_box_data(box_size, data);
                    StscBox* box = new StscBox(box_size, data);
                    box->set_parent(parent);
                    break;
                }
                case BoxType::STSZ:
                {
                    read_box_data(box_size, data);
                    StszBox* box = new StszBox(box_size, data);
                    box->set_parent(parent);
                    break;
                }
                case BoxType::STCO:
                {
                    read_box_data(box_size, data);
                    StcoBox* box = new StcoBox(box_size, data);
                    box->set_parent(parent);
                    break;
                }
                default:
                {
                    read_box_data(box_size, data);
                    break;
                }
            }
        }
    }
    return 0;
}

// 获取video trak
Box* Mp4File::get_video_trak()
{
    Box* moov = boxes_[BoxType::MOOV];
    for(auto box : moov->children_)
    {
        if(box->type_ == BoxType::TRAK)
        {
            LOG_DEBUG("find trak box");
            for(auto child : box->children_)
            {
                if(child->type_ == BoxType::MDIA)
                {
                    LOG_DEBUG("find mdia box");
                    for(auto hdlr:child->children_)
                    {
                        if(hdlr->type_ == BoxType::HDLR &&
                                !memcmp(&((HdlrBox*)hdlr)->handler_type_, "vide", 4))
                        {
                            LOG_DEBUG("find hdlr box");
                            return box;
                        }
                    }
                }
            }
        }
    }
    LOG_DEBUG("cannnot find video trak");
    return nullptr;
}

Box* Mp4File::get_box(BoxType box_type, Box* box)
{
    LOG_DEBUG("find box type:{}",uint32ToString(box->type_));
    LOG_DEBUG("find box type2:{}",uint32ToString(box_type));
    if(box->type_ == box_type)
    {
        return box;
    }
    for(auto child:box->children_)
    {
        Box* box = get_box(box_type, child);
        if(box != nullptr)
        {
            return box;
        }
    }
    return nullptr;
}

// 根据时间获取文件偏移
uint32_t Mp4File::get_offset(double duration)
{
    uint32_t offset = 0;
    do
    {
        // 1.获取到video_trak
        TrakBox* video_trak = (TrakBox*)get_video_trak();
        if(video_trak == nullptr)
        {
            LOG_DEBUG("there is no video trak");
            break;
        }
        // 2.获取mdhd中的time scale值，得到对应时间基的duration 
        MdhdBox* mdhd_box = (MdhdBox*)get_box(BoxType::MDHD, video_trak);
        if(mdhd_box == nullptr)
        {
            LOG_DEBUG("there is no mdhd box");
            break;
        }
        duration = duration * mdhd_box->timescale_;
        LOG_DEBUG("time scale duration:{}", duration);
        // 3.通过time-to-sample box找到指定track的给定时间之前的第一个sample number
        SttsBox* stss_box = (SttsBox*)get_box(BoxType::STSS, video_trak);
        uint32_t sample_number = stss_box->get_sample_number(duration);
        LOG_DEBUG("prev sample number:{}", sample_number);
    }while(0);
    return offset;
}

void Mp4File::show_box(Box* box, int level)
{
    std::string str;
    for(auto i = 0; i < level; ++i)
    {
        str.append("----");
    }
    LOG_INFO("{}box type:{}", str, uint32ToString(box->type_));
    for(auto child:box->children_)
    {
        show_box(child, level + 1);
    }
}

void Mp4File::show_file()
{
    LOG_INFO("=====================");
    for(auto box:boxes_)
    {
        show_box(box.second, 0);
    }
    LOG_INFO("=====================");
}

