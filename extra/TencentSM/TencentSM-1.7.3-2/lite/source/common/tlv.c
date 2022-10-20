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

#include <string.h>
#include <stdlib.h>

#include "../include/tlv.h"

#define tlv_foreach(element, array)  \
    for(element = (array != NULL) ? (array)->tlv_head: NULL; element != NULL; element = element->next)

tlv_box_t* tlv_box_create()
{
    tlv_box_t *box = calloc(1, sizeof(tlv_box_t));
    if (!box) {
        return box;
    }

    box->tlv_type = TLV_OBJECT;
    return box;
}

tlv_box_t* tlv_box_create_array()
{
    tlv_box_t *box = calloc(1, sizeof(tlv_box_t));
    if (!box) {
        return box;
    }

    box->tlv_type = TLV_ARRAY;
    return box;
}

static void add_object(tlv_box_t *box, tlv_entry_t *entry)
{
    entry->next = box->tlv_head;
    if (box->tlv_head)
        box->tlv_head->prev = entry;

    box->tlv_head = entry;
    ++ box->num_of_entry;
}

tlv_bool tlv_is_array(const tlv_box_t * const array)
{
    return array->tlv_type == TLV_ARRAY ? 1 : 0;
}

static void remove_entry(tlv_entry_t **head, tlv_entry_t *entry)
{
    if (entry == NULL)
        return;

    tlv_entry_t *prev = entry->prev;
    tlv_entry_t *next = entry->next;

    if (prev != NULL) {
        prev->next = next;
    } else {
        *head = next;;
    }

    if (next != NULL) {
        next->prev = prev;
    }
}

int tlv_delete_item_from_array(tlv_box_t *box, uint32_t which)
{
    if (which >= box->num_of_entry) {
        return -1;
    }

    uint32_t i = 0;
    tlv_entry_t *entry = NULL;
    tlv_foreach(entry, box) {
        if ((i ++) == which) {
            remove_entry(&box->tlv_head, entry);
            --box->num_of_entry;
            box->serialized_size -= (entry->t.length + sizeof(tlv_t));
            free(entry);
            return 0;
        }
    }

    return -1;
}

int tlv_delete_item_from_object(tlv_box_t *box, uint32_t type)
{
    tlv_entry_t *entry = NULL;
    tlv_foreach(entry, box) {
        if (type == entry->t.type) {
            remove_entry(&box->tlv_head, entry);
            --box->num_of_entry;
            free(entry);
            return 0;
        }
    }

    return -1;
}

int tlv_add_object_to_array(tlv_box_t *box, tlv_box_t *item)
{
    uint8_t *bufptr = NULL;
    size_t size = 0;
    int ret = tlv_serialized_box(item, &bufptr, &size);
    if (ret != 0) {
        return -1;
    }

    tlv_entry_t *entry = calloc(1, sizeof(tlv_entry_t) + size);
    if (!entry) {
        return -2;
    }

    entry->t.tlv_type = TLV_ARRAY;
    entry->t.type = 0x0;
    entry->t.length =  (uint32_t)size;

    memcpy(entry->t.data, bufptr, size);
    add_object(box, entry);
    box->serialized_size += (sizeof(tlv_t) + size);
    return 0;
}

tlv_box_t* tlv_get_array_item(const tlv_box_t *array, int index)
{
    //TODO
    return NULL;
}

int tlv_add_object(tlv_box_t *box, uint32_t type, uint8_t *value, uint32_t len)
{
    tlv_entry_t *entry = calloc(1, sizeof(tlv_entry_t) + len);
    if (!entry) {
        return -1;
    }

    entry->t.tlv_type = TLV_OBJECT;
    entry->t.type = type;
    entry->t.length = len;
    memcpy(entry->t.data, value, len);

    add_object(box, entry);
    box->serialized_size += (sizeof(tlv_t) + len);
    return 0;
}

int tlv_get_object_size(const tlv_box_t *box)
{
    return (int)(box->num_of_entry);
}

int tlv_get_object_by_type(const tlv_box_t *box, uint32_t type, uint8_t **value, uint32_t *len)
{
    tlv_entry_t *entry = NULL;
    tlv_foreach(entry, box) {
        if (entry->t.type == type) {
            *value = entry->t.data;
            *len = entry->t.length;
            return 0;
        }
    }

    return -1;
}

int tlv_get_object_by_index(const tlv_box_t *box, uint32_t index, uint32_t *type, uint8_t **value, uint32_t *len)
{
    if (index >= box->num_of_entry) {
        return -1;
    }

    tlv_entry_t *entry = box->tlv_head;
    while((index--) > 0) {
        entry = entry->next;     
    }

    *type = entry->t.type;
    *value = entry->t.data;
    *len = entry->t.length;
    return 0;
}

int tlv_serialized_box(tlv_box_t *box, uint8_t **bufptr, size_t *size)
{
    uint8_t *buffer = malloc(box->serialized_size);

    if (!buffer) {
        return -1;
    }

    size_t offset = 0;
    tlv_entry_t *entry = NULL;
    tlv_foreach(entry, box) {
        memcpy(buffer + offset, &entry->t, entry->t.length + sizeof(tlv_t));
        offset += (entry->t.length + sizeof(tlv_t));
    }

    if (box->serialized_buffer) {
        free(box->serialized_buffer);
    }

    box->serialized_buffer = buffer;
    *bufptr = box->serialized_buffer;
    *size = box->serialized_size;
    return 0;
}

static void _tlv_destroy(tlv_box_t *box)
{
    if (box) {
        tlv_entry_t *head = box->tlv_head;
        while (head) {
            tlv_entry_t *entry = head->next;
            free(head);
            head = entry;
        }

        if (box->serialized_buffer) {
            free(box->serialized_buffer);
            box->serialized_buffer = NULL;
        }
    }
    return ;
}

tlv_box_t* tlv_parse(const uint8_t *bufptr, size_t size, int *err)
{
    tlv_box_t *box = calloc(1, sizeof(tlv_box_t));
    if (!box) {
        *err = -1;
        return NULL;
    }

    const uint8_t *ptr = bufptr;
    int offset = 0;
    int headSz = sizeof(tlv_t);

    uint16_t tlv_type = 0;
    size_t num_of_entry = 0;

    *err = -2;
    while ((offset + headSz) < size) {
        tlv_t *t = (tlv_t *)ptr;
        if (t->length + headSz > size) {
            goto err;
        }

        tlv_entry_t *entry = (tlv_entry_t *)calloc(1, sizeof(tlv_entry_t) + t->length);
        memcpy(&entry->t, t, t->length + headSz);

        if (tlv_type != 0 &&
                tlv_type != t->tlv_type) {
            goto err;
        }

        tlv_type = t->tlv_type;
        add_object(box, entry);
        offset += (t->length + headSz);
        ptr += (t->length + headSz);
        ++ num_of_entry;
    }

    *err = 0;
    box->tlv_type = tlv_type;
    box->serialized_size = offset;
    box->num_of_entry = num_of_entry;

    return box;
err:
    _tlv_destroy(box);
    return NULL;
}

void tlv_box_destroy(tlv_box_t *box)
{
    _tlv_destroy(box);
    return ;
}
