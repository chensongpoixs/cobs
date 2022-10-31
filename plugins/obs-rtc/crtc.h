


/***********************************************************************************************
created: 		2022-05-07

author:			chensong

purpose:		log
************************************************************************************************/
#ifndef C_CLOUD_RENDERING_C_H
#define C_CLOUD_RENDERING_C_H

#include <stdint.h>
#ifdef _MSC_VER
#include <Windows.h>
#define DLLIMPORT  
#elif defined(__GNUC__) || defined(__APPLE__)
#define DLLIMPORT
#else
// 其他不支持的编译器需要自己实现这个方法
#error unexpected c complier (msc/gcc), Need to implement this method for demangle
#endif

typedef void (*send_video_callback)(void* data, unsigned char * rgba_ptr, int32_t width, int32_t height);
typedef void (*rtc_status_callback)(uint32_t  status, uint32_t  error );
//typedef void (*set_gpu_index_callback)(uint32_t);
#ifdef __cplusplus

void DLLIMPORT cpp_capture_init(void *data );
void DLLIMPORT cpp_capture_startup( );
void DLLIMPORT cpp_capture_destroy();
 

void cpp_set_video_callback(send_video_callback callback);
//////////////////////////RTC//////////////////////////////////////////////
bool DLLIMPORT cpp_rtc_global_init();
bool DLLIMPORT cpp_rtc_global_destroy();

bool DLLIMPORT cpp_rtc_init( );

void DLLIMPORT cpp_rtc_startup( );

void DLLIMPORT cpp_rtc_destroy( );

void DLLIMPORT cpp_set_rtc_status_callback(rtc_status_callback callback);

void DLLIMPORT cpp_rtc_video(unsigned char *rgba_ptr, uint32_t fmt, int width, int height);
 


const char *DLLIMPORT cpp_get_rtc_roomname();
const char *DLLIMPORT cpp_get_rtc_username(); 
void DLLIMPORT cpp_get_rtc_roomusername(char * buffer);
extern "C" {

#endif

//void DLLIMPORT c_set_send_video_callback(send_video_callback callback);
//void DLLIMPORT c_set_set_gpu_index_callback(set_gpu_index_callback callback);

void DLLIMPORT c_capture_init(void * data);
void DLLIMPORT c_capture_startup();
void DLLIMPORT c_capture_destroy();
void c_set_video_callback(send_video_callback callback);


//////////////////////////////C ===> RTC //////////////////////////////
bool DLLIMPORT c_rtc_global_init();
bool DLLIMPORT c_rtc_global_destroy();

bool DLLIMPORT c_rtc_init();

void DLLIMPORT c_rtc_startup();

void DLLIMPORT c_rtc_destroy();

void DLLIMPORT c_set_rtc_status_callback(rtc_status_callback callback);

void DLLIMPORT c_rtc_video(unsigned char *rgba_ptr, uint32_t fmt, int width,
			     int height);
const char *DLLIMPORT c_get_rtc_roomname();
const char *DLLIMPORT c_get_rtc_username(); 


void DLLIMPORT c_get_rtc_roomusername(char *buffer);
#ifdef __cplusplus
}

#endif

#endif // C_CLOUD_RENDERING_C_H
