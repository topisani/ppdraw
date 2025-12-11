#pragma once

#ifdef __cplusplus
#include <etl/string_view.h>

#include "draw.h"

#include <concepts>
#include <cstring>

namespace ppdraw
{
struct Vec : draw_vec_t {
	Vec(draw_dim_t x, draw_dim_t y) : draw_vec_t{x, y}
	{
	}
	Vec(draw_vec_t p) : draw_vec_t(p)
	{
	}
};

enum struct Align {
	Left = DRAW_ALIGN_LEFT,
	Right = DRAW_ALIGN_RIGHT,
	Center = DRAW_ALIGN_CENTER,
};

struct Ctx : draw_t {

	Ctx(draw_t ctx) : draw_t(ctx) {};

	void clear()
	{
		draw_clear(this);
	}
	Ctx clone()
	{
		return draw_clone(this);
	}

	void pixel(Vec pos)
	{
		draw_pixel(this, pos);
	}
	void line(Vec p0, Vec p1)
	{
		draw_line(this, p0, p1);
	}
	void rect_fill(Vec p0, Vec p1)
	{
		draw_rect_fill(this, p0, p1);
	}

	void glyph(char glyph, Vec p)
	{
		draw_glyph(this, glyph, p);
	}

	Vec str(Vec p, etl::string_view s, Align align = Align::Left)
	{
		return draw_str_aligned(this, p, static_cast<enum draw_align>(align), s.data(),
					s.size());
	}

	Vec get_str_size(etl::string_view str)
	{
		return draw_get_str_size(this, str.data(), str.size());
	}

	void rounded_frame(Vec p0, Vec p1, bool fill)
	{
		draw_rounded_frame(this, p0, p1, fill);
	}

	void img(const struct draw_img *img, Vec p)
	{
		draw_img(this, img, p);
	}

	int16_t width() const
	{
		return this->fb->width;
	}
	int16_t height() const
	{
		return this->fb->height;
	}
};
} // namespace padraic::draw

#endif
