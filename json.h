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

#include <stdbool.h>

#include <json-c/json.h>

#include "common.h"

choices_t *read_iso_choices(char *filename_cdimage, char *filename_releases);
iso_data_t *get_newest_iso(char *filename,
                           const char *arch, const char *os,
                           const char *image_type, const char *urlbase);

json_object *find_largest_key(json_object *obj, const char **ret_key);
json_object *find_newest_product(json_object *products, const char **ret_key,
                                 const char *arch, const char *os,
                                 const char *image_type);

json_object *get(json_object *obj, const char *key);
const char *str(json_object *obj);
bool eq(const char *a, const char *b);
bool lt(const char *a, const char *b);
bool ubuntu_version_lt(const char *a, const char *b);
