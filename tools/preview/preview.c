#include <ppdraw/draw.h>
#include <ppdraw/registry.h>

#include <memory.h>
#include <stdint.h>
#include <stdlib.h>

draw_t *ppdraw_ctx_alloc(int width, int height, int bpp) {
  size_t buf_size = (width * height * bpp + 7) / 8;
  uint8_t *buf = malloc(buf_size);
  memset(buf, 0, buf_size);
  draw_fb_t *fb = malloc(sizeof(draw_fb_t));
  fb->width = width;
  fb->height = height;
  fb->buf = buf;
  fb->buf_size = buf_size;
  fb->bpp = bpp;
  draw_t *ctx = malloc(sizeof(draw_t));
  draw_init(ctx, fb);
  return ctx;
}

void ppdraw_ctx_free(draw_t *ctx) {
  free(ctx->fb->buf);
  free(ctx->fb);
  free(ctx);
}

uint8_t *ppdraw_ctx_render_widget(draw_t *ctx, struct ppdraw_widget *widget) {
  draw_reset(ctx);
  widget->draw_cb(*ctx);
  return ctx->fb->buf;
}

__attribute__((used)) static void *reexport_symbols[] = {
    &ppdraw_widget_registry_count,
    &ppdraw_widget_registry_get,
};
