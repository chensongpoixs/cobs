#include <windows.h>
#include <obs-module.h>
#include <util/windows/win-version.h>
#include <util/platform.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("rtc", "en-US")
MODULE_EXPORT const char *obs_module_description(void)
{
	return "rtc";
}

 
extern struct obs_source_info window_capture_info;
 

extern bool cached_versions_match(void);
extern bool load_cached_graphics_offsets(bool is32bit, const char *config_path);
extern bool load_graphics_offsets(bool is32bit, bool use_hook_address_cache,
				  const char *config_path);

/* temporary, will eventually be erased once we figure out how to create both
 * 32bit and 64bit versions of the helpers/hook */
#ifdef _WIN64
#define IS32BIT false
#else
#define IS32BIT true
#endif

static const bool use_hook_address_cache = false;

 



//bool graphics_uses_d3d11 = false;
bool wgc_supported = false;

bool obs_module_load(void)
{
	struct win_version_info ver;
	bool win8_or_above = false;
	char *config_dir;

	struct win_version_info win1903 = {
		.major = 10, .minor = 0, .build = 18362, .revis = 0};

	config_dir = obs_module_config_path(NULL);
	if (config_dir) {
		os_mkdirs(config_dir);
		bfree(config_dir);
	}

	get_win_ver(&ver);

	win8_or_above = ver.major > 6 || (ver.major == 6 && ver.minor >= 2);

	/*obs_enter_graphics();
	graphics_uses_d3d11 = gs_get_device_type() == GS_DEVICE_DIRECT3D_11;
	obs_leave_graphics();

	if (graphics_uses_d3d11)
	{
		wgc_supported = win_version_compare(&ver, &win1903) >= 0;
	} */
	obs_register_source(&window_capture_info);
	 
	return true;
}

void obs_module_unload(void)
{
	//wait_for_hook_initialization();
}
