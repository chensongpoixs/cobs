/***********************************************************************************************
created: 		2022-01-20

author:			chensong

purpose:		assertion macros
************************************************************************************************/

#include "modules/desktop_capture/desktop_capture_options.h"
#include "rtc_base/logging.h"
#include "rtc_base/time_utils.h"
#include "third_party/libyuv/include/libyuv.h"
#include "cmediasoup_mgr.h"
//#include "cclient.h"
#include "capture.h"


#include "../crtc.h"
namespace chen {

DesktopCapture::DesktopCapture() : dc_(nullptr), start_flag_(false) {}

DesktopCapture::~DesktopCapture()
{
	Destory();
}

void DesktopCapture::Destory()
{
	StopCapture();

	if (!dc_)
		return;

	dc_.reset(nullptr);
}

DesktopCapture *DesktopCapture::Create(size_t target_fps,
				       size_t capture_screen_index)
{
	std::unique_ptr<DesktopCapture> dc(new DesktopCapture());
	if (!dc->Init(target_fps, capture_screen_index)) {
		RTC_LOG(LS_WARNING) << "Failed to create DesktopCapture(fps = " << target_fps << ")";
		return nullptr;
	}
	return dc.release();
}

bool DesktopCapture::Init(size_t target_fps, size_t capture_screen_index)
{
	// 窗口
	/*dc_ = webrtc::DesktopCapturer::CreateWindowCapturer(
            webrtc::DesktopCaptureOptions::CreateDefault());*/
	//桌面
	webrtc::DesktopCaptureOptions result;
	result.set_allow_directx_capturer(true);
	dc_ = webrtc::DesktopCapturer::CreateScreenCapturer(result);

	if (!dc_)
		return false;

	webrtc::DesktopCapturer::SourceList sources;
	dc_->GetSourceList(&sources);
	if (capture_screen_index > sources.size()) {
		RTC_LOG(LS_WARNING)
			<< "The total sources of screen is " << sources.size()
			<< ", but require source of index at "
			<< capture_screen_index;
		return false;
	}

	RTC_CHECK(dc_->SelectSource(sources[capture_screen_index].id));
	window_title_ = sources[capture_screen_index].title;
	fps_ = target_fps;

	RTC_LOG(LS_INFO)
		<< "Init DesktopCapture finish window_title = " << window_title_
		<< " , fps = " << fps_ << "";
	// Start new thread to capture
	return true;
}

void DesktopCapture::OnCaptureResult(webrtc::DesktopCapturer::Result result,
				     std::unique_ptr<webrtc::DesktopFrame> frame)
{
	//RTC_LOG(LS_INFO) << "new Frame";

	static auto timestamp =
		std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch())
			.count();
	static size_t cnt = 0;

	cnt++;
	auto timestamp_curr =
		std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch())
			.count();
	if (timestamp_curr - timestamp > 1000) {
		//RTC_LOG(LS_INFO) << "FPS: " << cnt;
		cnt = 0;
		timestamp = timestamp_curr;
	}

	// Convert DesktopFrame to VideoFrame
	if (result != webrtc::DesktopCapturer::Result::SUCCESS) {
		//  RTC_LOG(LS_ERROR) << "Capture frame faiiled, result: " << result;

		return;
	}
	if (m_callback)
	{
		m_callback(m_data, frame->data(), frame->size().width(),
			   frame->size().height());
	}
	/*static FILE *out_file_ptr = ::fopen("./chensong.yuv", "wb+");
	if (out_file_ptr)
	{
		::fwrite(frame->data(),
			 frame->size().width() * frame->size().height() * 3,1,
			 out_file_ptr);
		::fflush(out_file_ptr);
	}*/
	//int width = frame->size().width();
	//int height = frame->size().height();
	//// int half_width = (width + 1) / 2;

	//if (!i420_buffer_.get() ||
	//    i420_buffer_->width() * i420_buffer_->height() < width * height) {
	//	i420_buffer_ = webrtc::I420Buffer::Create(width, height);
	//}
	//memcpy(i420_buffer_->MutableDataY(), frame->data(), width * height * 4);
	/*  libyuv::ConvertToI420(frame->data(), 0, i420_buffer_->MutableDataY(),
            i420_buffer_->StrideY(), i420_buffer_->MutableDataU(),
            i420_buffer_->StrideU(), i420_buffer_->MutableDataV(),
            i420_buffer_->StrideV(), 0, 0, width, height, width,
            height, libyuv::kRotate0, libyuv::FOURCC_ARGB);*/

	// seting 马流的信息

	/*webrtc::VideoFrame captureFrame =
		webrtc::VideoFrame::Builder()
			.set_video_frame_buffer(i420_buffer_)
			.set_timestamp_rtp(0)
			.set_timestamp_ms(rtc::TimeMillis())
			.set_rotation(webrtc::kVideoRotation_0)
			.build();*/
	// captureFrame.set_ntp_time_ms(0);
	//s_client.webrtc_video(captureFrame);
	// DesktopCaptureSource::OnFrame(captureFrame);
	// rtc media info
	/* DesktopCaptureSource::OnFrame(
            webrtc::VideoFrame(i420_buffer_, 0, 0, webrtc::kVideoRotation_0));*/
}

void DesktopCapture::StartCapture()
{
	if (start_flag_) {
		RTC_LOG(WARNING) << "Capture already been running...";
		return;
	}

	start_flag_ = true;

	// Start new thread to capture
	capture_thread_.reset(new std::thread([this]() {
		dc_->Start(this);

		while (start_flag_) {
			dc_->CaptureFrame();
			std::this_thread::sleep_for(
				std::chrono::milliseconds(1000 / 60));
		}
	}));
}

void DesktopCapture::StopCapture()
{
	start_flag_ = false;

	if (capture_thread_ && capture_thread_->joinable()) {
		capture_thread_->join();
	}
}

}



//////////////////////////////////////////////////////

std::unique_ptr<chen::DesktopCapture> desktop_capture_ptr = NULL;
static chen::crtc_mgr g_rtc_mgr;
void   cpp_capture_init(void *data)
{
	desktop_capture_ptr.reset(chen::DesktopCapture::Create(30, 0));
	if (desktop_capture_ptr) {
		desktop_capture_ptr->SetData(data);
	}
}
void   cpp_capture_startup()
{
	if (desktop_capture_ptr) {

		desktop_capture_ptr->StartCapture();
	}
}
void   cpp_capture_destroy()
{
	desktop_capture_ptr->StopCapture();
}
void cpp_set_video_callback(send_video_callback callback)
{
	desktop_capture_ptr->SetCallback(callback);
}
void c_set_video_callback(send_video_callback callback)
{
	cpp_set_video_callback(callback);
}
void   c_capture_init(void *data)
{
	cpp_capture_init(data);
}
void   c_capture_startup()
{
	cpp_capture_startup();
}
void   c_capture_destroy()
{
	cpp_capture_destroy();
}

bool   cpp_rtc_global_init()
{
	return g_rtc_mgr.global_init();
}
bool   cpp_rtc_global_destroy()
{
	 g_rtc_mgr.global_destroy();
	return true;
}
bool   cpp_rtc_init()
{
	return g_rtc_mgr.init(0);
}

void   cpp_rtc_startup()
{
	g_rtc_mgr.startup();
}

void   cpp_rtc_destroy()
{
	g_rtc_mgr.destroy();
}
void   cpp_set_rtc_status_callback(rtc_status_callback callback)
{
	g_rtc_mgr.set_rtc_status_callback(callback);
}

void   cpp_rtc_video(unsigned char *rgba_ptr, uint32_t fmt, int width,
			     int height)
{
	g_rtc_mgr.rtc_video(rgba_ptr, fmt, width, height);
}

const char *  cpp_get_rtc_roomname()
{
	return g_rtc_mgr.get_rtc_roomname().c_str();
}
const char *  cpp_get_rtc_username()
{
	return g_rtc_mgr.get_rtc_username().c_str();
}
void   cpp_get_rtc_roomusername(char *buffer)
{
	sprintf(buffer, "%s|%s", g_rtc_mgr.get_rtc_roomname().c_str(),
		g_rtc_mgr.get_rtc_username().c_str());
}
bool     c_rtc_global_init()
{
	return cpp_rtc_global_init();
}
bool   c_rtc_global_destroy()
{
	return cpp_rtc_global_destroy();
}
bool   c_rtc_init()
{
	return cpp_rtc_init();
}

void   c_rtc_startup()
{
	cpp_rtc_startup();
}

void   c_rtc_destroy()
{
	cpp_rtc_destroy();
}
void c_set_rtc_status_callback(rtc_status_callback callback)
{
	cpp_set_rtc_status_callback(callback);
}

void c_rtc_video(unsigned char *rgba_ptr, uint32_t fmt, int width,
			   int height)
{
	cpp_rtc_video(rgba_ptr, fmt, width, height);
}


const char *c_get_rtc_roomname()
{
	return cpp_get_rtc_roomname();
}
const char *c_get_rtc_username()
{
	return cpp_get_rtc_username();
}
void c_get_rtc_roomusername(char* buffer)
{
	cpp_get_rtc_roomusername(buffer);
}
