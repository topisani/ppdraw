#include <ppdraw/draw.h>
#include <ppdraw/preview.h>
#include <ppdraw/fonts.h>
#include <stdio.h>

PPDRAW_REGISTER_PREVIEW(preview_example, DRAW_VEC(64, 32));
static void preview_example(draw_t ctx) {
  ctx.font = &font_micro;
  draw_vec_t str1start = DRAW_VEC(4, 4);
  draw_vec_t str1end = draw_str(&ctx, str1start, "Hello", -1);
  ctx.font = &font_10x20;
  draw_str(&ctx, DRAW_VEC(str1start.x - 1, str1end.y + 4), "World!", -1);
  draw_line(&ctx, DRAW_VEC(str1start.x, 27), DRAW_VEC(58, 27));
}

PPDRAW_REGISTER_PREVIEW_NAMED(preview_with_params, DRAW_VEC(128, 64), "Preview With Params");
static void preview_with_params(draw_t ctx) {
  int counter = ppdraw_preview_param_int("counter", 0, ctx.size.x, ctx.size.x / 2);
  char buf[32];
  snprintf(buf, sizeof(buf), "%d", counter + 1);
  ctx.font = &font_10x20;
  draw_str_aligned(&ctx, DRAW_VEC(ctx.size.x / 2, ctx.size.y / 2 - 10), DRAW_ALIGN_CENTER, buf, -1);
}
