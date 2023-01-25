/*
 * Copyright 2022-2023 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#define _GNU_SOURCE

#define UNUSED(X) __attribute__((unused(X)))

typedef struct _iso_data
{
    char *label;
    char *url;
    char *sha256sum;
    int size;
} iso_data_t;

typedef struct _choices
{
    int len;
    int cur;
    iso_data_t **values;
} choices_t;

iso_data_t *iso_data_create(char *label, char *url, char *sha256sum, int size);
void iso_data_free(iso_data_t *iso_data);

choices_t *choices_create(int len);
void choices_free(choices_t *c);

char *saprintf(char *fmt, ...) __attribute__((format(printf, 1, 2)));
