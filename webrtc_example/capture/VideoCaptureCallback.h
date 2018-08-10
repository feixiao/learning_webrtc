#pragma once
#include "modules/video_capture/video_capture.h"
#include "modules/video_capture/video_capture_factory.h"
#include "SDL.h"
#include <cstdint>

class VideoCaptureCallback :public rtc::VideoSinkInterface<webrtc::VideoFrame>
{
public:

	VideoCaptureCallback() = default;
	~VideoCaptureCallback() = default;

	int Init(uint32_t width, uint32_t height);

	void OnFrame(const webrtc::VideoFrame& videoFrame) override;
	void OnDiscardedFrame() override;
private:
	uint32_t _width;
	uint32_t _height;

	std::shared_ptr<SDL_Window> _pWindow;
	std::shared_ptr<SDL_Renderer> _pRender;
	std::shared_ptr<SDL_Texture > _pTexture;
};

