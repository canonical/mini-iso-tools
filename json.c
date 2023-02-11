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

/* principles of the JSON layer:
 *  - absolutely everything is error checked
 *  - absolutely nobody is allowed to assume pointers are non-NULL
 *  - anything receiving an unexpected NULL may also return NULL */

#include <stdio.h>
#include <stdbool.h>
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

json_object *find_largest_key(json_object *obj, const char **ret_key)
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

    if(ret_key) *ret_key = cmp;
    return ret;
}

/* return the object, and optionally the key, of the product matching these
 * constraints with the largest version. */
json_object *find_newest_product(json_object *products, const char **ret_key,
                                 const char *arch, const char *os,
                                 const char *image_type)
{
    if(!products) return NULL;

    const char *cmp = NULL;
    const char *cmp_version = NULL;
    json_object *ret = NULL;

    json_object_object_foreach(products, key, val) {
        if(!eq(str(get(val, "arch")), arch)) {
            continue;
        }
        if(!eq(str(get(val, "os")), os)) {
            continue;
        }
        if(!eq(str(get(val, "image_type")), image_type)) {
            continue;
        }
        const char *version = str(get(val, "version"));
        if(!cmp || lt(cmp_version, version)) {
            cmp = key;
            cmp_version = version;
            ret = val;
            continue;
        }
    }

    if(ret_key) *ret_key = cmp;
    return ret;
}

iso_data_t *get_newest_iso(const char *filename,
                           const char *arch, const char *os,
                           const char *image_type,
                           const char *urlbase,
                           const char *descriptor)
{
    json_object *root = json_object_from_file(filename);
    if(!root) return NULL;

    /*
     * content_id could be used to choose release vs cdimage, desktop vs server
     * behaviors
     * com.ubuntu.cdimage.daily:ubuntu
     * com.ubuntu.cdimage.daily:ubuntu-server
     * com.ubuntu.releases:ubuntu
     * com.ubuntu.releases:ubuntu-server
     *
     * split on colon
     *   com.ubuntu.cdimage.daily -> cdimage -> image_type
     *   com.ubuntu.releases -> releases -> image_type
     *   ubuntu -> desktop -> os=ubuntu
     *   ubuntu-server -> ubuntu-server -> os=ubuntu-server
     */
    json_object *product = find_newest_product(get(root, "products"), NULL,
            arch, os, image_type);
    if(!product) return NULL;
    json_object *newest = find_largest_key(get(product, "versions"), NULL);
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

    iso_data_t *ret = iso_data_create(
            saprintf("%s %s (%s)", descriptor, str(title), str(codename)),
            saprintf("%s/%s", urlbase, str(path)),
            strdup(str(sha256)),
            json_object_get_int64(size));

    json_object_put(root);
    return ret;
}
