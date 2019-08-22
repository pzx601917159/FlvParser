/*************************************************************************
	> File Name: ByteUtil.h
	> Author: pzx
	> Created Time: 2019年02月21日 星期四 11时51分56秒
************************************************************************/
#ifndef BYTEUTIL_H_
#define BYTEUTIL_H_
#include <inttypes.h>
#include <string.h>
#include <string>

union int2double
{
    uint64_t i;
    double d;
};

inline unsigned int ShowU32(unsigned char *pBuf)
{
    return (pBuf[0] << 24) | (pBuf[1] << 16) | (pBuf[2] << 8) | pBuf[3];
}

inline uint64_t ShowU64(unsigned char* pBuf)
{
    return (((uint64_t)ShowU32(pBuf)) << 32) | ShowU32(pBuf + 4);
}

inline double ShowDouble(unsigned char* pBuf)
{
    uint64_t num = ShowU64(pBuf);
    union int2double v;
    v.i = num;
    return v.d;
}

inline unsigned int ShowU24(unsigned char *pBuf)
{
    return (pBuf[0] << 16) | (pBuf[1] << 8) | (pBuf[2]);
}

inline unsigned int ShowU16(unsigned char *pBuf)
{
    return (pBuf[0] << 8) | (pBuf[1]);
}

inline unsigned int ShowU8(unsigned char *pBuf)
{
    return (pBuf[0]);
}

inline void WriteU64(uint64_t* x, int length, int value)
{
    uint64_t mask = (uint64_t)0xFFFFFFFFFFFFFFFF >> (64 - length);
    *x = (*x << length) | ((uint64_t)value & mask);
}

inline unsigned int WriteU32(unsigned int n)
{
    unsigned int nn = 0;
    unsigned char *p = (unsigned char *)&n;
    unsigned char *pp = (unsigned char *)&nn;
    pp[0] = p[3];
    pp[1] = p[2];
    pp[2] = p[1];
    pp[3] = p[0];
    return nn;
}
struct BinaryData
{
    unsigned char* data_;
    uint32_t size_;
};

inline bool startWith(char* buf1, char* buf2)
{
    if(buf1 == NULL ||
        buf2 == NULL)
    {
        return false;
    }
    int len = strlen(buf2);
    if(len == 0)
    {
        return false;
    }
    if(strlen(buf1) < strlen(buf2))
    {
        return false;
    }
    for(int i = 0; i < len; ++i)
    {
        if(buf1[i] != buf2[i])
        {
            return false;
        }
    }
    return true;
}

inline std::string intToStr(uint32_t num)
{
    std::string str;
    str.append(1, reinterpret_cast<char *>(&num)[0]);
    str.append(1, reinterpret_cast<char *>(&num)[1]);
    str.append(1, reinterpret_cast<char *>(&num)[2]);
    str.append(1, reinterpret_cast<char *>(&num)[3]);
    return str;
}

#endif  // BYTEUTIL_H_
