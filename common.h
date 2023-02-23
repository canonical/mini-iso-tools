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

#pragma once

#define _GNU_SOURCE

#define UNUSED(X) __attribute__((unused(X)))

#include <stdbool.h>
#include <stdint.h>

typedef struct _iso_data
{
    char *label;
    char *url;
    char *sha256sum;
    int64_t size;
} iso_data_t;

typedef struct _choices
{
    int capacity; /* number of items allocated in the values array */
    int cur; /* index of the currently selected choice */
    int len; /* how many items in the values array actually used */
    iso_data_t **values; /* data array of iso choices */
} choices_t;

iso_data_t *iso_data_create(char *label, char *url, char *sha256sum,
                            int64_t size);
void iso_data_free(iso_data_t *iso_data);

choices_t *choices_create(int len);
void choices_free(choices_t *choices);
bool choices_append(choices_t *choices, iso_data_t *data);

char *saprintf(char *fmt, ...) __attribute__((format(printf, 1, 2)));
