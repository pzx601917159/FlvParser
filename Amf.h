/*************************************************************************
	> File Name: Amf.h
	> Author: pzx
	> Created Time: 2019年02月21日 星期四 17时32分12秒
************************************************************************/
#ifndef AMF_H_
#define AMF_H_
#include "ByteUtil.h"
#include <string>
enum AMFDataType
{
    AMF_DATA_TYPE_NUMBER      = 0x00,
    AMF_DATA_TYPE_BOOL        = 0x01,
    AMF_DATA_TYPE_STRING      = 0x02,
    AMF_DATA_TYPE_OBJECT      = 0x03,
    AMF_DATA_TYPE_NULL        = 0x05,
    AMF_DATA_TYPE_UNDEFINED   = 0x06,
    AMF_DATA_TYPE_REFERENCE   = 0x07,
    AMF_DATA_TYPE_MIXEDARRAY  = 0x08,
    AMF_DATA_TYPE_OBJECT_END  = 0x09,
    AMF_DATA_TYPE_ARRAY       = 0x0a,
    AMF_DATA_TYPE_DATE        = 0x0b,
    AMF_DATA_TYPE_LONG_STRING = 0x0c,
    AMF_DATA_TYPE_UNSUPPORTED = 0x0d,
};

inline std::string amfGetString(unsigned char* buf, uint32_t size)
{
    int length = ShowU16(buf);
    if(length > size)
    {
        return "";
    }
    std::string str(reinterpret_cast<char*>(buf+2), length);
    return str;
}


#endif  // AMF_H_
