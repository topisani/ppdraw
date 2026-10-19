#include <ppdraw/draw.h>
#include <stdlib.h>
#include <string.h>

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

static void draw_fb_pixel(draw_fb_t *fb, const draw_paint_t *paint, draw_vec_t pos) {
  if (pos.x < 0 || pos.x >= fb->width || pos.y < 0 || pos.y >= fb->height) {
    return;
  }
  uint16_t i = pos.y * fb->width + pos.x;
  if (fb->bpp == 8) {
    // The common case, without the divisions
    switch (paint->type) {
    case DRAW_PAINT_SOLID:
      fb->buf[i] = paint->color;
      break;
    case DRAW_PAINT_INVERT:
      fb->buf[i] ^= 0xff;
      break;
    }
    return;
  }
  uint8_t ppbyte = 8 / fb->bpp;
  uint8_t offs = (ppbyte - 1 - i % ppbyte) * fb->bpp;
  uint8_t mask = ((1 << fb->bpp) - 1) << offs;
  switch (paint->type) {
  case DRAW_PAINT_SOLID: {
    fb->buf[i / ppbyte] &= ~mask;
    fb->buf[i / ppbyte] |= (paint->color >> (8 - fb->bpp)) << offs;
  } break;
  case DRAW_PAINT_INVERT: {
    fb->buf[i / ppbyte] ^= mask;
  } break;
  }
}

void draw_init(draw_t *draw, draw_fb_t *fb) {
  draw->fb = fb;
  draw_reset(draw);
}

void draw_reset(draw_t *draw) {
  draw->font = NULL;
  draw->text_spacing = DRAW_VEC(0, 1);
  draw->paint = DRAW_SOLID(DRAW_WHITE);
  draw->clip_offset = DRAW_VEC(0,0);
  draw->size = DRAW_VEC(draw->fb->width, draw->fb->height);
  draw_clear(draw);
}

draw_t draw_clone(const draw_t *draw) { return *draw; }

void draw_pixel(const draw_t *draw, draw_vec_t pos) {
  if (pos.x >= draw->size.x || pos.y >= draw->size.y) return;
  pos.x += draw->clip_offset.x;
  pos.y += draw->clip_offset.y;
  draw_fb_pixel(draw->fb, &draw->paint, pos);
}

void draw_line(const draw_t *draw, draw_vec_t p0, draw_vec_t p1) {
  draw_dim_t dx = abs(p1.x - p0.x);
  draw_dim_t sx = p0.x < p1.x ? 1 : -1;
  draw_dim_t dy = -abs(p1.y - p0.y);
  draw_dim_t sy = p0.y < p1.y ? 1 : -1;
  draw_dim_t error = dx + dy;

  bool stuck = false;
  while (!stuck) {
    draw_pixel(draw, p0);
    if (p0.x == p1.x && p0.y == p1.y) {
      break;
    }
    draw_dim_t e2 = 2 * error;
    stuck = true;
    if (e2 >= dy) {
      if (p0.x == p1.x) {
        break;
      }
      error = error + dy;
      p0.x = p0.x + sx;
      stuck = false;
    }
    if (e2 <= dx) {
      if (p0.y == p1.y) {
        break;
      }
      error = error + dx;
      p0.y = p0.y + sy;
      stuck = false;
    }
  }
}

/* Rectangle corners are inclusive: p1 is the last pixel drawn, as with lines. */
void draw_rect_fill(const draw_t *draw, draw_vec_t p0, draw_vec_t p1) {
  if (draw->fb->bpp == 8 && draw->paint.type == DRAW_PAINT_SOLID) {
    // The pixels draw_pixel would draw: inside the clip size and the framebuffer, as one run per row
    const draw_fb_t *fb = draw->fb;
    const int x0 = MAX(p0.x, -draw->clip_offset.x), x1 = MIN(MIN(p1.x, draw->size.x - 1), fb->width - 1 - draw->clip_offset.x);
    const int y0 = MAX(p0.y, -draw->clip_offset.y), y1 = MIN(MIN(p1.y, draw->size.y - 1), fb->height - 1 - draw->clip_offset.y);
    for (int y = y0; y <= y1 && x0 <= x1; y++) {
      memset(&fb->buf[(y + draw->clip_offset.y) * fb->width + x0 + draw->clip_offset.x], draw->paint.color, x1 - x0 + 1);
    }
    return;
  }
  for (int x = p0.x; x <= p1.x; x++) {
    for (int y = p0.y; y <= p1.y; y++) {
      draw_pixel(draw, DRAW_VEC(x, y));
    }
  }
}

void draw_rect_stroke(const draw_t *draw, draw_vec_t p0, draw_vec_t p1) {
  for (int x = p0.x; x <= p1.x; x++) {
    draw_pixel(draw, DRAW_VEC(x, p0.y));
    draw_pixel(draw, DRAW_VEC(x, p1.y));
  }
  for (int y = p0.y; y <= p1.y; y++) {
    draw_pixel(draw, DRAW_VEC(p0.x, y));
    draw_pixel(draw, DRAW_VEC(p1.x, y));
  }
}

void draw_clear(const draw_t *draw) { memset(draw->fb->buf, 0, draw->fb->buf_size); }

void draw_img(const draw_t *draw, const struct draw_img *img, draw_vec_t p) {
  for (int y = 0; y < img->height; y++) {
    for (int x = 0; x < img->width; x++) {
      int i = (y * img->width + x);
      if ((img->data[i / 8] & (1 << (i % 8))) ? 1 : 0) {
        draw_pixel(draw, DRAW_VEC(p.x + x, p.y + y));
      }
    }
  }
}

void draw_rounded_frame(const draw_t *draw, draw_vec_t p0, draw_vec_t p1, bool fill) {

  for (int x = p0.x + 1; x < p1.x; x++) {
    draw_pixel(draw, DRAW_VEC(x, p0.y));
    draw_pixel(draw, DRAW_VEC(x, p1.y));
  }
  for (int y = p0.y + 1; y < p1.y; y++) {
    draw_pixel(draw, DRAW_VEC(p0.x, y));
    draw_pixel(draw, DRAW_VEC(p1.x, y));
  }
  if (fill) {
    draw_rect_fill(draw, DRAW_VEC(p0.x + 1, p0.y + 1),
                   DRAW_VEC(p1.x - 1, p1.y - 1));
  }
}

void draw_clip(draw_t *ctx, draw_vec_t position, draw_vec_t size) {
  ctx->clip_offset = draw_vec_add(ctx->clip_offset, position);
  ctx->size = draw_vec_clip_min(size, draw_vec_sub(ctx->size, position));
}

void draw_translate(draw_t *ctx, draw_vec_t position) {
  ctx->clip_offset = draw_vec_add(ctx->clip_offset, position);
  ctx->size = draw_vec_sub(ctx->size, position);
}

static size_t utf8_strlen(const char *str, size_t len) {
  size_t res = 0;
  for (int i = 0; i < len; i++) {
    if (str[i] == 0) {
      break;
    }
    if ((str[i] & 0xC0) == 0x80) {
      continue;
    }
    res++;
  }
  return res;
}

static uint32_t utf8_next_char(const char **str, size_t *nbytes) {
  uint32_t res = 0;
  uint8_t b0, b1, b2, b3;

  if (*nbytes == 0 || **str == 0) {
    return 0;
  }
  b0 = **str;
  (*str)++;
  (*nbytes)--;
  if ((b0 & 0x80) == 0) {
    res = b0;
    return res;
  }
  if (*nbytes == 0 || **str == 0) {
    return 0;
  }
  b1 = **str;
  (*str)++;
  (*nbytes)--;
  if ((b0 & 0xE0) == 0xC0) {
    res = (b0 & ~0xE0) << 6;
    res |= (b1 & ~0xC0);
    return res;
  }
  if (*nbytes == 0 || **str == 0) {
    return 0;
  }
  b2 = **str;
  (*str)++;
  (*nbytes)--;
  if ((b0 & 0xF0) == 0xE0) {
    res |= (b0 & ~0xF0) << 12;
    res |= (b1 & ~0xC0) << 6;
    res |= (b2 & ~0xC0);
    return res;
  }
  if (*nbytes == 0 || **str == 0) {
    return 0;
  }
  b3 = **str;
  (*str)++;
  (*nbytes)--;
  if ((b0 & 0xF7) == 0xF0) {
    res |= (b0 & ~0xF7) << 18;
    res |= (b1 & ~0xC0) << 12;
    res |= (b2 & ~0xC0) << 6;
    res |= (b3 & ~0xC0);
    return res;
  }
  return 0;
}

static int find_glyph(const draw_font_t *font, uint32_t glyph) {
  if (glyph > 0xFFFF) {
    goto notfound;
  }
  // The index is sorted
  int lo = 0, hi = font->chars - 1;
  while (lo <= hi) {
    const int mid = (lo + hi) / 2;
    if (font->index[mid] == glyph) {
      return mid;
    }
    if (font->index[mid] < glyph) {
      lo = mid + 1;
    } else {
      hi = mid - 1;
    }
  }
notfound:
  // U+FFFD is the replacement character '�'
  if (glyph != 0xFFFD) {
    return find_glyph(font, 0xFFFD);
  }
  return -1;
}

#define READ_BIT_U8(ptr, i) (((ptr)[(i) / 8] & (0x80 >> ((i) % 8))) ? 1 : 0)

draw_vec_t draw_glyph(const draw_t *draw, uint32_t glyph, draw_vec_t p) {
  const draw_font_t *font = draw->font;
  if (font == NULL)
    return DRAW_VEC(0, 0);
  const int glyph_idx = find_glyph(font, glyph);
  if (glyph_idx < 0) {
    // Glyph not found, skip it
    return p; // DRAW_VEC(p.x + font->width, p.y + font->height);
  }
  const int pitch = (7 + font->width) / 8;
  const uint8_t *glyph_p = &(font->bitmap[glyph_idx * pitch * font->height]);
  for (int y = 0; y < font->height; y++) {
    for (int x = 0; x < font->width; x++) {
      if (READ_BIT_U8(glyph_p + y * pitch, x)) {
        draw_pixel(draw, DRAW_VEC(p.x + x, p.y + y));
      }
    }
  }
  uint8_t advance_width = font->width;
  if (font->widths != 0) {
    advance_width = font->widths[glyph_idx];
  }
  return DRAW_VEC(p.x + advance_width, p.y + font->height);
}

draw_vec_t draw_get_str_size(const draw_t *draw, const char *str, int length) {
  size_t nbytes = length < 0 ? strlen(str) : length;
  const draw_font_t *font = draw->font;
  if (font == NULL)
    return DRAW_VEC(0, 0);
  draw_dim_t max_width = 0;
  draw_vec_t p = DRAW_VEC(0, 0);
  uint32_t codepoint;
  while (nbytes > 0 && *str != 0) {
    codepoint = utf8_next_char(&str, &nbytes);
    if (codepoint == '\n') {
      p.x = 0;
      p.y += font->height + draw->text_spacing.y;
    } else {
      int glyph_idx = find_glyph(font, codepoint);
      if (glyph_idx >= 0) {
        p.x += (font->widths ? font->widths[glyph_idx] : font->width);
        p.x += draw->text_spacing.x;
        max_width = p.x > max_width ? p.x : max_width;
      }
    }
  }
  return DRAW_VEC(max_width, p.y);
}

draw_vec_t draw_str(const draw_t *draw, draw_vec_t p, const char *str, int length) {
  size_t nbytes = length < 0 ? strlen(str) : length;
  draw_vec_t start = p;
  if (draw->font == NULL)
    return DRAW_VEC(0, 0);
  uint32_t codepoint;
  while (nbytes > 0 && *str != 0) {
    codepoint = utf8_next_char(&str, &nbytes);
    if (codepoint == '\n') {
      p.x = start.x;
      p.y += draw->font->height + draw->text_spacing.y;
    } else {
      p.x = draw_glyph(draw, codepoint, p).x;
      p.x += draw->text_spacing.x;
    }
  }
  return p;
}

draw_vec_t draw_str_aligned(const draw_t *draw, draw_vec_t p, enum draw_align align,
                            const char *str, int length) {
  size_t nbytes = length < 0 ? strlen(str) : length;
  switch (align) {
  case DRAW_ALIGN_LEFT:
    break;
  case DRAW_ALIGN_RIGHT:
    p.x -= draw_get_str_size(draw, str, nbytes).x;
    break;
  case DRAW_ALIGN_CENTER:
    p.x -= (draw_get_str_size(draw, str, nbytes).x) / 2;
    break;
  }
  return draw_str(draw, p, str, nbytes);
}

