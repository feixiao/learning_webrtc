#include "stdafx.h"
#include "AudioTransportCallback.h"
#include "rtc_base/logging.h"


#include <algorithm> 



size_t GenerateZeroBuffer(void* audio_buffer, size_t audio_buffer_size) {
	memset(audio_buffer, 0, audio_buffer_size);
	return audio_buffer_size;
}
size_t CopyFromRecBuffer(void* audio_buffer, size_t audio_buffer_size,
		void* rec_buffer_, size_t rec_buffer_bytes_ ) {

	const size_t min_buffer_size = min(audio_buffer_size, rec_buffer_bytes_);
	memcpy(audio_buffer, rec_buffer_, min_buffer_size);
	return min_buffer_size;
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

	rtc::CritScope cs(&_crit);
	_rec_buffer_bytes = nSamples * nBytesPerSample;
	if ((_rec_buffer_bytes == 0) ||
			(_rec_buffer_bytes > kNumberSamples * kNumberBytesPerSample)) {
		RTC_LOG(LS_ERROR) << _rec_buffer_bytes << " < " << kNumberSamples * kNumberBytesPerSample;
		return -1;
	}
	memcpy(_rec_buffer, audioSamples, _rec_buffer_bytes);
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
	
	//RTC_LOG(INFO) << __FUNCTION__;
	//RTC_LOG(INFO) << " nSamples : " << nSamples
	//	<< " nBytesPerSample: " << nBytesPerSample
	//	<< " nChannels : " << nChannels
	//	<< " samplesPerSec : " << samplesPerSec;

	rtc::CritScope cs(&_crit);
	const size_t audio_buffer_size = nSamples * nBytesPerSample;
	const size_t bytes_out = _rec_buffer_bytes != 0 ?
		CopyFromRecBuffer(audioSamples, audio_buffer_size, _rec_buffer, _rec_buffer_bytes) :
		GenerateZeroBuffer(audioSamples, audio_buffer_size);
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