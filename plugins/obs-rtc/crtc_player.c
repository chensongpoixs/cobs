
 
#include <stdlib.h>
#include <util/dstr.h>
#include <util/threading.h>
#include "dc-capture.h"
#include "window-helpers.h"
#include "../../libobs/util/platform.h"
#include "../../libobs-winrt/winrt-capture.h"
#include "crtc.h"
/* clang-format off */
 
#define TEXT_WINDOW_CAPTURE obs_module_text("Rtc.Name")
#define TEXT_WINDOW         obs_module_text("Rtc.RoomName")
#define TEXT_METHOD         obs_module_text("Rtc.WebSocket")
#define TEXT_METHOD_AUTO    obs_module_text("Rtc.WebSocket.AutoConnnect")
#define TEXT_METHOD_BITBLT  obs_module_text("WindowCapture.Method.BitBlt")
#define TEXT_METHOD_WGC     obs_module_text("WindowCapture.Method.WindowsGraphicsCapture")
#define TEXT_MATCH_PRIORITY obs_module_text("WindowCapture.Priority")
#define TEXT_MATCH_TITLE    obs_module_text("WindowCapture.Priority.Title")
#define TEXT_MATCH_CLASS    obs_module_text("WindowCapture.Priority.Class")
#define TEXT_MATCH_EXE      obs_module_text("WindowCapture.Priority.Exe")
#define TEXT_CAPTURE_CURSOR obs_module_text("CaptureCursor")
#define TEXT_COMPATIBILITY  obs_module_text("Compatibility")
#define TEXT_CLIENT_AREA    obs_module_text("ClientArea")

/* clang-format on */

#define WC_CHECK_TIMER 1.0f
#define RESIZE_CHECK_TIME 0.2f
#define CURSOR_CHECK_TIME 0.2f

typedef BOOL (*PFN_winrt_capture_supported)();
typedef BOOL (*PFN_winrt_capture_cursor_toggle_supported)();
typedef struct winrt_capture *(*PFN_winrt_capture_init_window)(
	BOOL cursor, HWND window, BOOL client_area);
typedef void (*PFN_winrt_capture_free)(struct winrt_capture *capture);

typedef BOOL (*PFN_winrt_capture_active)(const struct winrt_capture *capture);
typedef BOOL (*PFN_winrt_capture_show_cursor)(struct winrt_capture *capture,
					      BOOL visible);
typedef void (*PFN_winrt_capture_render)(struct winrt_capture *capture);
typedef uint32_t (*PFN_winrt_capture_width)(const struct winrt_capture *capture);
typedef uint32_t (*PFN_winrt_capture_height)(
	const struct winrt_capture *capture);

struct winrt_exports {
	PFN_winrt_capture_supported winrt_capture_supported;
	PFN_winrt_capture_cursor_toggle_supported
		winrt_capture_cursor_toggle_supported;
	PFN_winrt_capture_init_window winrt_capture_init_window;
	PFN_winrt_capture_free winrt_capture_free;
	PFN_winrt_capture_active winrt_capture_active;
	PFN_winrt_capture_show_cursor winrt_capture_show_cursor;
	PFN_winrt_capture_render winrt_capture_render;
	PFN_winrt_capture_width winrt_capture_width;
	PFN_winrt_capture_height winrt_capture_height;
};

enum window_capture_method {
	METHOD_AUTO,
	METHOD_BITBLT,
	METHOD_WGC,
};

typedef DPI_AWARENESS_CONTEXT(WINAPI *PFN_SetThreadDpiAwarenessContext)(
	DPI_AWARENESS_CONTEXT);
typedef DPI_AWARENESS_CONTEXT(WINAPI *PFN_GetThreadDpiAwarenessContext)(VOID);
typedef DPI_AWARENESS_CONTEXT(WINAPI *PFN_GetWindowDpiAwarenessContext)(HWND);

struct rtc_player_capture {
	obs_source_t *source;

	pthread_mutex_t update_mutex;

	char *title;
	char *class;
	char *executable;
	enum window_capture_method method;
	enum window_priority priority;
	bool cursor;
	bool compatibility;
	bool client_area;
	bool use_wildcards; /* TODO */

	struct dc_capture capture;

	bool previously_failed;
	void *winrt_module;
	struct winrt_exports exports;
	struct winrt_capture *capture_winrt;

	float resize_timer;
	float check_window_timer;
	float cursor_check_time;

	HWND window;
	RECT last_rect;

	PFN_SetThreadDpiAwarenessContext set_thread_dpi_awareness_context;
	PFN_GetThreadDpiAwarenessContext get_thread_dpi_awareness_context;
	PFN_GetWindowDpiAwarenessContext get_window_dpi_awareness_context;


	//////
	



};


struct rtc_capture {
	BITMAPINFO bmi;
	unsigned char *rgba_ptr;
	unsigned char *cur_rgba_ptr;
	int32_t width;
	int32_t height;
};


static struct rtc_capture rtc_data = {0};


//static FILE *out_rtc_file_ptr = NULL;
static  void capture_callback(void *data, unsigned char *rgba_ptr,
		      int32_t width, int32_t height)
{
	struct window_capture *wc = data;
	if (!wc)
	{
		
		return;
	}
	if (!rtc_data.rgba_ptr)
	{
		size_t size = width * height * 4;
		rtc_data.rgba_ptr = malloc(sizeof(unsigned char) * size);
		rtc_data.cur_rgba_ptr = malloc(sizeof(unsigned char) * size);
		if (!rtc_data.rgba_ptr || !rtc_data.cur_rgba_ptr)
		{
			return;
		}
		rtc_data.width = width;
		rtc_data.height = height;
		ZeroMemory(&rtc_data.bmi, sizeof(rtc_data.bmi));
		rtc_data.bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		rtc_data.bmi.bmiHeader.biPlanes = 1;
		rtc_data.bmi.bmiHeader.biBitCount = 32;
		rtc_data.bmi.bmiHeader.biCompression = BI_RGB;
		rtc_data.bmi.bmiHeader.biWidth = width;
		rtc_data.bmi.bmiHeader.biHeight = -height;
		rtc_data.bmi.bmiHeader.biSizeImage =
			 width * height * (rtc_data.bmi.bmiHeader.biBitCount >> 3);
	}
	/*if (!out_rtc_file_ptr) {
		out_rtc_file_ptr = fopen("./rtc_chensong.yuv", "wb+");
	}
	*/
	memcpy(rtc_data.rgba_ptr, rgba_ptr, width * height * 4);
	unsigned char *temp = rtc_data.cur_rgba_ptr;
	rtc_data.cur_rgba_ptr = rtc_data.rgba_ptr;
	rtc_data.rgba_ptr = temp;
	/*if (out_rtc_file_ptr) {
		fwrite(rtc_data.cur_rgba_ptr, width * height * 3, 1,
		       out_rtc_file_ptr);
		fflush(out_rtc_file_ptr);
	}*/
}



static const char *wgc_partial_match_classes[] = {
	"Chrome",
	"Mozilla",
	NULL,
};

static const char *wgc_whole_match_classes[] = {
	"ApplicationFrameWindow",
	"Windows.UI.Core.CoreWindow",
	"XLMAIN",        /* excel*/
	"PPTFrameClass", /* powerpoint */
	"OpusApp",       /* word */
	NULL,
};

static enum window_capture_method
choose_method(enum window_capture_method method, bool wgc_supported,
	      const char *current_class)
{
	if (!wgc_supported)
		return METHOD_BITBLT;

	if (method != METHOD_AUTO)
		return method;

	if (!current_class)
		return METHOD_BITBLT;

	const char **class = wgc_partial_match_classes;
	while (*class) {
		if (astrstri(current_class, *class) != NULL) {
			return METHOD_WGC;
		}
		class ++;
	}

	class = wgc_whole_match_classes;
	while (*class) {
		if (astrcmpi(current_class, *class) == 0) {
			return METHOD_WGC;
		}
		class ++;
	}

	return METHOD_BITBLT;
}

static const char *get_method_name(int method)
{
	const char *method_name = "";
	switch (method) {
	case METHOD_AUTO:
		method_name = "Automatic";
		break;
	case METHOD_BITBLT:
		method_name = "BitBlt";
		break;
	case METHOD_WGC:
		method_name = "WGC";
		break;
	}

	return method_name;
}

static void log_settings(struct rtc_player_capture *wc, obs_data_t *s)
{
	int method = (int)obs_data_get_int(s, "method");

	if (wc->title != NULL) {
		blog(LOG_INFO,
		     "[window-capture: '%s'] update settings:\n"
		     "\texecutable: %s\n"
		     "\tmethod selected: %s\n"
		     "\tmethod chosen: %s\n",
		     obs_source_get_name(wc->source), wc->executable,
		     get_method_name(method), get_method_name(wc->method));
		blog(LOG_DEBUG, "\tclass:      %s", wc->class);
	}
}

extern bool wgc_supported;

static void update_settings(struct rtc_player_capture *wc, obs_data_t *s)
{
	pthread_mutex_lock(&wc->update_mutex);

	int method = (int)obs_data_get_int(s, "method");
	const char *window = obs_data_get_string(s, "window");
	int priority = (int)obs_data_get_int(s, "priority");

	bfree(wc->title);
	bfree(wc->class);
	bfree(wc->executable);

	build_window_strings(window, &wc->class, &wc->title, &wc->executable);

	wc->method = choose_method(method, wgc_supported, wc->class);
	wc->priority = (enum window_priority)priority;
	wc->cursor = obs_data_get_bool(s, "cursor");
	wc->use_wildcards = obs_data_get_bool(s, "use_wildcards");
	wc->compatibility = obs_data_get_bool(s, "compatibility");
	wc->client_area = obs_data_get_bool(s, "client_area");

	pthread_mutex_unlock(&wc->update_mutex);
}

/* ------------------------------------------------------------------------- */

static const char *rtc_player_getname(void *unused)
{
	UNUSED_PARAMETER(unused);
	return TEXT_WINDOW_CAPTURE;
}

#define WINRT_IMPORT(func)                                           \
	do {                                                         \
		exports->func = (PFN_##func)os_dlsym(module, #func); \
		if (!exports->func) {                                \
			success = false;                             \
			blog(LOG_ERROR,                              \
			     "Could not load function '%s' from "    \
			     "module '%s'",                          \
			     #func, module_name);                    \
		}                                                    \
	} while (false)

static bool load_winrt_imports(struct winrt_exports *exports, void *module,
			       const char *module_name)
{
	bool success = true;

	WINRT_IMPORT(winrt_capture_supported);
	WINRT_IMPORT(winrt_capture_cursor_toggle_supported);
	WINRT_IMPORT(winrt_capture_init_window);
	WINRT_IMPORT(winrt_capture_free);
	WINRT_IMPORT(winrt_capture_active);
	WINRT_IMPORT(winrt_capture_show_cursor);
	WINRT_IMPORT(winrt_capture_render);
	WINRT_IMPORT(winrt_capture_width);
	WINRT_IMPORT(winrt_capture_height);

	return success;
}

//extern bool graphics_uses_d3d11;

static void *rtc_player_create(obs_data_t *settings, obs_source_t *source)
{
	struct rtc_player_capture *wc =
		bzalloc(sizeof(struct rtc_player_capture));
	//memset(wc, 0, sizeof(struct window_capture));
	wc->source = source;

	

	pthread_mutex_init(&wc->update_mutex, NULL);
	 
	/*c_capture_init(wc);
	c_set_video_callback(&capture_callback);
	c_capture_startup();*/
	 
	 
	const HMODULE hModuleUser32 = GetModuleHandle(L"User32.dll");
	if (hModuleUser32) {
		//让 DPI 察觉到当前线程
		PFN_SetThreadDpiAwarenessContext
			set_thread_dpi_awareness_context =
				(PFN_SetThreadDpiAwarenessContext)GetProcAddress(
					hModuleUser32,
					"SetThreadDpiAwarenessContext");
		PFN_GetThreadDpiAwarenessContext
			get_thread_dpi_awareness_context =
				(PFN_GetThreadDpiAwarenessContext)GetProcAddress(
					hModuleUser32,
					"GetThreadDpiAwarenessContext");
		// 获取某个窗口的DPI模式
		PFN_GetWindowDpiAwarenessContext
			get_window_dpi_awareness_context =
				(PFN_GetWindowDpiAwarenessContext)GetProcAddress(
					hModuleUser32,
					"GetWindowDpiAwarenessContext");
		if (set_thread_dpi_awareness_context &&
		    get_thread_dpi_awareness_context &&
		    get_window_dpi_awareness_context) {
			wc->set_thread_dpi_awareness_context =
				set_thread_dpi_awareness_context;
			wc->get_thread_dpi_awareness_context =
				get_thread_dpi_awareness_context;
			wc->get_window_dpi_awareness_context =
				get_window_dpi_awareness_context;
		}
	}

	update_settings(wc, settings);
	log_settings(wc, settings);
	return wc;
}

static void rtc_player_actual_destroy(void *data)
{
	struct rtc_player_capture *wc = data;

	if (wc->capture_winrt) {
		wc->exports.winrt_capture_free(wc->capture_winrt);
	}

	obs_enter_graphics();
	dc_capture_free(&wc->capture);
	obs_leave_graphics();

	bfree(wc->title);
	bfree(wc->class);
	bfree(wc->executable);

	if (wc->winrt_module)
		os_dlclose(wc->winrt_module);

	pthread_mutex_destroy(&wc->update_mutex);

	bfree(wc);
}

static void rtc_player_destroy(void *data)
{
	//c_capture_destroy();
	obs_queue_task(OBS_TASK_GRAPHICS, rtc_player_actual_destroy, data,
		       false);
}

static void force_reset(struct rtc_player_capture *wc)
{
	wc->window = NULL;
	wc->resize_timer = RESIZE_CHECK_TIME;
	wc->check_window_timer = WC_CHECK_TIMER;
	wc->cursor_check_time = CURSOR_CHECK_TIME;

	wc->previously_failed = false;
}

static void rtc_player_update(void *data, obs_data_t *settings)
{
	struct rtc_player_capture *wc = data;
	update_settings(wc, settings);
	log_settings(wc, settings);

	force_reset(wc);
}

static uint32_t rtc_player_width(void *data)
{
	struct rtc_player_capture *wc = data;
	return rtc_data.width;
	/*return (wc->method == METHOD_WGC)
		       ? wc->exports.winrt_capture_width(wc->capture_winrt)
		       : wc->capture.width;*/
}

static uint32_t rtc_player_height(void *data)
{
	struct rtc_player_capture *wc = data;
	return rtc_data.height;
	/*return (wc->method == METHOD_WGC)
		       ? wc->exports.winrt_capture_height(wc->capture_winrt)
		       : wc->capture.height;*/
}

static void rtc_player_defaults(obs_data_t *defaults)
{
	obs_data_set_default_int(defaults, "method", METHOD_AUTO);
	obs_data_set_default_bool(defaults, "cursor", true);
	obs_data_set_default_bool(defaults, "compatibility", false);
	obs_data_set_default_bool(defaults, "client_area", true);
}

static void update_settings_visibility(obs_properties_t *props,
				       struct rtc_player_capture *wc)
{
	pthread_mutex_lock(&wc->update_mutex);

	const enum window_capture_method method = wc->method;
	const bool bitblt_options = method == METHOD_BITBLT;
	const bool wgc_options = method == METHOD_WGC;

	const bool wgc_cursor_toggle =
		wgc_options &&
		wc->exports.winrt_capture_cursor_toggle_supported();

	obs_property_t *p = obs_properties_get(props, "cursor");
	obs_property_set_visible(p, bitblt_options || wgc_cursor_toggle);

	p = obs_properties_get(props, "compatibility");
	obs_property_set_visible(p, bitblt_options);

	p = obs_properties_get(props, "client_area");
	obs_property_set_visible(p, wgc_options);

	pthread_mutex_unlock(&wc->update_mutex);
}

static bool rtc_player_capture_method_changed(obs_properties_t *props,
				      obs_property_t *p, obs_data_t *settings)
{
	UNUSED_PARAMETER(p);

	struct rtc_player_capture *wc = obs_properties_get_param(props);
	if (!wc)
		return false;

	update_settings(wc, settings);

	update_settings_visibility(props, wc);

	return true;
}

static void insert_preserved_val(obs_property_t *p, const char *val, size_t idx)
{
	char *class = NULL;
	char *title = NULL;
	char *executable = NULL;
	struct dstr desc = {0};

	build_window_strings(val, &class, &title, &executable);

	dstr_printf(&desc, "[%s]: %s", executable, title);
	obs_property_list_insert_string(p, idx, desc.array, val);
	obs_property_list_item_disable(p, idx, true);

	dstr_free(&desc);
	bfree(class);
	bfree(title);
	bfree(executable);
}

//bool check_window_property_setting(obs_properties_t *ppts, obs_property_t *p,
//				   obs_data_t *settings, const char *val,
//				   size_t idx)
//{
//	const char *cur_val;
//	bool match = false;
//	size_t i = 0;
//
//	cur_val = obs_data_get_string(settings, val);
//	if (!cur_val) {
//		return false;
//	}
//
//	for (;;) {
//		const char *val = obs_property_list_item_string(p, i++);
//		if (!val)
//			break;
//
//		if (strcmp(val, cur_val) == 0) {
//			match = true;
//			break;
//		}
//	}
//
//	if (cur_val && *cur_val && !match) {
//		insert_preserved_val(p, cur_val, idx);
//		return true;
//	}
//
//	UNUSED_PARAMETER(ppts);
//	return false;
//}
static bool rtc_player_window_changed(obs_properties_t *props,
				      obs_property_t *p,
			      obs_data_t *settings)
{
	struct rtc_player_capture *wc = obs_properties_get_param(props);
	if (!wc)
		return false;

	update_settings(wc, settings);

	update_settings_visibility(props, wc);

	check_window_property_setting(props, p, settings, "window", 0);
	return true;
}

static obs_properties_t *rtc_player_properties(void *data)
{
	struct rtc_player_capture *wc = data;

	obs_properties_t *ppts = obs_properties_create();
	obs_properties_set_param(ppts, wc, NULL);

	obs_property_t *p;

	p = obs_properties_add_list(ppts, "window", TEXT_WINDOW,
				    OBS_COMBO_TYPE_LIST,
				    OBS_COMBO_FORMAT_STRING);
	fill_window_list(p, EXCLUDE_MINIMIZED, NULL);
	obs_property_set_modified_callback(p, rtc_player_window_changed);

	p = obs_properties_add_list(ppts, "method", TEXT_METHOD,
				    OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(p, TEXT_METHOD_AUTO, METHOD_AUTO);
	obs_property_list_add_int(p, TEXT_METHOD_BITBLT, METHOD_BITBLT);
	obs_property_list_add_int(p, TEXT_METHOD_WGC, METHOD_WGC);
	obs_property_list_item_disable(p, 2, !wgc_supported);
	obs_property_set_modified_callback(p,
					   rtc_player_capture_method_changed);

	p = obs_properties_add_list(ppts, "priority", TEXT_MATCH_PRIORITY,
				    OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(p, TEXT_MATCH_TITLE, WINDOW_PRIORITY_TITLE);
	obs_property_list_add_int(p, TEXT_MATCH_CLASS, WINDOW_PRIORITY_CLASS);
	obs_property_list_add_int(p, TEXT_MATCH_EXE, WINDOW_PRIORITY_EXE);

	obs_properties_add_bool(ppts, "cursor", TEXT_CAPTURE_CURSOR);

	obs_properties_add_bool(ppts, "compatibility", TEXT_COMPATIBILITY);

	obs_properties_add_bool(ppts, "client_area", TEXT_CLIENT_AREA);

	return ppts;
}

static void rtc_player_hide(void *data)
{
	struct rtc_player_capture *wc = data;

	if (wc->capture_winrt) {
		wc->exports.winrt_capture_free(wc->capture_winrt);
		wc->capture_winrt = NULL;
	}

	memset(&wc->last_rect, 0, sizeof(wc->last_rect));
}

static void rtc_player_tick(void *data, float seconds)
{
	struct rtc_player_capture *wc = data;
	RECT rect;
	bool reset_capture = false;

	if (!obs_source_showing(wc->source))
		return;

	 

	wc->cursor_check_time += seconds;
	 
	obs_enter_graphics();
	{
		/*DPI_AWARENESS_CONTEXT previous = NULL;
		if (wc->get_window_dpi_awareness_context != NULL) {
			const DPI_AWARENESS_CONTEXT context =
				wc->get_window_dpi_awareness_context(
					wc->window);
			previous =
				wc->set_thread_dpi_awareness_context(context);
		}

		GetClientRect(wc->window, &rect);

		if (!reset_capture) {
			wc->resize_timer += seconds;

			if (wc->resize_timer >= RESIZE_CHECK_TIME) {
				if ((rect.bottom - rect.top) !=
					    (wc->last_rect.bottom -
					     wc->last_rect.top) ||
				    (rect.right - rect.left) !=
					    (wc->last_rect.right -
					     wc->last_rect.left))
					reset_capture = true;

				wc->resize_timer = 0.0f;
			}
		}*/

		/*if (reset_capture) {
			wc->resize_timer = 0.0f;
			wc->last_rect = rect;
			dc_capture_free(&wc->capture);
			dc_capture_init(&wc->capture, 0, 0,
					rect.right - rect.left,
					rect.bottom - rect.top, wc->cursor,
					wc->compatibility);
		}*/
		if (rtc_data.rgba_ptr)
		{
			if ( wc->capture.width <= 0 || wc->capture.height <= 0)
			{
				dc_capture_init(&wc->capture, 0, 0,
						rtc_data.width, rtc_data.height,
						0, wc->compatibility);
			}
				
			dc_capture_capture(rtc_data.cur_rgba_ptr, rtc_data.bmi, rtc_data.width, rtc_data.height,
					   &wc->capture,
					   wc->window);

		}
		
		//if (previous)
		//	wc->set_thread_dpi_awareness_context(previous);
	}

	obs_leave_graphics();
}

static void rtc_player_render(void *data, gs_effect_t *effect)
{
	struct rtc_player_capture *wc = data;

	dc_capture_render(&wc->capture,
			  obs_source_get_texcoords_centered(wc->source));

	UNUSED_PARAMETER(effect);
}

struct obs_source_info rtc_player_info = {
	.id = "rtc_player",
	.type = OBS_SOURCE_TYPE_INPUT,
	.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW |
			OBS_SOURCE_SRGB,
	.get_name = rtc_player_getname,
	.create = rtc_player_create,
	.destroy = rtc_player_destroy,
	.update = rtc_player_update,
	.video_render = rtc_player_render,
	.hide = rtc_player_hide,
	.video_tick = rtc_player_tick,
	.get_width = rtc_player_width,
	.get_height = rtc_player_height,
	.get_defaults = rtc_player_defaults,
	.get_properties = rtc_player_properties,
	.icon_type = OBS_ICON_TYPE_WINDOW_CAPTURE,
};
 
