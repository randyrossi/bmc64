#ifndef RASPI_MENU_WIFI_H
#define RASPI_MENU_WIFI_H

#include "ui.h"

void show_wifi_access_points(struct menu_item *ssid_item,
                             struct menu_item *security_item);

#endif