


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
//typedef void (*set_gpu_index_callback)(uint32_t);
#ifdef __cplusplus

//int cpp_func(int input);
void DLLIMPORT cpp_capture_init(void *data );
void DLLIMPORT cpp_capture_startup( );
void DLLIMPORT cpp_capture_destroy();
 

void cpp_set_video_callback(send_video_callback callback);
 
extern "C" {

#endif

//void DLLIMPORT c_set_send_video_callback(send_video_callback callback);
//void DLLIMPORT c_set_set_gpu_index_callback(set_gpu_index_callback callback);

void DLLIMPORT c_capture_init(void * data);
void DLLIMPORT c_capture_startup();
void DLLIMPORT c_capture_destroy();
void c_set_video_callback(send_video_callback callback);
 
#ifdef __cplusplus
}

#endif

#endif // C_CLOUD_RENDERING_C_H
