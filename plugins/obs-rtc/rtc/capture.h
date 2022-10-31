/***********************************************************************************************
created: 		2022-10-28

author:			chensong

purpose:		cglobal_rtc


************************************************************************************************/

#ifndef _C_RTC_CAPTURE_H_
#define _C_RTC_CAPTURE_H_
#include "api/video/video_frame.h"
#include "api/video/video_sink_interface.h"
//#include "desktop_capture_source.h"
#include "modules/desktop_capture/desktop_capturer.h"
#include "modules/desktop_capture/desktop_frame.h"
#include "api/video/i420_buffer.h"
#include "../crtc.h"
#include <thread>
#include <atomic>

namespace chen {
class cclient;
class DesktopCapture : public webrtc::DesktopCapturer::Callback,
		       public rtc::VideoSinkInterface<webrtc::VideoFrame> {
public:
	static DesktopCapture *Create(size_t target_fps,
				      size_t capture_screen_index);

	~DesktopCapture() override;

	std::string GetWindowTitle() const { return window_title_; }

	void StartCapture();
	void StopCapture();


	void SetData(void *data) { m_data = data; }
	void SetClientCallback(cclient *client) { m_client_ptr = client; }
	void SetCallback(send_video_callback callback)
	{
		m_callback = callback;
	} //send_video_callback callback

private:
	DesktopCapture();

	void Destory();

	void OnFrame(const webrtc::VideoFrame &frame) override {}

	bool Init(size_t target_fps, size_t capture_screen_index);

	void OnCaptureResult(webrtc::DesktopCapturer::Result result,
			std::unique_ptr<webrtc::DesktopFrame> frame) override;

	std::unique_ptr<webrtc::DesktopCapturer> dc_;

	size_t fps_;
	std::string window_title_;

	std::unique_ptr<std::thread> capture_thread_;
	std::atomic_bool start_flag_;

	rtc::scoped_refptr<webrtc::I420Buffer> i420_buffer_;
	void *m_data {NULL};
	send_video_callback m_callback{NULL};
	cclient *m_client_ptr{NULL};
};

}
#endif // _C_RTC_CAPTURE_H_
