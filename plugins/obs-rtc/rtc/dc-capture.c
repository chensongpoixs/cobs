#include "dc-capture.h"

#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

static inline void init_textures(struct dc_capture *capture)
{
	if (capture->compatibility) {
		capture->texture = gs_texture_create(capture->width,
						     capture->height, GS_BGRA,
						     1, NULL, GS_DYNAMIC);
	} else {
		capture->texture =
			gs_texture_create_gdi(capture->width, capture->height);

		if (capture->texture) {
			capture->extra_texture = gs_texture_create(
				capture->width, capture->height, GS_BGRA, 1,
				NULL, 0);
			if (!capture->extra_texture) {
				blog(LOG_WARNING, "[dc_capture_init] Failed to "
						  "create textures");
				gs_texture_destroy(capture->texture);
				capture->texture = NULL;
			}
		}
	}

	if (!capture->texture) {
		blog(LOG_WARNING, "[dc_capture_init] Failed to "
				  "create textures");
		return;
	}

	capture->valid = true;
}

void dc_capture_init(struct dc_capture *capture, int x, int y, uint32_t width,
		     uint32_t height, bool cursor, bool compatibility)
{
	memset(capture, 0, sizeof(struct dc_capture));

	capture->x = x;
	capture->y = y;
	capture->width = width;
	capture->height = height;
	capture->capture_cursor = cursor;

	obs_enter_graphics();

	if (!gs_gdi_texture_available())
		compatibility = true;

	capture->compatibility = compatibility;

	init_textures(capture);

	obs_leave_graphics();

	if (!capture->valid)
		return;

	if (compatibility) {
		BITMAPINFO bi = {0};
		BITMAPINFOHEADER *bih = &bi.bmiHeader;
		bih->biSize = sizeof(BITMAPINFOHEADER);
		bih->biBitCount = 32;
		bih->biWidth = width;
		bih->biHeight = height;
		bih->biPlanes = 1;

		capture->hdc = CreateCompatibleDC(NULL);
		capture->bmp =
			CreateDIBSection(capture->hdc, &bi, DIB_RGB_COLORS,
					 (void **)&capture->bits, NULL, 0);
		capture->old_bmp = SelectObject(capture->hdc, capture->bmp);
	}
}

void dc_capture_free(struct dc_capture *capture)
{
	if (capture->hdc) {
		SelectObject(capture->hdc, capture->old_bmp);
		DeleteDC(capture->hdc);
		DeleteObject(capture->bmp);
	}

	obs_enter_graphics();
	gs_texture_destroy(capture->extra_texture);
	gs_texture_destroy(capture->texture);
	obs_leave_graphics();

	memset(capture, 0, sizeof(struct dc_capture));
}

static void draw_cursor(struct dc_capture *capture, HDC hdc, HWND window)
{
	HICON icon;
	ICONINFO ii;
	CURSORINFO *ci = &capture->ci;
	POINT win_pos = {capture->x, capture->y};

	if (!(capture->ci.flags & CURSOR_SHOWING))
		return;

	icon = CopyIcon(capture->ci.hCursor);
	if (!icon)
		return;

	if (GetIconInfo(icon, &ii)) {
		POINT pos;

		if (window)
			ClientToScreen(window, &win_pos);

		pos.x = ci->ptScreenPos.x - (int)ii.xHotspot - win_pos.x;
		pos.y = ci->ptScreenPos.y - (int)ii.yHotspot - win_pos.y;

		DrawIconEx(hdc, pos.x, pos.y, icon, 0, 0, 0, NULL, DI_NORMAL);

		DeleteObject(ii.hbmColor);
		DeleteObject(ii.hbmMask);
	}

	DestroyIcon(icon);
}

static inline HDC dc_capture_get_dc(struct dc_capture *capture)
{
	if (!capture->valid)
		return NULL;

	if (capture->compatibility)
		return capture->hdc;
	else
		return gs_texture_get_dc(capture->texture);
}

static inline void dc_capture_release_dc(struct dc_capture *capture)
{
	if (capture->compatibility) {
		gs_texture_set_image(capture->texture, capture->bits,
				     capture->width * 4, false);
	} else {
		gs_texture_release_dc(capture->texture);
	}
}
//static FILE *out_file_ptr = NULL;

void dc_capture_capture(unsigned char *rgba_ptr, BITMAPINFO bmi, int32_t width,
			int32_t height,
			struct dc_capture *capture,
			HWND window)
{
	 
	HDC hdc_target;
	HDC hdc;

	 
	hdc = dc_capture_get_dc(capture);
	if (!hdc) {
		blog(LOG_WARNING, "[capture_screen] Failed to get "
				  "texture DC");
		return;
	}

	
	StretchDIBits(hdc, 0, 0, width, height, 0, 0, width, height,
		      rgba_ptr,
		      (BITMAPINFO *)&bmi.bmiHeader, DIB_RGB_COLORS, SRCCOPY);

	 
	 
	dc_capture_release_dc(capture);

	capture->texture_written = true;
}

static void draw_texture(struct dc_capture *capture, bool texcoords_centered)
{
	gs_effect_t *effect = obs_get_base_effect(OBS_EFFECT_BILINEAR_LOWRES);
	gs_technique_t *tech = gs_effect_get_technique(effect, "Draw");
	gs_eparam_t *image = gs_effect_get_param_by_name(effect, "image");

	gs_texture_t *texture = capture->texture;
	const bool compatibility = capture->compatibility;
	bool linear_sample = compatibility;
	if (!linear_sample && !texcoords_centered) {
		gs_texture_t *const extra_texture = capture->extra_texture;
		gs_copy_texture(extra_texture, texture);
		texture = extra_texture;
		linear_sample = true;
	}

	const bool previous = gs_framebuffer_srgb_enabled();
	gs_enable_framebuffer_srgb(linear_sample);
	gs_enable_blending(false);

	if (linear_sample)
		gs_effect_set_texture_srgb(image, texture);
	else
		gs_effect_set_texture(image, texture);

	const uint32_t flip = compatibility ? GS_FLIP_V : 0;
	const size_t passes = gs_technique_begin(tech);
	for (size_t i = 0; i < passes; i++) {
		if (gs_technique_begin_pass(tech, i)) {
			gs_draw_sprite(texture, flip, 0, 0);

			gs_technique_end_pass(tech);
		}
	}
	gs_technique_end(tech);

	gs_enable_blending(true);
	gs_enable_framebuffer_srgb(previous);
}

void dc_capture_render(struct dc_capture *capture, bool texcoords_centered)
{
	if (capture->valid && capture->texture_written)
		draw_texture(capture, texcoords_centered);
}
