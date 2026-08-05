#include "menu_wifi.h"

#include <stdio.h>
#include <string.h>

#include "circle.h"
#include "menu.h"

static struct menu_item *wifi_ssid_item;
static struct menu_item *wifi_security_item;
static struct wifi_access_point wifi_access_points[MAX_WIFI_ACCESS_POINTS];
static int wifi_access_point_count;

static int wifi_signal_percentage(int signal) {
  int percentage = (signal + 100) * 100 / 70;
  if (percentage < 0) {
    return 0;
  }
  if (percentage > 100) {
    return 100;
  }
  return percentage;
}

static void sort_wifi_access_points(void) {
  for (int index = 1; index < wifi_access_point_count; index++) {
    struct wifi_access_point access_point = wifi_access_points[index];
    int sorted_index = index;
    while (sorted_index > 0 &&
           wifi_access_points[sorted_index - 1].signal < access_point.signal) {
      wifi_access_points[sorted_index] =
          wifi_access_points[sorted_index - 1];
      sorted_index--;
    }
    wifi_access_points[sorted_index] = access_point;
  }
}

static void wifi_access_point_selected(struct menu_item *item) {
  if (item->value < 0 || item->value >= wifi_access_point_count) {
    return;
  }

  strncpy(wifi_ssid_item->str_value, wifi_access_points[item->value].ssid,
          wifi_ssid_item->max_length);
  wifi_ssid_item->str_value[wifi_ssid_item->max_length] = '\0';
  wifi_ssid_item->value = strlen(wifi_ssid_item->str_value);
  wifi_security_item->value = wifi_access_points[item->value].secure ? 0 : 1;
  ui_pop_menu();
}

void show_wifi_access_points(struct menu_item *ssid_item,
                             struct menu_item *security_item) {
  wifi_access_point_count = circle_scan_wifi_access_points(
      wifi_access_points, MAX_WIFI_ACCESS_POINTS);
  ui_pop_menu();
  struct menu_item *root = ui_push_menu(42, 12);

  wifi_ssid_item = ssid_item;
  wifi_security_item = security_item;
  if (wifi_access_point_count <= 0) {
    ui_menu_add_button(MENU_ID_DO_NOTHING, root, "No WiFi networks found")->disabled = 1;
    return;
  }

  sort_wifi_access_points();

  for (int index = 0; index < wifi_access_point_count; index++) {
    char signal_label[MAX_DSP_VAL_LEN];
    snprintf(signal_label, sizeof(signal_label), "%d%% %c",
             wifi_signal_percentage(wifi_access_points[index].signal),
             wifi_access_points[index].secure ? '*' : 'o');
    struct menu_item *item = ui_menu_add_button_with_value(
        MENU_WIFI_SSID, root, wifi_access_points[index].ssid, index, "",
        signal_label);
    item->on_value_changed = wifi_access_point_selected;
  }
}