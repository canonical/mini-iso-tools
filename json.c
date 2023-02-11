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

#include <string.h>

#include <json-c/json.h>

#include "json.h"

json_object *get(json_object *obj, const char *key)
{
    if(!obj || !key) return NULL;
    return json_object_object_get(obj, key);
}

const char *str(json_object *obj)
{
    if(!obj) return NULL;
    return json_object_get_string(obj);
}

bool eq(const char *a, const char *b)
{
    if(!a || !b) return false;
    return strcmp(a, b) == 0;
}

bool lt(const char *a, const char *b)
{
    if(!a || !b) return false;
    return strcmp(a, b) < 0;
}


char *find_largest_subkey(json_object *obj)
{
    char *ret = NULL;
    json_object_object_foreach(obj, key, val) {
        (void)val;
        if(!ret || strcmp(ret, key) < 0) {
            ret = key;
        }
    }
    return ret;
}

choices_t *read_iso_choices(char *filename)
{
    json_object *root = json_object_from_file(filename);
    if(!root) return NULL;

    json_object *products = json_object_object_get(root, "products");
    if(!products) return NULL;

    json_object *product = json_object_object_get(products,
            "com.ubuntu.cdimage.daily:ubuntu-server:daily-live:23.04:amd64");
    if(!product) return NULL;

    json_object *codename = json_object_object_get(
            product, "release_codename");
    if(!codename) return NULL;
    json_object *title = json_object_object_get(product, "release_title");
    if(!title) return NULL;

    json_object *versions = json_object_object_get(product, "versions");
    if(!versions) return NULL;

    char *recent = find_largest_subkey(versions);
    if(!recent) return NULL;
    json_object *date = json_object_object_get(versions, recent);
    if(!date) return NULL;
    json_object *items = json_object_object_get(date, "items");
    if(!items) return NULL;
    json_object *iso = json_object_object_get(items, "iso");
    if(!iso) return NULL;

    json_object *path = json_object_object_get(iso, "path");
    if(!path) return NULL;
    json_object *size = json_object_object_get(iso, "size");
    if(!size) return NULL;
    json_object *sha256 = json_object_object_get(iso, "sha256");
    if(!sha256) return NULL;

    choices_t *choices = choices_create(2);
    choices->values[0] = iso_data_create(
            strdup("Ubuntu Server 22.10 (Kinetic Kudu)"),
            strdup("https://releases.ubuntu.com/kinetic/ubuntu-22.10-live-server-amd64.iso"),
            strdup("874452797430a94ca240c95d8503035aa145bd03ef7d84f9b23b78f3c5099aed"),
            1642631168);
    choices->values[1] = iso_data_create(
            saprintf("Ubuntu Server %s (%s)",
                    json_object_get_string(title),
                    json_object_get_string(codename)),
            saprintf("https://cdimage.ubuntu.com/%s",
                    json_object_get_string(path)),
            strdup(json_object_get_string(sha256)),
            json_object_get_int(size));

    json_object_put(root);
    return choices;
}
