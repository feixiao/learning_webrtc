#pragma once
#include "modules/video_capture/video_capture.h"
#include "modules/video_capture/video_capture_factory.h"
#include "modules/audio_device/include/audio_device.h"
#include "modules/audio_device/include/audio_device_defines.h"
#include "SDL.h"
#include <cstdint>

class VideoCaptureCallback :public rtc::VideoSinkInterface<webrtc::VideoFrame>, public webrtc::AudioTransport
{
public:

	VideoCaptureCallback() = default;
	~VideoCaptureCallback() = default;

	int Init(uint32_t width, uint32_t height);

	void OnFrame(const webrtc::VideoFrame& videoFrame) override;
	void OnDiscardedFrame() override;

	int32_t RecordedDataIsAvailable(const void* audioSamples,
		const size_t nSamples,
		const size_t nBytesPerSample,
		const size_t nChannels,
		const uint32_t samplesPerSec,
		const uint32_t totalDelayMS,
		const int32_t clockDrift,
		const uint32_t currentMicLevel,
		const bool keyPressed,
		uint32_t& newMicLevel) override;  // NOLINT

									 // Implementation has to setup safe values for all specified out parameters.
	int32_t NeedMorePlayData(const size_t nSamples,
		const size_t nBytesPerSample,
		const size_t nChannels,
		const uint32_t samplesPerSec,
		void* audioSamples,
		size_t& nSamplesOut,  // NOLINT
		int64_t* elapsed_time_ms,
		int64_t* ntp_time_ms) override;  // NOLINT

	// Method to pull mixed render audio data from all active VoE channels.
	// The data will not be passed as reference for audio processing internally.
	void PullRenderData(int bits_per_sample,
		int sample_rate,
		size_t number_of_channels,
		size_t number_of_frames,
		void* audio_data,
		int64_t* elapsed_time_ms,
		int64_t* ntp_time_ms) override ;
private:
	uint32_t _width;
	uint32_t _height;

	std::shared_ptr<SDL_Window> _pWindow;
	std::shared_ptr<SDL_Renderer> _pRender;
	std::shared_ptr<SDL_Texture > _pTexture;
};

