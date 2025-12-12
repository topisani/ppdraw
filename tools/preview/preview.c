#include "ppdraw/registry.h"
#include <ppdraw/draw.h>
#include <ppdraw/preview.h>

#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

extern struct ppdraw_preview_widget __start_ppdraw_preview_registry[];
extern struct ppdraw_preview_widget __stop_ppdraw_preview_registry[];

size_t ppdraw_preview_count() {
  return __stop_ppdraw_preview_registry - __start_ppdraw_preview_registry;
}

struct ppdraw_preview_widget *ppdraw_preview_get(size_t index) {
  return &__start_ppdraw_preview_registry[index];
}

static draw_fb_t *ppdraw_fb_alloc(int width, int height, int bpp) {
  size_t buf_size = (width * height * bpp + 7) / 8;
  uint8_t *buf = malloc(buf_size);
  memset(buf, 0, buf_size);
  draw_fb_t *fb = malloc(sizeof(draw_fb_t));
  fb->width = width;
  fb->height = height;
  fb->buf = buf;
  fb->buf_size = buf_size;
  fb->bpp = bpp;
  return fb;
}

static void ppdraw_fb_free(draw_t *ctx) {
  free(ctx->fb->buf);
  free(ctx->fb);
}

static struct ppdraw_preview_widget *find_widget_by_name(const char *name) {
  for (int i = 0; i < ppdraw_preview_count(); i++) {
    struct ppdraw_preview_widget *w = ppdraw_preview_get(i);
    if (strcmp(name, w->name) == 0) {
      return w;
    }
  }
  return NULL;
};

// Params

enum param_kind {
  PARAM_INT,
};

struct param {
  const char *name;
  enum param_kind kind;
  int min;
  int max;
  int value;
  bool reported;
};

#define MAX_PARAMS 64
static struct param params[MAX_PARAMS];

static int find_param(const char *name, struct param **param) {
  for (int i = 0; i < MAX_PARAMS; i++) {
    *param = &params[i];
    if (params[i].name == NULL) {
      return -1;
    }
    if (strcmp(params[i].name, name) == 0) {
      return 0;
    }
  }
  *param = NULL;
  return -1;
}

static void process_param_inputs(const char *name) {
  char *line = NULL;
  size_t len;
  ssize_t read;
  while ((read = getline(&line, &len, stdin)) != -1) {
    if (read > 0 && line[read - 1] == '\n') {
      line[read - 1] = '\0';
      read--;
    }

    if (strncmp(line, "param=", 6) == 0) {
      char *rest = line + 6;
      char *colon = strchr(rest, ':');
      if (colon == NULL) {
        continue;
      }
      *colon = '\0';
      char *param_name = rest;
      char *param_val_str = colon + 1;
      int param_val = atoi(param_val_str);

      struct param *param = NULL;
      if (find_param(param_name, &param) < 0) {
        if (param == NULL) {
          fprintf(stderr, "Maximum parameters reached");
          continue;
        }

        char *newname = malloc(strlen(param_name) + 1);
        strcpy(newname, param_name);
        param->name = newname;
      }

      param->value = param_val;

      if (param->name == name) {
        return;
      }
    }
  }
  free(line);
}

int ppdraw_preview_param_int(const char *name, int min, int max, int def) {
  struct param *param = NULL;
  if (find_param(name, &param) < 0) {
    if (param == NULL) {
      fprintf(stderr, "Maximum parameters reached");
      return def;
    }

    param->name = name;
    param->value = def;
  }
  param->kind = PARAM_INT;
  param->min = min;
  param->max = max;
  if (!param->reported) {
    printf("param=%s:%s:%d:%d:%d\n", param->name, "int", param->min, param->max,
           param->value);
    fflush(stdout);
    param->reported = true;
  }
  process_param_inputs(param->name);
  return param->value;
}

// In/out

static draw_fb_t *
ppdraw_preview_render(const struct ppdraw_preview_widget *widget) {
  draw_fb_t *fb = ppdraw_fb_alloc(widget->size.x, widget->size.y, 8);
  draw_t ctx;
  draw_init(&ctx, fb);
  widget->draw_cb(ctx);
  return fb;
}

static void print_data(uint8_t *data, int size) {
  for (int i = 0; i < size; i++) {
    printf("%02X", data[i]);
  }
}

int main(int argc, const char **argv) {
  if (argc == 1) {
    // list widgets
    for (int i = 0; i < ppdraw_preview_count(); i++) {
      printf("%s\n", ppdraw_preview_get(i)->name);
    }
    return 0;
  }
  if (argc == 2) {
    // render widget
    const char *widget_name = argv[1];
    struct ppdraw_preview_widget *widget = find_widget_by_name(widget_name);
    if (widget == NULL) {
      fprintf(stderr, "Widget not found: '%s'\n", widget_name);
      return -1;
    }

    draw_fb_t *fb = ppdraw_preview_render(widget);

    printf("width=%d\n", fb->width);
    printf("height=%d\n", fb->height);
    printf("bpp=%d\n", fb->bpp);
    printf("data=");
    print_data(fb->buf, fb->buf_size);
    printf("\n");

    return 0;
  }
}
