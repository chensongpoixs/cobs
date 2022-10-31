/***********************************************************************************************
created: 		2019-03-02

author:			chensong

purpose:		log
************************************************************************************************/
#ifndef C_MEDIASOUP_H
#define C_MEDIASOUP_H
#include <thread>
#include <string>
#include <functional>
namespace chen
{
	typedef std::function<void(uint32_t status, uint32_t error_info)>     rtc_status_update_cb;



	class  crtc_mgr
	{
	public:
		crtc_mgr();
		~crtc_mgr();

	public:
		static bool global_init();
		static void global_destroy();

	public:
		bool init( uint32_t gpu_index);

		void startup(const char * rtcIp = "127.0.0.1", uint16_t rtcPort = 9500, const  char * roomName = "chensong", const char* clientName = "chensong"
			, uint32_t reconnectWaittime = 5);

		void destroy();
		/// <summary>
		/// 
		/// </summary>
		/// <param name="rgba_ptr"></param>
		/// <param name="width"></param>
		/// <param name="height"></param>
		void   rtc_video(unsigned char * rgba_ptr /*DXGI_FORMAT_B8G8R8A8_UNORM*/, uint32_t fmt,  int width, int height);
		/// <summary>
		/// 
		/// </summary>
		/// <param name="y_ptr"></param>
		/// <param name="uv_ptr"></param>
		/// <param name="width"></param>
		/// <param name="height"></param>
		void   rtc_video(unsigned char * y_ptr, unsigned char * uv_ptr, uint32_t fmt, int width, int height);
		void rtc_texture(void * texture, uint32_t fmt, int width, int height);
		void rtc_pause();
		void rtc_resume();
		bool rtc_video_staus() const { return m_rtc_pause; }
		bool  rtc_run();
		const std::string& get_rtc_roomname() const;
		const std::string &get_rtc_username() const;

		void set_rtc_status_callback(rtc_status_update_cb callback);
	private:
		void _rtc_thread();
	private:
		crtc_mgr(const crtc_mgr &);
		crtc_mgr &operator=(const crtc_mgr &);
	private:
		bool				m_init ;
		std::thread			m_thread;
		bool				m_rtc_pause;
		std::string			m_rtc_ip;
		uint16_t			m_rtc_port;
		std::string			m_room_name;
		std::string			m_client_name;
		uint32_t			m_reconnect_wait;
		void*				m_client_ptr;
	};
}
#endif // C_MEDIASOUP_H
