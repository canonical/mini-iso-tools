/*
 * Copyright 2022-2023 Canonical Ltd.
 *
 * SPDX-License-Identifier: GPL-3.0
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "common.h"

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

char *saprintf(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    char *out = NULL;
    int rv = vasprintf(&out, fmt, ap);
    if(rv == -1) return NULL;
    return out;
}

/* create the iso_data_t structure.  Caller allocates a free()able string, and
 * a later call to iso_data_free() will release both the iso_data_t and the
 * strings supplied here. */
iso_data_t *iso_data_create(char *label, char *url, char *sha256sum,
                            int64_t size)
{
    iso_data_t *ret = calloc(sizeof(iso_data_t), 1);
    if(!ret) return NULL;

    ret->label = label;
    ret->url = url;
    ret->sha256sum = sha256sum;
    ret->size = size;
    return ret;
}

void iso_data_free(iso_data_t *iso_data)
{
    if(!iso_data) return;
    free(iso_data->label);
    free(iso_data->url);
    free(iso_data);
}

choices_t *choices_create(int capacity)
{
    choices_t *ret = (choices_t *)calloc(sizeof(choices_t), 1);
    if(!ret) return NULL;

    ret->values = (iso_data_t **)calloc(sizeof(iso_data_t *), capacity);
    if(!ret->values) {
        free(ret);
        return NULL;
    }
    ret->capacity = capacity;
    return ret;
}

void choices_free(choices_t *choices)
{
    if(!choices) return;

    for(int i = 0; i < choices->len; i++) {
        iso_data_free(choices->values[i]);
    }
    free(choices);
}

bool choices_append(choices_t *choices, iso_data_t *data)
{
    if(choices->len < choices->capacity) {
        choices->values[choices->len++] = data;
        return true;
    }
    return false;
}
