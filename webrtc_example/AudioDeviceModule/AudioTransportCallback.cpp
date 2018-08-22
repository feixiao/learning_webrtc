#include "stdafx.h"
#include "AudioTransportCallback.h"
#include "rtc_base/logging.h"
#include "api/audio/audio_mixer.h"
#include "modules/audio_processing/include/audio_processing.h"
#include "rtc_base/checks.h"
#include "audio/utility/audio_frame_operations.h"
#include "audio/remix_resample.h"
#include <algorithm> 


size_t GenerateZeroBuffer(void* audio_buffer, size_t audio_buffer_size) {
	memset(audio_buffer, 0, audio_buffer_size);
	return audio_buffer_size;
}
size_t CopyFromRecBuffer(void* audio_buffer, size_t audio_buffer_size,
	void* rec_buffer_, size_t rec_buffer_bytes_) {

	const size_t min_buffer_size = std::min(audio_buffer_size, 
		rec_buffer_bytes_);
	memcpy(audio_buffer, rec_buffer_, min_buffer_size);
	return min_buffer_size;
}


AudioTransportCallback::AudioTransportCallback(webrtc::AudioMixer* mixer,
	webrtc::AudioProcessing* audio_processing) 
	: _pMixer(mixer), _pAudioProcessing(audio_processing){

	_pFile = fopen("./audio.bin","wb+");
}

AudioTransportCallback::~AudioTransportCallback() {
	fclose(_pFile);
}


int32_t AudioTransportCallback::RecordedDataIsAvailable(const void* audioSamples,
	const size_t nSamples,
	const size_t nBytesPerSample,
	const size_t nChannels,
	const uint32_t samplesPerSec,
	const uint32_t totalDelayMS,
	const int32_t clockDrift,
	const uint32_t currentMicLevel,
	const bool keyPressed,
	uint32_t& newMicLevel) {



	//RTC_LOG(INFO) << __FUNCTION__;
	//RTC_LOG(INFO) << " nSamples : " << nSamples 
	//	<< " nBytesPerSample: " << nBytesPerSample
	//	<< " nChannels : " << nChannels
	//	<< " samplesPerSec : " << samplesPerSec
	//	<< " totalDelayMS : " << totalDelayMS;

	RTC_DCHECK(audioSamples);
	RTC_DCHECK_GE(nChannels, 1);
	RTC_DCHECK_LE(nChannels, 2);
	RTC_DCHECK_EQ(2 * nChannels, nBytesPerSample);
	RTC_DCHECK_GE(samplesPerSec, webrtc::AudioProcessing::NativeRate::kSampleRate8kHz);

	// 100 = 1 second / data duration (10 ms).
	RTC_DCHECK_EQ(nSamples * 100, samplesPerSec);
	RTC_DCHECK_LE(nBytesPerSample * nSamples * nChannels,
		webrtc::AudioFrame::kMaxDataSizeBytes);

	rtc::CritScope cs(&_crit);
	_rec_buffer_bytes = nBytesPerSample * nSamples * nChannels;
	memcpy(_rec_buffer, audioSamples, _rec_buffer_bytes);
	//fwrite(audioSamples,1, _rec_buffer_bytes, _pFile);
	newMicLevel = currentMicLevel;
	return 0;
}

int32_t AudioTransportCallback::NeedMorePlayData(const size_t nSamples,
	const size_t nBytesPerSample,
	const size_t nChannels,
	const uint32_t samplesPerSec,
	void* audioSamples,
	size_t& nSamplesOut,
	int64_t* elapsed_time_ms,
	int64_t* ntp_time_ms) {
	
	RTC_DCHECK_EQ(sizeof(int16_t) * nChannels, nBytesPerSample);
	RTC_DCHECK_GE(nChannels, 1);
	RTC_DCHECK_LE(nChannels, 2);
	RTC_DCHECK_GE(
		samplesPerSec,
		static_cast<uint32_t>(webrtc::AudioProcessing::NativeRate::kSampleRate8kHz));

	//RTC_LOG(INFO) << __FUNCTION__;
	//RTC_LOG(INFO) << " nSamples : " << nSamples
	//	<< " nBytesPerSample: " << nBytesPerSample
	//	<< " nChannels : " << nChannels
	//	<< " samplesPerSec : " << samplesPerSec;

	rtc::CritScope cs(&_crit);

	if (_rec_buffer_bytes == 0) {
		return 0;
	}
	const size_t audio_buffer_size = nBytesPerSample * nSamples * nChannels;

	const size_t  bytes_out = audio_buffer_size;
	// 单声道 转换为 双声道
	size_t index = 0;
	for(int i = 0; i < _rec_buffer_bytes; i+=4) {
		index = i * 2;
		memcpy(static_cast<uint8_t*>(audioSamples) + index, _rec_buffer+i, 4);
		memcpy(static_cast<uint8_t*>(audioSamples) + index + 4, _rec_buffer+i, 4);
	}

	//const size_t bytes_out = _rec_buffer_bytes != 0 ?
	//	CopyFromRecBuffer(audioSamples, audio_buffer_size, _rec_buffer, _rec_buffer_bytes) :
	//	GenerateZeroBuffer(audioSamples, audio_buffer_size);
	nSamplesOut = bytes_out / nBytesPerSample;
	*elapsed_time_ms = 0;
	*ntp_time_ms = 0;
	return 0;
}


void AudioTransportCallback::PullRenderData(int bits_per_sample,
	int sample_rate,
	size_t number_of_channels,
	size_t number_of_frames,
	void* audio_data,
	int64_t* elapsed_time_ms,
	int64_t* ntp_time_ms) {

	RTC_LOG(INFO) << __FUNCTION__;

}