/*
 * raspi_util.c
 *
 * Written by
 *  Randy Rossi <randy.rossi@gmail.com>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */
#include "raspi_util.h"

#include <stdio.h>
#include <string.h>

// RASPI includes
#include "ui.h"

struct menu_item *sorted_merge(struct menu_item *a, struct menu_item *b) {
  struct menu_item *result = NULL;

  if (a == NULL)
    return (b);
  else if (b == NULL)
    return (a);

  int cmp = strcasecmp(a->name, b->name);
  if (cmp <= 0) {
    result = a;
    result->next = sorted_merge(a->next, b);
  } else {
    result = b;
    result->next = sorted_merge(a, b->next);
  }
  return result;
}

void split(struct menu_item *source, struct menu_item **frontRef,
           struct menu_item **backRef) {

  struct menu_item *fast;
  struct menu_item *slow;
  slow = source;
  fast = source->next;

  while (fast != NULL) {
    fast = fast->next;
    if (fast != NULL) {
      slow = slow->next;
      fast = fast->next;
    }
  }

  *frontRef = source;
  *backRef = slow->next;
  slow->next = NULL;
}

void merge_sort(struct menu_item **headRef) {
  struct menu_item *head = *headRef;
  struct menu_item *a;
  struct menu_item *b;

  if ((head == NULL) || (head->next == NULL)) {
    return;
  }

  split(head, &a, &b);

  merge_sort(&a);
  merge_sort(&b);

  *headRef = sorted_merge(a, b);
}
