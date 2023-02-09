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

#include <stdbool.h>
#include <string.h>

#include <json-c/json.h>

#include "json.h"

json_object *get(json_object *obj, char *key)
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

json_object *find_newest_product(json_object *products)
{
    /*
     * assuming https://cdimage.ubuntu.com/streams/v1/com.ubuntu.cdimage.daily:ubuntu-server.json
     * structure is
     * {
     *   products
     *     product id like com.ubuntu.cdimage.daily:ubuntu-server:daily-live:20.04:amd64
     *       arch: amd64
     *       os: ubuntu-server
     *       version: 23.04
     * }
     * select the product key with the largest version, matching arch,
     * os==ubuntu-server
     *
     * input object is the products object
     * returns the child object of the selected product
     */
    if(!products) return NULL;

    char *cmp = NULL;
    json_object *ret = NULL;

    json_object_object_foreach(products, key, val) {
        if(!eq(str(get(val, "arch")), "amd64")) {
            continue;
        }
        if(!eq(str(get(val, "os")), "ubuntu-server")) {
            continue;
        }
        if(!cmp || strcmp(cmp, key) < 0) {
            cmp = key;
            ret = val;
        }
    }
    return ret;
}

choices_t *read_iso_choices(char *filename)
{
    json_object *root = json_object_from_file(filename);
    if(!root) return NULL;
    json_object *product = get(get(root, "products"),
            "com.ubuntu.cdimage.daily:ubuntu-server:daily-live:23.04:amd64");
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
