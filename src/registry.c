#include <ppdraw/registry.h>

extern struct ppdraw_widget_registration __start_ppdraw_widget_registry[];
extern struct ppdraw_widget_registration __stop_ppdraw_widget_registry[];

size_t ppdraw_widget_registry_count() {
  return __stop_ppdraw_widget_registry - __start_ppdraw_widget_registry;
}

struct ppdraw_widget_registration *ppdraw_widget_registry_get(size_t index) {
  return &__start_ppdraw_widget_registry[index];
}
