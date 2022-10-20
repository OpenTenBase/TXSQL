/* Copyright 2019, Tencent Technology (Shenzhen) Co Ltd

This file is part of the Tencent SM (Lite Version) Library.

The Tencent SM (Lite Version) Library is free software; you can redistribute it and/or modify
it under the terms of either:

* the GNU Lesser General Public License as published by the Free
Software Foundation; either version 3 of the License, or (at your
option) any later version.

or

* the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any
later version.

or both in parallel, as here.

The Tencent SM (Lite Version) Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received copies of the GNU General Public License and the
GNU Lesser General Public License along with the Tencent SM (Lite Version) Library.  If not,
see https://www.gnu.org/licenses/.  */

#ifndef TENCENTSM_LITE_SOURCE_TLV_H_
#define TENCENTSM_LITE_SOURCE_TLV_H_

#include <stdint.h>

#define TLV_OBJECT 0x1
#define TLV_ARRAY 0x2

typedef struct tlv_t {
  uint16_t tlv_type;
  uint16_t type;
  uint32_t length;
  uint8_t data[0];
} tlv_t;

typedef struct tlv_entry_t {
  struct tlv_entry_t *next;
  struct tlv_entry_t *prev;
  struct tlv_entry_t *child;
  tlv_t t;
} tlv_entry_t;

typedef struct tlv_box_t {
  int tlv_type;
  tlv_entry_t *tlv_head;
  size_t num_of_entry;
  uint8_t *serialized_buffer;
  uint32_t serialized_size;
} tlv_box_t;

typedef int tlv_bool;

tlv_box_t *tlv_box_create(void);
tlv_box_t *tlv_box_create_array(void);
void tlv_box_destroy(tlv_box_t *box);

tlv_box_t *tlv_get_array_item(const tlv_box_t *array, int index);
tlv_bool tlv_is_array(const tlv_box_t *const array);
int tlv_get_object_size(const tlv_box_t *box);
int tlv_add_object_to_array(tlv_box_t *box, tlv_box_t *item);
int tlv_delete_item_from_array(tlv_box_t *box, uint32_t which);
int tlv_delete_item_from_object(tlv_box_t *box, uint32_t type);
int tlv_add_object(tlv_box_t *box, uint32_t type, uint8_t *value, uint32_t len);
int tlv_get_object_by_type(const tlv_box_t *box, uint32_t type, uint8_t **value, uint32_t *len);
int tlv_get_object_by_index(const tlv_box_t *box, uint32_t index, uint32_t *type, uint8_t **value,
                            uint32_t *len);
int tlv_serialized_box(tlv_box_t *box, uint8_t **bufptr, size_t *size);
tlv_box_t *tlv_parse(const uint8_t *bufptr, size_t size, int *err);

#define tlv_array_foreach(element, array)                                     \
  for (element = (array != NULL) ? (array)->tlv_head : NULL; element != NULL; \
       element = element->next)

#endif //TENCENTSM_LITE_SOURCE_TLV_H_
