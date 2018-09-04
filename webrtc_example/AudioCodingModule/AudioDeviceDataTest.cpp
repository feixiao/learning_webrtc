#include "stdafx.h"
#include "AudioDeviceDataTest.h"
#include "rtc_base/logging.h"



AudioDeviceDataTest::~AudioDeviceDataTest() {
	
}

int32_t AudioDeviceDataTest::Init() {
	_pACM.reset(webrtc::AudioCodingModule::Create());
	if (_pACM.get() == nullptr) {
		return -1;
	}

	int index = _pACM->Codec("opus", 48000, 2);
	if (index < 0) {
		RTC_LOG(LS_ERROR) << "opus 48000 2 ";
		return -1;
	}

	RTC_LOG(INFO) << "index of codecs : " << index;
	webrtc::CodecInst c;
	int ret = _pACM->Codec(index, &c);
	if (0 != ret) {
		return ret;
	}
	ret = _pACM->RegisterSendCodec(c);
	if (0 != ret) {
		return ret;
	}

	_pACM->RegisterTransportCallback(this);


	// 创建编码器工厂对象
	_pAEF = webrtc::CreateBuiltinAudioEncoderFactory();

	auto acs = _pAEF->GetSupportedEncoders();
	for (auto a :acs){
		RTC_LOG(INFO) << "SupportedEncoders : " << a;
	}
	webrtc::SdpAudioFormat format("opus", 48000, 2);
	_pEncoder = _pAEF->MakeAudioEncoder(1,format);
	RTC_DCHECK(_pEncoder.get());

	_pWavReader.reset(new webrtc::WavReader("./music_orig.wav"));
	RTC_DCHECK(_pWavReader.get());
	
	RTC_LOG(INFO) << " num_channels : " << _pWavReader->num_channels() << " sample_rate " << _pWavReader->sample_rate() 
		<< " num_samples_ : " << _pWavReader->num_samples();
	return ret;
}


void AudioDeviceDataTest::OnCaptureData(
		const void * audio_samples, 
		const size_t num_samples,
		const size_t bytes_per_sample, 
		const size_t num_channels, 
		const uint32_t samples_per_sec){
	// RTC_LOG(INFO) << __FUNCTION__;

	if (nullptr == _pWavWriter.get()) {
		RTC_LOG(INFO) << __FUNCTION__ << " samples_per_sec : " << samples_per_sec << " num_channels : " << num_channels;
		RTC_LOG(INFO) << __FUNCTION__ << " bytes_per_sample : " << bytes_per_sample << " num_samples : " << num_samples;
		_pWavWriter.reset(new webrtc::WavWriter("./test.wav", samples_per_sec, num_channels));
	}
	_pWavWriter->WriteSamples(static_cast<const int16_t*>(audio_samples), num_samples * num_channels);


	rtc::ArrayView<const int16_t> audio(static_cast<const int16_t*>(audio_samples), num_samples * num_channels);
	rtc::Buffer  encoded;
	_pEncoder->Encode(_timestamp, audio, &encoded);
	RTC_LOG(INFO) << __FUNCTION__ << " encoded : " << encoded.size();
	_timestamp += 10;

//	rtc_WavWriteSamples(_pWavWriter, static_cast<const float*>(audio_samples), num_samples);
}

void AudioDeviceDataTest::OnRenderData(
		const void * audio_samples, 
		const size_t num_samples, 
		const size_t bytes_per_sample, 
		const size_t num_channels, 
		const uint32_t samples_per_sec) {
	RTC_LOG(INFO) << __FUNCTION__;

	size_t pcmSize = _pWavReader->ReadSamples(num_samples * num_channels, (int16_t*)audio_samples);

}

int32_t AudioDeviceDataTest::SendData(
		FrameType frame_type, 
		uint8_t payload_type, 
		uint32_t timestamp, 
		const uint8_t * payload_data, 
		size_t payload_len_bytes, 
		const RTPFragmentationHeader * fragmentation) {

	RTC_LOG(INFO) << __FUNCTION__;
	return int32_t();
}


