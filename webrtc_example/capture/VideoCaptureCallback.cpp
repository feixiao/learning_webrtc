#include "stdafx.h"
#include "VideoCaptureCallback.h"
#include "rtc_base/logging.h"

void PrintVideoFrameInfo(const webrtc::VideoFrame& videoFrame) {
	RTC_LOG(INFO) << "VideoFrame size : " << videoFrame.size()
		<< " timestamp_us : " << videoFrame.timestamp_us();
}


int VideoCaptureCallback::Init(uint32_t width, uint32_t height) {
	_width = width;
	_height = height;

	int ret = SDL_Init(SDL_INIT_VIDEO);
	if (ret < 0) {
		RTC_LOG(LS_ERROR)<<"Couldn't initialize SDL:" << SDL_GetError();
		return -1;
	}

	_pWindow.reset(SDL_CreateWindow("YUY2 Test",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		_width, _height,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE));

	_pRender.reset( SDL_CreateRenderer(_pWindow.get(), -1, 0) );
	if (_pRender.get() == nullptr) {
		RTC_LOG(LS_ERROR) << "SDL_CreateRenderer Error:" << SDL_GetError();
		return -1;
	}

	_pTexture.reset(SDL_CreateTexture(
		_pRender.get(),
//		SDL_PIXELFORMAT_YUY2,
		SDL_PIXELFORMAT_IYUV,
		SDL_TEXTUREACCESS_STREAMING,
		_width,
		_height)
	);

	if (_pTexture.get() == nullptr) {
		RTC_LOG(LS_ERROR) << "SDL_CreateTexture Error: " << SDL_GetError();
		return -2;
	}
	return 0;
}

void VideoCaptureCallback::OnFrame(const webrtc::VideoFrame& videoFrame) {
//	RTC_LOG(INFO) << __FUNCTION__;
//	PrintVideoFrameInfo(videoFrame);

	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = _width;
	rect.h = _height;

	rtc::scoped_refptr<webrtc::I420BufferInterface> buffer(
		videoFrame.video_frame_buffer()->ToI420());

	SDL_UpdateYUVTexture(_pTexture.get(), &rect,
		buffer->DataY(), buffer->StrideY(),
		buffer->DataU(), buffer->StrideU(),
		buffer->DataV(), buffer->StrideV());

	SDL_RenderClear(_pRender.get());
	SDL_RenderCopy(_pRender.get(), _pTexture.get(), &rect, &rect);
	SDL_RenderPresent(_pRender.get());
}

void VideoCaptureCallback::OnDiscardedFrame() {
	RTC_LOG(INFO) << __FUNCTION__;
};
