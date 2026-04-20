/*
 * MediaFormat.h
 *
 *  Created on: 2016年1月12日
 *      Author: terry
 */

#ifndef MEDIAFORMAT_H_
#define MEDIAFORMAT_H_

#include "MediaType.h"
#include "ByteBuffer.h"


namespace av
{


class RawMediaFormat
{
public:
	int m_width;
	int m_height;
	int m_framerate;
	int m_pixelFormat;

	int m_channels;
	int m_sampleRate;
	int m_sampleFormat;

	RawMediaFormat():
		m_width(),
		m_height(),
		m_framerate(),
		m_pixelFormat(),
		m_channels(),
		m_sampleRate(),
		m_sampleFormat()
	{
	}
};


class DLLEXPORT MediaFormat
{
public:
	int m_duration;         // millisecond

	int m_codec;
	int m_width;
	int m_height;
	int m_framerate;
	int m_bitrate;
	int m_profile;			/// video codec profile
    int m_clockRate;

	int m_audioCodec;
	int m_audioProfile;		/// audio codec profile
	int m_channels;
	int m_sampleRate;
	int m_sampleBits;
	int m_frameSize;
	int m_audioBitrate;
    int m_audioRate;

	ByteBuffer m_videoProp;
	ByteBuffer m_audioConfig;

	MediaFormat():
		m_duration(),
		m_codec(),
		m_width(),
		m_height(),
		m_framerate(),
		m_bitrate(),
		m_profile(),
        m_clockRate(),
		m_audioCodec(),
		m_audioProfile(),
		m_channels(0),
		m_sampleRate(0),
		m_sampleBits(16),
		m_frameSize(1024),
		m_audioBitrate(),
		m_videoProp(),
		m_audioConfig(),
        m_audioRate()
	{
	}

	~MediaFormat()
	{
	}

	bool hasVideo() const
	{
		return (m_codec != 0);
	}

	bool hasAudio() const
	{
		return (m_audioCodec != 0);
	}

	bool isValid() const
	{
		return (m_codec != 0) || (m_audioCodec != 0);
	}

	int getAudioBitrate() const
	{
		return m_sampleRate * m_channels * m_sampleBits;
	}

	size_t getVideoPropSize() const
	{
		return m_videoProp.size();
	}

	const uint8_t* getVideoProp() const
	{
		return (const uint8_t*)m_videoProp.c_str();
	}

	size_t getAudioConfigSize() const
	{
		return m_audioConfig.size();
	}

	const uint8_t* getAudioConfig() const
	{
		return (const uint8_t*)m_audioConfig.c_str();
	}

	void setVideoProp(const uint8_t* data, size_t length)
	{
		m_videoProp.assign((const char*)data, length);
	}

	void setVideoProp(const char* data, size_t length)
	{
		m_videoProp.assign(data, length);
	}

	void setAudioConfig(const uint8_t* data, size_t length)
	{
		m_audioConfig.assign((const char*)data, length);
	}

	void setAudioConfig(const char* data, size_t length)
	{
		m_audioConfig.assign(data, length);
	}

	void clear()
	{
		m_duration = 0;
		m_codec = 0;
		m_width = 0;
		m_height = 0;
		m_framerate = 0;
		m_bitrate = 0;

		m_audioCodec = 0;
		m_audioProfile = 0;
		m_channels = 0;
		m_sampleRate = 0;
		m_sampleBits = 16;
		m_frameSize = 1024;
		m_audioBitrate = 0;

		m_videoProp.clear();
		m_audioConfig.clear();
	}


};



} /* namespace av */

#endif /* MEDIAFORMAT_H_ */
