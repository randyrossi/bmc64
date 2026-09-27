#ifndef BMC64_KEYBOARD_ROUTER_H
#define BMC64_KEYBOARD_ROUTER_H

typedef enum {
  KEYBOARD_SOURCE_USB,
  KEYBOARD_SOURCE_GPIO
} keyboard_source_t;

#ifdef __cplusplus
extern "C" {
#endif

void keyboard_router_physical_key(keyboard_source_t source,
                                  unsigned physical_id, long keycode,
                                  int pressed);

#ifdef __cplusplus
}
#endif

#endif