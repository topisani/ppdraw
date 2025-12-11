#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct draw_img {
    uint16_t width;
    uint16_t height;
    const uint8_t *data;
} draw_img_t;

typedef struct draw_font {
    uint8_t width;
    uint8_t height;
    uint16_t chars;
    const uint8_t *bitmap;
    const uint16_t *index;
    const uint8_t *widths;
} draw_font_t;

/// Base type for colors. Will be truncated to MSB as needed
typedef uint8_t draw_color_t;

static const draw_color_t DRAW_BLACK = 0x00;
static const draw_color_t DRAW_WHITE = 0xFF;

typedef enum draw_paint_type {
	DRAW_PAINT_SOLID,
	DRAW_PAINT_INVERT,
} draw_paint_type_t;

typedef struct draw_paint {
	draw_paint_type_t type;
	draw_color_t color;
} draw_paint_t;

#define DRAW_PAINT(type, color) ((draw_paint_t){type, color})
#define DRAW_SOLID(color)       ((draw_paint_t){DRAW_PAINT_SOLID, color})
#define DRAW_INVERT()           ((draw_paint_t){DRAW_PAINT_INVERT, 0})

/// The base type for a dimension
typedef int16_t draw_dim_t;

typedef struct draw_vec {
	draw_dim_t x;
	draw_dim_t y;
} draw_vec_t;

#define DRAW_VEC(x, y) ((draw_vec_t){x, y})

typedef struct draw_fb {
	draw_dim_t width;
	draw_dim_t height;
	uint8_t *buf;
	uint16_t buf_size;
	uint8_t bpp; // Bits per pixel
} draw_fb_t;

struct draw {
	draw_fb_t *fb;
	bool invert;
	const draw_font_t *font;
	draw_vec_t text_spacing;
	draw_paint_t paint;
};

typedef struct draw draw_t;

void draw_init(draw_t *draw, draw_fb_t *fb);
void draw_reset(draw_t *draw);
draw_t draw_clone(draw_t *draw);

void draw_clear(draw_t *draw);
void draw_pixel(draw_t *draw, draw_vec_t pos);
void draw_line(draw_t *draw, draw_vec_t p0, draw_vec_t p1);
void draw_rect_fill(draw_t *draw, draw_vec_t p0, draw_vec_t p1);

void draw_rounded_frame(draw_t *draw, draw_vec_t p0, draw_vec_t p1, bool fill);

void draw_img(draw_t *draw, const struct draw_img *img, draw_vec_t p);

enum draw_align {
	DRAW_ALIGN_LEFT,
	DRAW_ALIGN_RIGHT,
	DRAW_ALIGN_CENTER,
};

#define DRAW_STRLEN SIZE_MAX

draw_vec_t draw_glyph(draw_t *draw, uint32_t glyph, draw_vec_t p);
draw_vec_t draw_get_str_size(draw_t *draw, const char* str, int nbytes);
draw_vec_t draw_str(draw_t *draw, draw_vec_t p, const char* str, int nbytes);
draw_vec_t draw_str_aligned(draw_t *draw, draw_vec_t p, enum draw_align, const char *str, int nbytes);

#ifdef __cplusplus
}
#endif
