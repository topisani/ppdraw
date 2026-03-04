#pragma once

#ifdef __cplusplus
#include <etl/string_view.h>

#include "draw.h"

namespace ppdraw {
struct Vec : draw_vec_t {
  Vec(int x, int y) : draw_vec_t{(draw_dim_t)x, (draw_dim_t)y} {}
  Vec(draw_vec_t p) : draw_vec_t(p) {}
};
}; // namespace ppdraw

inline ppdraw::Vec operator+(draw_vec_t a, draw_vec_t b) {
  return draw_vec_add(a, b);
}
inline ppdraw::Vec operator-(draw_vec_t a, draw_vec_t b) {
  return draw_vec_sub(a, b);
}
inline ppdraw::Vec operator*(draw_vec_t a, draw_vec_t b) {
  return {static_cast<draw_dim_t>(a.x * b.x),
          static_cast<draw_dim_t>(a.y * b.y)};
}
inline ppdraw::Vec operator/(draw_vec_t a, draw_vec_t b) {
  return {static_cast<draw_dim_t>(a.x / b.x),
          static_cast<draw_dim_t>(a.y / b.y)};
}
inline ppdraw::Vec operator-(draw_vec_t a) {
  return {static_cast<draw_dim_t>(-a.x), static_cast<draw_dim_t>(-a.y)};
}
inline ppdraw::Vec operator*(draw_vec_t a, draw_dim_t s) {
  return {static_cast<draw_dim_t>(a.x * s), static_cast<draw_dim_t>(a.y * s)};
}
inline ppdraw::Vec operator*(draw_dim_t s, draw_vec_t a) { return a * s; }
inline ppdraw::Vec operator/(draw_vec_t a, draw_dim_t s) {
  return {static_cast<draw_dim_t>(a.x / s), static_cast<draw_dim_t>(a.y / s)};
}

inline draw_vec_t &operator+=(draw_vec_t &a, draw_vec_t b) { return a = a + b; }
inline draw_vec_t &operator-=(draw_vec_t &a, draw_vec_t b) { return a = a - b; }
inline draw_vec_t &operator*=(draw_vec_t &a, draw_vec_t b) { return a = a * b; }
inline draw_vec_t &operator/=(draw_vec_t &a, draw_vec_t b) { return a = a / b; }
inline draw_vec_t &operator*=(draw_vec_t &a, draw_dim_t s) { return a = a * s; }
inline draw_vec_t &operator/=(draw_vec_t &a, draw_dim_t s) { return a = a / s; }

inline bool operator==(draw_vec_t a, draw_vec_t b) {
  return a.x == b.x && a.y == b.y;
}
inline bool operator!=(draw_vec_t a, draw_vec_t b) { return !(a == b); }

inline ppdraw::Vec &operator+=(ppdraw::Vec &a, draw_vec_t b) { return a = a + b; }
inline ppdraw::Vec &operator-=(ppdraw::Vec &a, draw_vec_t b) { return a = a - b; }
inline ppdraw::Vec &operator*=(ppdraw::Vec &a, draw_vec_t b) { return a = a * b; }
inline ppdraw::Vec &operator/=(ppdraw::Vec &a, draw_vec_t b) { return a = a / b; }
inline ppdraw::Vec &operator*=(ppdraw::Vec &a, draw_dim_t s) { return a = a * s; }
inline ppdraw::Vec &operator/=(ppdraw::Vec &a, draw_dim_t s) { return a = a / s; }

namespace ppdraw {

enum struct Align {
  Left = DRAW_ALIGN_LEFT,
  Right = DRAW_ALIGN_RIGHT,
  Center = DRAW_ALIGN_CENTER,
};

struct Ctx : draw_t {
  Ctx(draw_t ctx) : draw_t(ctx) {};
  Ctx(draw_fb_t *fb) { draw_init(this, fb); }

  void reset() { draw_reset(this); }
  void clear() { draw_clear(this); }
  Ctx clone() const { return draw_clone(this); }

  void clip(Vec position, Vec size) { draw_clip(this, position, size); }
  void translate(Vec position) { draw_translate(this, position); }
  Ctx clipped(Vec p, Vec size) const { return draw_clipped(this, p, size); }

  void pixel(Vec pos) const { draw_pixel(this, pos); }
  void line(Vec p0, Vec p1) const { draw_line(this, p0, p1); }
  void rect_fill(Vec p0, Vec p1) const { draw_rect_fill(this, p0, p1); }
  void rect_stroke(Vec p0, Vec p1) const { draw_rect_stroke(this, p0, p1); }
  void rounded_frame(Vec p0, Vec p1, bool fill) const {
    draw_rounded_frame(this, p0, p1, fill);
  }

  void img(const struct draw_img *img, Vec p) const { draw_img(this, img, p); }

  Vec glyph(uint32_t glyph, Vec p) const { return draw_glyph(this, glyph, p); }

  Vec str(Vec p, etl::string_view s, Align align = Align::Left) const {
    return draw_str_aligned(this, p, static_cast<enum draw_align>(align),
                            s.data(), s.size());
  }

  Vec get_str_size(etl::string_view str) const {
    return draw_get_str_size(this, str.data(), str.size());
  }

  int16_t width() const { return this->fb->width; }
  int16_t height() const { return this->fb->height; }
};
} // namespace ppdraw

#endif
