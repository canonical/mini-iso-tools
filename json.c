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


char *find_largest_subkey(json_object *obj)
{
    char *ret = NULL;
    json_object_object_foreach(obj, key, val) {
        (void)val;
        if(!ret || lt(ret, key)) {
            ret = key;
        }
    }
    return ret;
}

choices_t *read_iso_choices(char *filename)
{
    json_object *root = json_object_from_file(filename);
    if(!root) return NULL;

    json_object *products = get(root, "products");
    if(!products) return NULL;

    json_object *product = get(products,
            "com.ubuntu.cdimage.daily:ubuntu-server:daily-live:23.04:amd64");
    if(!product) return NULL;

    json_object *codename = get(product, "release_codename");
    if(!codename) return NULL;
    json_object *title = get(product, "release_title");
    if(!title) return NULL;

    json_object *versions = get(product, "versions");
    if(!versions) return NULL;

    char *recent = find_largest_subkey(versions);
    if(!recent) return NULL;
    json_object *date = get(versions, recent);
    if(!date) return NULL;
    json_object *items = get(date, "items");
    if(!items) return NULL;
    json_object *iso = get(items, "iso");
    if(!iso) return NULL;

    json_object *path = get(iso, "path");
    if(!path) return NULL;
    json_object *size = get(iso, "size");
    if(!size) return NULL;
    json_object *sha256 = get(iso, "sha256");
    if(!sha256) return NULL;

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
