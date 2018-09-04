#pragma once

#include "modules/audio_device/include/audio_device_data_observer.h"
#include "rtc_base/constructormagic.h"
#include "modules/audio_coding/include/audio_coding_module.h"
#include "modules/include/module_common_types.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "common_audio/wav_file.h"

using webrtc::FrameType;
using webrtc::RTPFragmentationHeader;

class AudioDeviceDataTest : public webrtc::AudioDeviceDataObserver, public webrtc::AudioPacketizationCallback
{
public:
	AudioDeviceDataTest() = default;
	virtual ~AudioDeviceDataTest();

	int32_t Init();

	// 通过 AudioDeviceDataObserver 继承
	virtual void OnCaptureData(const void * audio_samples, const size_t num_samples, const size_t bytes_per_sample, const size_t num_channels, const uint32_t samples_per_sec) override;
	virtual void OnRenderData(const void * audio_samples, const size_t num_samples, const size_t bytes_per_sample, const size_t num_channels, const uint32_t samples_per_sec) override;

	// 通过 AudioPacketizationCallback 继承
	virtual int32_t SendData(FrameType frame_type, uint8_t payload_type, uint32_t timestamp, const uint8_t * payload_data, size_t payload_len_bytes, const RTPFragmentationHeader * fragmentation) override;
private:

	std::unique_ptr<webrtc::AudioCodingModule> _pACM;

	rtc::scoped_refptr<webrtc::AudioEncoderFactory> _pAEF;
	std::unique_ptr<webrtc::AudioEncoder> _pEncoder;

	uint64_t _timestamp = 0;

	std::unique_ptr<webrtc::WavWriter> _pWavWriter = nullptr;
	std::unique_ptr<webrtc::WavReader> _pWavReader = nullptr;

	RTC_DISALLOW_COPY_AND_ASSIGN(AudioDeviceDataTest);

};

