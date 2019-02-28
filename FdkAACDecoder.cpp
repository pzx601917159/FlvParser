#include "FdkAACDecoder.h"
#include <vector>
#include "SDLPlayer.h"

FdkAACDecoder::FdkAACDecoder()
{
	_h.dec = NULL;
    pos_ = 0;
    aacdec_init_adts();
}

FdkAACDecoder::~FdkAACDecoder()
{
	aacdec_close();
}

int FdkAACDecoder::init()
{
    return 0;
}

int FdkAACDecoder::destory()
{
    return 0;
}

int FdkAACDecoder::decodeFrame(unsigned char* frameData, unsigned int frameSize, int* width, int* height, int* pixFmt, int pts)
{
    std::vector<char> aac_buf(frameSize,0);
    memcpy(&aac_buf[0], frameData, frameSize);
    aacBuf_.insert(aacBuf_.end(), aac_buf.begin(),aac_buf.end());
    int pcmSize = aacdec_pcm_size();
    if(pcmSize <= 0)
    {
        pcmSize = 50 * 1024;
    }
    std::vector<char> pcm_buf(pcmSize, 0);
    while(1)
    {
        if(aacBuf_.size() - pos_ < 7)
        {
            break;
        }
        adts_header_t* adts = (adts_header_t*)(&aacBuf_[0] + pos_);
        //printf("decode aac frame\n");
        if (adts->syncword_0_to_8 != 0xff || adts->syncword_9_to_12 != 0xf) 
        {
            //printf("break\n");
            break;
        }
  
        int aac_frame_size = adts->frame_length_0_to_1 << 11 | adts->frame_length_2_to_9 << 3 | adts->frame_length_10_to_12;

        if(pos_ + aac_frame_size > aacBuf_.size())
        {
            //printf("no enough audio\n");
            break;
        }
        int leftSize = aac_frame_size;
        int ret = aacdec_fill(&aacBuf_[0] + pos_, aac_frame_size, &leftSize);
        pos_ += aac_frame_size;
        //printf("ret = %x leftSize:%d\n",ret, leftSize);
  
        if (ret != 0)
        {
            continue;
        }
  
        if (leftSize > 0)
        {
            continue;
        }
  
        int validSize = 0;
        ret = aacdec_decode_frame(&pcm_buf[0], pcm_buf.size(), &validSize);
        //printf("ret = %x\n",ret);
        if(ret == AAC_DEC_OK)
        {
            // 得到pts
            //printf("acc decode success");
            SDLPlayer::instance()->playAudio((unsigned char*)&(pcm_buf[0]), validSize);
        }
        else
        {
            //printf("aac decode fail\n");
        }
  
        if (ret == AAC_DEC_NOT_ENOUGH_BITS) {
            continue;
        }
  
        if (ret != 0) {
            continue;
        }

        return 0;
    }
}


int FdkAACDecoder::aacdec_init_adts()
{
	_h.sample_bits = 16;
	_h.is_adts = 0;
	_h.filled_bytes = 0;

	_h.dec = aacDecoder_Open(TT_MP4_ADTS, 1);
	if (!_h.dec) {
		return -1;
	}

	_h.info = NULL;

	return 0;
}

void FdkAACDecoder::aacdec_close()
{
	if (_h.dec) {
		aacDecoder_Close(_h.dec);
	}
	_h.dec = NULL;
}

int FdkAACDecoder::aacdec_fill(char *data, int nb_data, int *pnb_left)
{
	_h.filled_bytes += nb_data;

	unsigned char *udata = (unsigned char *)data;
	unsigned int unb_data = (unsigned int)nb_data;
	unsigned int unb_left = unb_data;
	AAC_DECODER_ERROR err = aacDecoder_Fill(_h.dec, &udata, &unb_data, &unb_left);
	if (err != AAC_DEC_OK) 
    {
        //printf("aacDecoder fill:%x\n", err);
		return err;
	}

	if (pnb_left) 
    {
		*pnb_left = (int)unb_left;
	}

	return 0;
}

int FdkAACDecoder::aacdec_sample_bits()
{
	return _h.sample_bits;
}

int FdkAACDecoder::aacdec_pcm_size()
{
	if (!_h.info) {
		return 0;
	}
	return (int)(_h.info->numChannels * _h.info->frameSize * _h.sample_bits / 8);
}

int FdkAACDecoder::aacdec_decode_frame(char *pcm, int nb_pcm, int *pnb_valid)
{
	// when buffer left bytes not enough, directly return not-enough-bits.
	// we requires atleast 7bytes header for adts.
	if (_h.is_adts && _h.info && _h.filled_bytes - _h.info->numTotalBytes <= 7) {
		return AAC_DEC_NOT_ENOUGH_BITS;
	}

	INT_PCM* upcm = (INT_PCM*)pcm;
	int unb_pcm = (int)nb_pcm;
	AAC_DECODER_ERROR err = aacDecoder_DecodeFrame(_h.dec, upcm, unb_pcm, 0);
	// user should fill more bytes then decode.
	if (err == AAC_DEC_NOT_ENOUGH_BITS) {
		return err;
	}
	if (err != AAC_DEC_OK) {
		return err;
	}

	// when decode ok, retrieve the info.
	if (!_h.info) {
		_h.info = aacDecoder_GetStreamInfo(_h.dec);
	}

	// the actual size of pcm.
	if (pnb_valid) {
		*pnb_valid = aacdec_pcm_size();
	}

	return 0;
}

int FdkAACDecoder::aacdec_sample_rate()
{
	if (!_h.info) {
		return 0;
	}
	return _h.info->sampleRate;
}

int FdkAACDecoder::aacdec_frame_size()
{
	if (!_h.info) {
		return 0;
	}
	return _h.info->frameSize;
}

int FdkAACDecoder::aacdec_num_channels()
{
	if (!_h.info) {
		return 0;
	}
	return _h.info->numChannels;
}

int FdkAACDecoder::aacdec_aac_sample_rate()
{
	if (!_h.info) {
		return 0;
	}
	return _h.info->aacSampleRate;
}

int FdkAACDecoder::aacdec_profile()
{
	if (!_h.info) {
		return 0;
	}
	return _h.info->profile;
}

int FdkAACDecoder::aacdec_audio_object_type()
{
	if (!_h.info) {
		return 0;
	}
	return _h.info->aot;
}

int FdkAACDecoder::aacdec_channel_config()
{
	if (!_h.info) {
		return 0;
	}
	return _h.info->channelConfig;
}

int FdkAACDecoder::aacdec_bitrate()
{
	if (!_h.info) {
		return 0;
	}
	return _h.info->bitRate;
}

int FdkAACDecoder::aacdec_aac_samples_per_frame()
{
	if (!_h.info) {
		return 0;
	}
	return _h.info->aacSamplesPerFrame;
}

int FdkAACDecoder::aacdec_aac_num_channels()
{
	if (!_h.info) {
		return 0;
	}
	return _h.info->aacNumChannels;
}

int FdkAACDecoder::aacdec_extension_audio_object_type()
{
	if (!_h.info) {
		return 0;
	}
	return _h.info->extAot;
}

int FdkAACDecoder::aacdec_extension_sampling_rate()
{
	if (!_h.info) {
		return 0;
	}
	return _h.info->extSamplingRate;
}

int FdkAACDecoder::aacdec_num_lost_access_units()
{
	if (!_h.info) {
		return 0;
	}
	return _h.info->numLostAccessUnits;
}

int FdkAACDecoder::aacdec_num_total_bytes()
{
	if (!_h.info) {
		return 0;
	}
	return _h.info->numTotalBytes;
}

int FdkAACDecoder::aacdec_num_bad_bytes()
{
	if (!_h.info) {
		return 0;
	}
	return _h.info->numBadBytes;
}

int FdkAACDecoder::aacdec_num_total_access_units()
{
	if (!_h.info) {
		return 0;
	}
	return _h.info->numTotalAccessUnits;
}

int FdkAACDecoder::aacdec_num_bad_access_units()
{
	if (!_h.info) {
		return 0;
	}
	return _h.info->numBadAccessUnits;
}
