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

json_object *get(json_object *obj, const char *key);
const char *str(json_object *obj);
bool eq(const char *a, const char *b);
bool lt(const char *a, const char *b);

/* The criteria is a mapping from a content_id to the information we need to
 * retrieve, and show info about, a given ISO. Simple stream JSON contains a
 * content_id on the top level object, which is then used to determine which
 * ISOs we should look for. */
typedef struct _criteria_t
{
    /* simplestream JSON has at the top level a content_id, which allows table
     * lookup of other necessary info */
    const char *content_id;

    /* os and image_type are both needed to uniquely locate the interesting
     * products */
    const char *os;
    const char *image_type;

    /* urlbase is the scheme and host information that needs to be
     * combined with the product path to obtain the full URL */
    const char *urlbase;
    /* descriptor is friendly description of the product,
     * such as "Ubuntu Server" */
    const char *descriptor;
} criteria_t;

criteria_t *criteria_for_content_id(const char *content_id);

bool choices_extend_from_json(choices_t *choices, const char *filename,
                              const char *arch);
iso_data_t *get_newest_iso(const char *filename, const char *arch);

json_object *find_largest_key(json_object *obj, const char **ret_key);
json_object *find_newest_product(json_object *products, const char **ret_key,
                                 const char *arch, const char *os,
                                 const char *image_type);
