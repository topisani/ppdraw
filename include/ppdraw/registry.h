#pragma once

#include <stddef.h>

#include "ppdraw/draw.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ppdraw_widget {
  int width;
  int height;
  void (*draw_cb)(draw_t ctx);
};

struct ppdraw_widget_registration {
  struct ppdraw_widget *widget;
  const char *name;
} __attribute__((aligned(8)));

size_t ppdraw_widget_registry_count();
struct ppdraw_widget_registration * ppdraw_widget_registry_get(size_t index);

#define PPDRAW_REGISTER_WIDGET(widget_name)                                    \
  extern struct ppdraw_widget widget_name;                                              \
  struct ppdraw_widget_registration                                     \
      _ppdraw_widget_registration_##widget_name                                \
      __attribute__((section("ppdraw_widget_registry"), used)) = {             \
          .widget = &widget_name,                                              \
          .name = #widget_name,                                                \
  };                                                                           \
  struct ppdraw_widget widget_name

#ifdef __cplusplus
}
#endif
