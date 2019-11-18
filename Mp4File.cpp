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
    return true;
}

// 解析文件
int Mp4File::parse()
{
    uint32_t left_size = 0;
    uint32_t parse_len = 0;
    uint32_t box_size = 0;
    uint32_t box_type = 0;
    int depth = 0;
    Box* parent_ = nullptr;
    std::vector<uint8_t> data;
    while(!fs_.eof())
    {
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
                    parent_ = nullptr;
                    read_box_data(box_size, data);
                    // 解析完header
                    FtypBox* box = new FtypBox();
                    // 解析box
                    box->parse(data);
                    LOG_DEBUG("major_band_:{}", intToStr(box->major_band_));
                    LOG_DEBUG("minor version:{}", intToStr(box->minor_version_));
                    LOG_DEBUG("compatible_brands_:{}",intToStr(box->compatible_brands_));
                    boxes_[box_type] = box;
                    break;
                }
                case BoxType::FREE:
                {
                    read_box_data(box_size, data);
                    parent_ = nullptr;
                    break;
                }
                case BoxType::MDAT:
                {
                    read_box_data(box_size, data);
                    parent_ = nullptr;
                    break;
                }
                case BoxType::PDIN:
                {
                    read_box_data(box_size, data);
                    parent_ = nullptr;
                    break;
                }
                case BoxType::MOOF:
                {
                    read_box_data(box_size, data);
                    parent_ = nullptr;
                    break;
                }
                case BoxType::MFRA:
                {
                    read_box_data(box_size, data);
                    parent_ = nullptr;
                    break;
                }
                case BoxType::SKIP:
                {
                    read_box_data(box_size, data);
                    parent_ = nullptr;
                    break;
                }
                case BoxType::MOOV:
                {
                    parent_ = nullptr;
                    MoovBox* moovBox = new MoovBox();
                    boxes_[box_type] = moovBox;
                    parent_ = moovBox;
                    // 解析完所有的数据
                    break;
                }
                case BoxType::MVHD:
                {
                    // 分配足够的内存
                    data.reserve(box_size - 8);
                    // 读取box body
                    fs_.read((char*)data.data(), box_size - 8);
                    /*
                    MvhdBox box;
                    parse_len += (moov_box_size - 8);
                    // 这也时container box
                    // 继续解析
                    box_size = ShowU32((unsigned char*)moovBox.data() + parse_len);
                    std::cout << "mvhd box size:" << box_size << std::endl;
                    parse_len += 4;
                    memcpy(box_type, moovBox.data() + parse_len, 4);
                    parse_len += 4;
                    std::cout << "mvhd container box:" << box_type << std::endl;
                    */
                    break;
                }
                case BoxType::TRAK:
                {
                    // 分配足够的内存
                    // data.reserve(box_size - 8);
                    // 读取box body
                    // fs_.read((char*)data.data(), box_size - 8);
                    // TODO
                    break;
                }
                // 这里区分时音频还是视频
                case BoxType::HDLR:
                {
                    read_box_data(box_size, data);
                    HdlrBox* box = new HdlrBox();
                    box->parse(data);
                    LOG_DEBUG("handler_type:{}", uint32ToString(box->handler_type_));
                }
                case BoxType::MDIA:
                {
                    break;
                }
                case BoxType::MINF:
                {
                    break;
                }
                case BoxType::STBL:
                {
                    break;
                }
                default:
                {
                    // 分配足够的内存
                    data.reserve(box_size - 8);
                    // 读取box body
                    fs_.read((char*)data.data(), box_size - 8);
                    // 相当于metadata
                    break;
                }
            }
        }
    }
    return 0;
}

