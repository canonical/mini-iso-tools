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
#include <syslog.h>

#include <json-c/json.h>

#include "json.h"

/* The way this mini.iso chainboots depends on PMEM kernel modules,
 * and ISOs below 22.04.2 have kernels that don't have those modules.*/
#define MINIMUM_UBUNTU_VERSION "22.04.2"

criteria_t content_id_to_criteria[] = {
    {
        .content_id = "com.ubuntu.cdimage.daily:ubuntu",
        .os = "ubuntu",
        .image_type = "daily-live",
        .urlbase = "https://cdimage.ubuntu.com",
        .descriptor = "Ubuntu Desktop",
    },
    {
        .content_id = "com.ubuntu.cdimage.daily:ubuntu-server",
        .os = "ubuntu-server",
        .image_type = "daily-live",
        .urlbase = "https://cdimage.ubuntu.com",
        .descriptor = "Ubuntu Server",
    },
    {
        .content_id = "com.ubuntu.releases:ubuntu",
        .os = "ubuntu",
        .image_type = "desktop",
        .urlbase = "https://releases.ubuntu.com",
        .descriptor = "Ubuntu Desktop",
    },
    {
        .content_id = "com.ubuntu.releases:ubuntu-server",
        .os = "ubuntu-server",
        .image_type = "live-server",
        .urlbase = "https://releases.ubuntu.com",
        .descriptor = "Ubuntu Server",
    },
    {} /* must be last */
};

criteria_t *criteria_for_content_id(const char *content_id)
{
    if(!content_id) return NULL;
    for(int i = 0; content_id_to_criteria[i].content_id; i++) {
        if(eq(content_id, content_id_to_criteria[i].content_id)) {
            return &content_id_to_criteria[i];
        }
    }
    return NULL;
}

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

iso_data_t *iso_data_for_product(json_object *product, criteria_t *criteria)
{
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

    return iso_data_create(
            saprintf("%s %s (%s)",
                     criteria->descriptor, str(title), str(codename)),
            saprintf("%s/%s", criteria->urlbase, str(path)),
            strdup(str(sha256)),
            json_object_get_int64(size));
}

bool choices_extend_from_json(choices_t *choices, const char *filename,
                              const char *arch)
{
    /* extend the choices available to include all viable isos
     * found in this file */
    json_object *root = json_object_from_file(filename);
    if(!root) return false;

    const char *content_id = str(get(root, "content_id"));
    criteria_t *criteria = criteria_for_content_id(content_id);
    if(!criteria) return false;

    json_object *products = get(root, "products");
    if(!products) return false;

    json_object_object_foreach(products, product_key, product) {
        (void)product_key;

        if(!eq(str(get(product, "arch")), arch)) continue;
        if(!eq(str(get(product, "os")), criteria->os)) continue;
        if(!eq(str(get(product, "image_type")), criteria->image_type))
            continue;
        if(lt(str(get(product, "release_title")), MINIMUM_UBUNTU_VERSION))
            continue;
        json_object *versions = get(product, "versions");
        if(!versions) continue;
        json_object *newest = find_largest_key(versions, NULL);
        if(!newest) continue;

        if(!choices_append(choices, iso_data_for_product(product, criteria)))
            return false;

    }

    json_object_put(root);
    return true;
}

iso_data_t *get_newest_iso(const char *filename, const char *arch)
{
    json_object *root = json_object_from_file(filename);
    if(!root) return NULL;

    const char *content_id = str(get(root, "content_id"));
    criteria_t *criteria = criteria_for_content_id(content_id);
    if(!criteria) return NULL;

    /* I want to iterate not just over newest product, but over all products
     * above a given version */
    json_object *product = find_newest_product(get(root, "products"), NULL,
            arch, criteria->os, criteria->image_type);
    if(!product) return NULL;

    iso_data_t *ret = iso_data_for_product(product, criteria);
    json_object_put(root);
    return ret;
}
