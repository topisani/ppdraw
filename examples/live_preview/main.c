#include <ppdraw/draw.h>
#include <ppdraw/registry.h>
#include <ppdraw/fonts.h>

static void example_draw(draw_t ctx) {
  ctx.font = &font_micro;
  draw_vec_t str1start = DRAW_VEC(4, 4);
  draw_vec_t str1end = draw_str(&ctx, str1start, "Hello", -1);
  ctx.font = &font_10x20;
  draw_str(&ctx, DRAW_VEC(str1start.x - 1, str1end.y + 4), "World!", -1);
  draw_line(&ctx, DRAW_VEC(str1start.x, 27), DRAW_VEC(58, 27));
}

PPDRAW_REGISTER_WIDGET(example_widget) = {
    .width = 64,
    .height = 32,
    .draw_cb = example_draw,
};
