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

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/param.h>

#include <json-c/json.h>

#include "json.h"

json_object *get(json_object *obj, const char *key)
{
    return json_object_object_get(obj, key);
}

const char *str(json_object *obj)
{
    return json_object_get_string(obj);
}

bool eq(const char *a, const char *b)
{
    return strcmp(a, b) == 0;
}

bool lt(const char *a, const char *b)
{
    return strcmp(a, b) < 0;
}

bool ubuntu_version_lt(const char *a, const char *b)
{
    return lt(a, b);
}

json_object *find_obj_of_biggest_key(json_object *obj)
{
    if(!obj) return NULL;

    char *cmp = NULL;
    json_object *ret = NULL;

    json_object_object_foreach(obj, key, val) {
        if(!cmp || lt(cmp, key)) {
            cmp = key;
            ret = val;
        }
    }
    return ret;
}

const char *find_newest_product_key(json_object *products)
{
    if(!products) return NULL;

    const char *cmp = NULL;
    const char *cmp_version = NULL;

    json_object_object_foreach(products, key, val) {
        if(!eq(str(get(val, "arch")), "amd64")) {
            continue;
        }
        if(!eq(str(get(val, "os")), "ubuntu-server")) {
            continue;
        }
        if(!eq(str(get(val, "image_type")), "daily-live")) {
            continue;
        }
        const char *version = str(get(val, "version"));
        if(!cmp || lt(cmp_version, version)) {
            cmp = key;
            cmp_version = version;
            continue;
        }
    }
    return cmp;
}

choices_t *read_iso_choices(char *filename)
{
    json_object *root = json_object_from_file(filename);
    if(!root) return NULL;
    json_object *products = get(root, "products");
    const char *product_key = find_newest_product_key(products);
    json_object *product = get(products, product_key);
    if(!product) return NULL;
    json_object *newest = find_obj_of_biggest_key(get(product, "versions"));
    if(!newest) return NULL;
    json_object *iso = get(get(newest, "items"), "iso");
    if(!iso) return NULL;

    json_object *title = get(product, "release_title");
    if(!title) return NULL;
    json_object *codename = get(product, "release_codename");
    if(!codename) return NULL;
    json_object *path = get(iso, "path");
    if(!path) return NULL;
    json_object *sha256 = get(iso, "sha256");
    if(!sha256) return NULL;
    json_object *size = get(iso, "size");
    if(!size) return NULL;

    choices_t *choices = choices_create(2);
    choices->values[0] = iso_data_create(
            strdup("Ubuntu Server 22.10 (Kinetic Kudu)"),
            strdup("https://releases.ubuntu.com/kinetic/ubuntu-22.10-live-server-amd64.iso"),
            strdup("874452797430a94ca240c95d8503035aa145bd03ef7d84f9b23b78f3c5099aed"),
            1642631168);
    choices->values[1] = iso_data_create(
            saprintf("Ubuntu Server %s (%s)", str(title), str(codename)),
            saprintf("https://cdimage.ubuntu.com/%s", str(path)),
            strdup(str(sha256)),
            json_object_get_int(size));

    json_object_put(root);
    return choices;
}
