#pragma once

#include <stddef.h>

#include "ppdraw/draw.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ppdraw_preview_widget {
  const char *name;
  draw_vec_t size;
  void (*draw_cb)(draw_t ctx);
}__attribute__((aligned(32)));

/**
 * Register a parameter and get its value
 */
int ppdraw_preview_param_int(const char *name, int min, int max, int def);

#define PPDRAW_REGISTER_PREVIEW_NAMED(function_, size_, name_)                 \
  static void function_(draw_t ctx);                                           \
  static struct ppdraw_preview_widget _ppdraw_preview_##function_              \
      __attribute__((section("ppdraw_preview_registry"), used)) = {     \
          .name = name_,                                                       \
          .size = size_,                                                       \
          .draw_cb = &function_,                                               \
  }
#define PPDRAW_REGISTER_PREVIEW(function_, size_)                              \
  PPDRAW_REGISTER_PREVIEW_NAMED(function_, size_, #function_)

// Weak implementations

#ifdef __cplusplus
}
#endif
