//==============================================================================
//
//  OvenMediaEngine
//
//  Created by Kwon Keuk Han
//  Copyright (c) 2018 AirenSoft. All rights reserved.
//
//==============================================================================
#include "media_track.h"

#include <base/ovlibrary/converter.h>
#include <base/ovlibrary/ovlibrary.h>

#define OV_LOG_TAG "MediaTrack"

using namespace cmn;

MediaTrack::MediaTrack()
	: _id(0),
	  _media_type(MediaType::Unknown),
	  _codec_id(MediaCodecId::None),
	  _codec_library_id(cmn::MediaCodecLibraryId::AUTO),
	  _bitrate(0),
	  _bitrate_conf(0),
	  _byass(false),
	  _bypass_conf(false),
	  _start_frame_time(0),
	  _last_frame_time(0)
{
}

MediaTrack::MediaTrack(const MediaTrack &media_track)
{
	_id = media_track._id;
	_media_type = media_track._media_type;
	_codec_id = media_track._codec_id;
	_codec_library_id = media_track._codec_library_id;

	// Video
	_framerate = media_track._framerate;
	_framerate_conf = media_track._framerate_conf;

	_width = media_track._width;
	_width_conf  = media_track._width_conf;
	
	_height = media_track._height;
	_height_conf = media_track._height_conf;

	// Audio
	_sample = media_track._sample;
	_channel_layout = media_track._channel_layout;

	_time_base = media_track._time_base;

	_bitrate = media_track._bitrate;
	_bitrate_conf = media_track._bitrate_conf;

	_byass = media_track._byass;
	_bypass_conf = media_track._bypass_conf;

	_start_frame_time = 0;
	_last_frame_time = 0;

	_decoder_configuration_record = media_track._decoder_configuration_record;

	_origin_bitstream_format = media_track._origin_bitstream_format;
}

MediaTrack::~MediaTrack()
{
}

void MediaTrack::SetId(uint32_t id)
{
	_id = id;
}

uint32_t MediaTrack::GetId() const
{
	return _id;
}

// Track Name (used for Renditions)
void MediaTrack::SetVariantName(const ov::String &name)
{
	_variant_name = name;
}

ov::String MediaTrack::GetVariantName() const
{
	if (_variant_name.IsEmpty())
	{
		// If variant name is not set, return media type string
		return cmn::GetMediaTypeString(GetMediaType());
	}

	return _variant_name;
}

// Public Name (used for multiple audio/video tracks. e.g. multilingual audio)
void MediaTrack::SetPublicName(const ov::String &name)
{
	_public_name = name;
}
ov::String MediaTrack::GetPublicName() const
{
	return _public_name;
}

// Language (rfc5646)
void MediaTrack::SetLanguage(const ov::String &language)
{
	_language = language;
}
ov::String MediaTrack::GetLanguage() const
{
	return _language;
}

void MediaTrack::SetMediaType(MediaType type)
{
	_media_type = type;
}

MediaType MediaTrack::GetMediaType() const
{
	return _media_type;
}

void MediaTrack::SetCodecId(MediaCodecId id)
{
	_codec_id = id;
}

MediaCodecId MediaTrack::GetCodecId() const
{
	return _codec_id;
}

void MediaTrack::SetCodecLibraryId(cmn::MediaCodecLibraryId id)
{
	_codec_library_id = id;
}
cmn::MediaCodecLibraryId MediaTrack::GetCodecLibraryId() const
{
	return _codec_library_id;
}

void MediaTrack::SetOriginBitstream(cmn::BitstreamFormat format)
{
	_origin_bitstream_format = format;
}

cmn::BitstreamFormat MediaTrack::GetOriginBitstream() const
{
	return _origin_bitstream_format;
}

const cmn::Timebase &MediaTrack::GetTimeBase() const
{
	return _time_base;
}

void MediaTrack::SetTimeBase(int32_t num, int32_t den)
{
	_time_base.Set(num, den);
}

void MediaTrack::SetTimeBase(const cmn::Timebase &time_base)
{
	_time_base = time_base;
}

void MediaTrack::SetStartFrameTime(int64_t time)
{
	_start_frame_time = time;
}

int64_t MediaTrack::GetStartFrameTime() const
{
	return _start_frame_time;
}

void MediaTrack::SetLastFrameTime(int64_t time)
{
	_last_frame_time = time;
}

int64_t MediaTrack::GetLastFrameTime() const
{
	return _last_frame_time;
}

void MediaTrack::SetBypass(bool flag)
{
	_byass = flag;
}

bool MediaTrack::IsBypass() const
{
	return _byass;
}

std::shared_ptr<DecoderConfigurationRecord> MediaTrack::GetDecoderConfigurationRecord() const
{
	return _decoder_configuration_record;
}

void MediaTrack::SetDecoderConfigurationRecord(const std::shared_ptr<DecoderConfigurationRecord> &dcr)
{
	_decoder_configuration_record = dcr;
}

ov::String MediaTrack::GetCodecsParameter() const
{
	switch (GetCodecId())
	{
		case cmn::MediaCodecId::H264:
		case cmn::MediaCodecId::H265:
		case cmn::MediaCodecId::Aac:
		{
			auto config = GetDecoderConfigurationRecord();
			if (config != nullptr)
			{
				return config->GetCodecsParameter();
			}
			break;
		}
		
		case cmn::MediaCodecId::Opus:
		case cmn::MediaCodecId::Multiopus: 
		{
			// https://developer.mozilla.org/en-US/docs/Web/Media/Formats/codecs_parameter
			// In an MP4 container, the codecs parameter for Opus is "mp4a.ad"
			return "mp4a.ad";
		}

		case cmn::MediaCodecId::Vp8:
		{
			return "vp8";
		}

		case cmn::MediaCodecId::Vp9:
		{
			return "vp9";
		}

		case cmn::MediaCodecId::None:
		default:
			break;
	}

	return "";
}

ov::String MediaTrack::GetInfoString()
{
	ov::String out_str = "";

	switch (GetMediaType())
	{
		case MediaType::Video:
			out_str.AppendFormat(
				"Video Track #%d: "
				"Public Name(%s) "
				"Variant Name(%s) "
				"Bitrate(%s) "
				"Codec(%d,%s,%s) "
				"BSF(%s) "
				"Resolution(%dx%d) "
				"Framerate(%.2ffps) "
				"KeyInterval(%d) "
				"BFrames(%d) ",
				GetId(), GetPublicName().CStr(), GetVariantName().CStr(),
				ov::Converter::BitToString(GetBitrate()).CStr(),
				GetCodecId(), ::StringFromMediaCodecId(GetCodecId()).CStr(), IsBypass()?"Passthrough":GetStringFromCodecLibraryId(GetCodecLibraryId()).CStr(),
				GetBitstreamFormatString(GetOriginBitstream()).CStr(),
				GetWidth(), GetHeight(),
				GetFrameRate(),
				GetKeyFrameInterval(),
				GetBFrames());
			break;

		case MediaType::Audio:
			out_str.AppendFormat(
				"Audio Track #%d: "
				"Public Name(%s) "
				"Variant Name(%s) "
				"Bitrate(%s) "
				"Codec(%d,%s,%s) "
				"BSF(%s) "
				"Samplerate(%s) "
				"Format(%s, %d) "
				"Channel(%s, %d) ",
				GetId(), GetPublicName().CStr(), GetVariantName().CStr(),
				ov::Converter::BitToString(GetBitrate()).CStr(),
				GetCodecId(), ::StringFromMediaCodecId(GetCodecId()).CStr(), IsBypass()?"Passthrough":GetStringFromCodecLibraryId(GetCodecLibraryId()).CStr(),
				GetBitstreamFormatString(GetOriginBitstream()).CStr(),
				ov::Converter::ToSiString(GetSampleRate(), 1).CStr(),
				GetSample().GetName(), GetSample().GetSampleSize() * 8,
				GetChannel().GetName(), GetChannel().GetCounts());
			break;
		case MediaType::Data:
			out_str.AppendFormat(
				"Data  Track #%d: "
				"Public Name(%s) "
				"Variant Name(%s) "
				"Codec(%d,%s,%s) "
				"BSF(%s) ",
				GetId(), GetPublicName().CStr(), GetVariantName().CStr(),
				GetCodecId(), ::StringFromMediaCodecId(GetCodecId()).CStr(), IsBypass()?"Passthrough":GetStringFromCodecLibraryId(GetCodecLibraryId()).CStr(),
				GetBitstreamFormatString(GetOriginBitstream()).CStr());
			break;

		default:
			break;
	}

	out_str.AppendFormat("timebase(%s)", GetTimeBase().ToString().CStr());

	return out_str;
}

bool MediaTrack::IsValid()
{
	if (_is_valid == true)
	{
		return true;
	}

	// data type is always valid
	if(GetMediaType() == MediaType::Data)
	{
		_is_valid = true;
		return true;
	}

	switch (GetCodecId())
	{
		case MediaCodecId::H264: {
			if (_width > 0 &&
				_height > 0 &&
				_time_base.GetNum() > 0 &&
				_time_base.GetDen() > 0 && 
				GetDecoderConfigurationRecord() != nullptr)

			{
				_is_valid = true;
				return true;
			}
		}
		break;
		case MediaCodecId::H265: {
			if (_width > 0 &&
				_height > 0 &&
				_time_base.GetNum() > 0 &&
				_time_base.GetDen() > 0 &&
				GetDecoderConfigurationRecord() != nullptr)
			{
				_is_valid = true;
				return true;
			}
		}
		break;
		case MediaCodecId::Vp8: {
			if (_width > 0 &&
				_height > 0 &&
				_time_base.GetNum() > 0 &&
				_time_base.GetDen() > 0)
			{
				_is_valid = true;
				return true;
			}
		}
		break;
		case MediaCodecId::Vp9:
		case MediaCodecId::Flv: {
			if (_width > 0 &&
				_height > 0 &&
				_time_base.GetNum() > 0 &&
				_time_base.GetDen() > 0)
			{
				_is_valid = true;
				return true;
			}
		}
		break;
		case MediaCodecId::Jpeg:
		case MediaCodecId::Png: {
			if (_width > 0 &&
				_height > 0 &&
				_time_base.GetNum() > 0 &&
				_time_base.GetDen() > 0)
			{
				_is_valid = true;
				return true;
			}
		}
		break;
		case MediaCodecId::Aac: {
			if (_time_base.GetNum() > 0 &&
				_time_base.GetDen() > 0 &&
				_channel_layout.GetCounts() > 0 &&
				_channel_layout.GetLayout() > cmn::AudioChannel::Layout::LayoutUnknown && 
				GetDecoderConfigurationRecord() != nullptr)
			{
				_is_valid = true;
				return true;
			}
		}
		break;
		case MediaCodecId::Multiopus:
		case MediaCodecId::Opus: {
			if (_time_base.GetNum() > 0 &&
				_time_base.GetDen() > 0 &&
				_channel_layout.GetCounts() > 0 &&
				_channel_layout.GetLayout() > cmn::AudioChannel::Layout::LayoutUnknown &&
				_sample.GetRate() == cmn::AudioSample::Rate::R48000)
			{
				_is_valid = true;
				return true;
			}
		}
		break;
		case MediaCodecId::Mp3: {
			if (_time_base.GetNum() > 0 &&
				_time_base.GetDen() > 0 &&
				_channel_layout.GetCounts() > 0 &&
				_channel_layout.GetLayout() > cmn::AudioChannel::Layout::LayoutUnknown)
			{
				_is_valid = true;
				return true;
			}
		}
		break;

		default:
			break;
	}

	return false;
}

bool MediaTrack::HasQualityMeasured()
{
	if (_has_quality_measured == true)
	{
		return true;
	}

	switch (GetMediaType())
	{
		case MediaType::Video:
		{
			if (_bitrate > 0 &&	_framerate > 0.0)
			{
				_has_quality_measured = true;
			}
		}
		break;

		case MediaType::Audio:
		{
			if (_bitrate > 0)
			{
				_has_quality_measured = true;
			}
		}
		break;

		default:
			_has_quality_measured = true;
			break;
	}

	return _has_quality_measured;
}

void MediaTrack::OnFrameAdded(const std::shared_ptr<MediaPacket> &media_packet)
{
	if (_clock_from_first_frame_received.IsStart() == false)
	{
		_clock_from_first_frame_received.Start();
	}

	size_t bytes = media_packet->GetDataLength();

	_total_frame_count++;
	_total_frame_bytes += bytes;

	// If bitrate is not set, calculate bitrate
	if (_clock_from_first_frame_received.IsElapsed(VALID_BITRATE_CALCULATION_THRESHOLD_MSEC))
	{
		auto seconds = static_cast<double>(_clock_from_first_frame_received.Elapsed()) / 1000.0;
		auto bytes_per_second = static_cast<double>(_total_frame_bytes) / seconds;
		auto bitrate = static_cast<int32_t>(bytes_per_second * 8.0);
		SetBitrateByMeasured(bitrate);

		logtd("Track(%u) Bitrates(%s)", GetId(), ov::Converter::BitToString(bitrate).CStr());
	}

	// If framerate is not set, calculate framerate
	if (_clock_from_first_frame_received.IsElapsed(VALID_BITRATE_CALCULATION_THRESHOLD_MSEC))
	{
		auto seconds = static_cast<double>(_clock_from_first_frame_received.Elapsed()) / 1000.0;
		auto frame_count = static_cast<double>(_total_frame_count);
		auto framerate = frame_count / seconds;
		SetFrameRateByMeasured(framerate);

		logtd("Track(%u) FPS(%f)", GetId(), framerate);
	}

	if (GetMediaType() == cmn::MediaType::Video)
	{
		if (media_packet->GetFlag() == MediaPacketFlag::Key)
		{
			SetKeyFrameIntervalByMeasured(_key_frame_interval_count);
			_key_frame_interval_count = 1;
		}
		else if (_key_frame_interval_count > 0)
		{
			_key_frame_interval_count++;
		}
	}
}

int64_t MediaTrack::GetTotalFrameCount() const
{
	return _total_frame_count;
}

int64_t MediaTrack::GetTotalFrameBytes() const
{
	return _total_frame_bytes;
}

// void MediaTrack::SetBitrate(int32_t bitrate)
// {
// 	_bitrate = bitrate;
// }

int32_t MediaTrack::GetBitrate() const
{
	if (_bitrate_conf > 0)
	{
		return _bitrate_conf;
	}

	return _bitrate;
}

void MediaTrack::SetBitrateByMeasured(int32_t bitrate)
{
	_bitrate = bitrate;
}

int32_t MediaTrack::GetBitrateByMeasured() const
{
	return _bitrate;
}

void MediaTrack::SetBitrateByConfig(int32_t bitrate)
{
	_bitrate_conf = bitrate;
}

int32_t MediaTrack::GetBitrateByConfig() const
{
	return _bitrate_conf;
}

void MediaTrack::SetBypassByConfig(bool flag)
{
	_bypass_conf = flag;
}

bool MediaTrack::IsBypassByConf() const
{
	return _bypass_conf;
}

std::shared_ptr<MediaTrack> MediaTrack::Clone()
{
	auto track = std::make_shared<MediaTrack>();

	// Media Track
	track->_is_valid = _is_valid;
	track->_has_quality_measured = _has_quality_measured;
	track->_id = _id;
	track->_variant_name = _variant_name;
	track->_public_name = _public_name;
	track->_language = _language;
	track->_codec_id = _codec_id;
	track->_codec_library_id = _codec_library_id;
	track->_origin_bitstream_format = _origin_bitstream_format;	
	track->_media_type = _media_type;
	track->_time_base = _time_base;
	track->_bitrate = _bitrate;
	track->_bitrate_conf = _bitrate_conf;
	track->_byass = _byass;
	track->_bypass_conf = _bypass_conf;
	track->_start_frame_time = _start_frame_time;
	track->_last_frame_time = _last_frame_time;
	track->_decoder_configuration_record = _decoder_configuration_record;
	track->_total_frame_count = _total_frame_count;
	track->_total_frame_bytes = _total_frame_bytes;

	// Video Track
	track->_framerate = _framerate;
	track->_framerate_conf = _framerate_conf;
	track->_framerate_estimated = _framerate_estimated;
	track->_video_timescale = _video_timescale;
	track->_width = _width;
	track->_width_conf = _width_conf;
	track->_height = _height;
	track->_height_conf = _height_conf;
	track->_key_frame_interval = _key_frame_interval;
	track->_key_frame_interval_conf = _key_frame_interval_conf;
	track->_b_frames = _b_frames;
	track->_has_bframe = _has_bframe;
	track->_preset = _preset;
	track->_use_hwaccel = _use_hwaccel;
	track->_colorspace = _colorspace;

	// Audio Track
	track->_channel_layout = _channel_layout;
	track->_sample = _sample;
	track->_audio_timescale = _audio_timescale;
	track->_audio_samples_per_frame= _audio_samples_per_frame;
	
	return track;
}