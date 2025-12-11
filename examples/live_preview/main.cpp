#include <ppdraw/draw.h>
#include <ppdraw/registry.h>
#include <ppdraw/draw.hpp>
#include <ppdraw/fonts.h>

static void example_draw(draw_t cctx) {
  ppdraw::Ctx ctx(cctx);

  ctx.font = &font_10x20;
  auto world_size = ctx.str({32, 10}, "World", ppdraw::Align::Center);
  ctx.font = &font_micro;
  ctx.str({world_size.x,5}, "Hello", ppdraw::Align::Right);
}

PPDRAW_REGISTER_WIDGET(example_widget) = {
    .width = 64,
    .height = 32,
    .draw_cb = example_draw,
};
