/*************************************************************************
	> File Name: ByteUtil.h
	> Author: pzx
	> Created Time: 2019年02月21日 星期四 11时51分56秒
************************************************************************/
#ifndef __BYTEUTIL_H__
#define __BYTEUTIL_H__

inline unsigned int ShowU32(unsigned char *pBuf) 
{
    return (pBuf[0] << 24) | (pBuf[1] << 16) | (pBuf[2] << 8) | pBuf[3]; 
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

inline void WriteU64(uint64_t & x, int length, int value)
{
    uint64_t mask = 0xFFFFFFFFFFFFFFFF >> (64 - length);
    x = (x << length) | ((uint64_t)value & mask);
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
#endif //__BYTEUTIL_H__
