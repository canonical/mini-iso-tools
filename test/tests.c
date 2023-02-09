#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>

#include <json-c/json.h>

#include "json.h"

static void find_largest_NULL(void **state)
{
    assert_null(find_largest_key(NULL, NULL));
}

static void find_largest_simple(void **state)
{
    json_object *root = json_tokener_parse("{'a': 1, 'b': 2, 'c': 3}");
    assert_non_null(root);
    const char *key = NULL;
    json_object *largest = find_largest_key(root, &key);
    assert_string_equal("c", key);
    assert_non_null(largest);
    assert_int_equal(3, json_object_get_int(largest));
    assert_int_equal(largest, find_largest_key(root, NULL));
}

static void find_largest_reversed(void **state)
{
    json_object *root = json_tokener_parse("{'c': 3, 'b': 2, 'a': 1}");
    assert_non_null(root);
    const char *key = NULL;
    json_object *largest = find_largest_key(root, &key);
    assert_string_equal("c", key);
    assert_non_null(largest);
    assert_int_equal(3, json_object_get_int(largest));
}

static void newest_product_NULL(void **state)
{
    assert_null(find_newest_product(NULL, NULL, NULL, NULL, NULL));
}

static void newest_product_basic(void **state)
{
    json_object *root = json_tokener_parse("{"
        "'a': {"
            "'arch': 'amd64',"
            "'os': 'ubuntu-server',"
            "'image_type': 'daily-live',"
            "'version': '1'"
        "}"
    "}");
    const char *key = NULL;
    json_object *obj_a = find_newest_product(root, &key,
            "amd64", "ubuntu-server", "daily-live");
    assert_string_equal("a", key);

    assert_int_equal(obj_a, get(root, "a"));

    assert_int_equal(obj_a, find_newest_product(root, NULL,
            "amd64", "ubuntu-server", "daily-live"));
}

static void newest_product_first(void **state)
{
    json_object *root = json_tokener_parse("{"
        "'b': {"
            "'arch': 'amd64',"
            "'os': 'ubuntu-server',"
            "'image_type': 'daily-live',"
            "'version': '2'"
        "},"
        "'a': {"
            "'arch': 'amd64',"
            "'os': 'ubuntu-server',"
            "'image_type': 'daily-live',"
            "'version': '1'"
        "}"
    "}");
    const char *key = NULL;
    assert_non_null(find_newest_product(root, &key,
            "amd64", "ubuntu-server", "daily-live"));
    assert_string_equal("b", key);
}

static void newest_product_second(void **state)
{
    json_object *root = json_tokener_parse("{"
        "'b': {"
            "'arch': 'amd64',"
            "'os': 'ubuntu-server',"
            "'image_type': 'daily-live',"
            "'version': '1'"
        "},"
        "'a': {"
            "'arch': 'amd64',"
            "'os': 'ubuntu-server',"
            "'image_type': 'daily-live',"
            "'version': '2'"
        "}"
    "}");
    const char *key = NULL;
    assert_non_null(find_newest_product(root, &key,
            "amd64", "ubuntu-server", "daily-live"));
    assert_string_equal("a", key);
}

static void read_NULL(void **state)
{
    assert_null(get_newest_iso(NULL, NULL, NULL, NULL, NULL));
}

static void read_not_exist(void **state)
{
    assert_null(get_newest_iso("/not/exist", NULL, NULL, NULL, NULL));
}

static void read_empty_obj(void **state)
{
    assert_null(get_newest_iso("test/emtpy-obj.json", NULL, NULL, NULL, NULL));
}

static void read_ubuntu_server(void **state)
{
    iso_data_t *iso_data = get_newest_iso("test/ubuntu-server.json",
            "amd64", "ubuntu-server", "daily-live",
            "https://cdimage.ubuntu.com");
    assert_string_equal("Ubuntu Server 23.04 (Lunar Lobster)", iso_data->label);
    assert_string_equal(
            "https://cdimage.ubuntu.com/ubuntu-server/daily-live/20230122/lunar-live-server-amd64.iso",
            iso_data->url);
    assert_string_equal(
            "b67e566f6b7ff5d314173a2b55bb413cf4ab2b1b94c59f1ff8b65b862c1d7de7",
            iso_data->sha256sum);
    assert_int_equal(1762381824, iso_data->size);
}

static void read_ubuntu_server_releases(void **state)
{
    iso_data_t *iso_data = get_newest_iso("test/com.ubuntu.releases:ubuntu-server.json",
            "amd64", "ubuntu-server", "live-server",
            "https://releases.ubuntu.com");
    assert_string_equal("Ubuntu Server 22.10 (Kinetic Kudu)", iso_data->label);
    assert_string_equal(
            "https://releases.ubuntu.com/kinetic/ubuntu-22.10-live-server-amd64.iso",
            iso_data->url);
    assert_string_equal(
            "874452797430a94ca240c95d8503035aa145bd03ef7d84f9b23b78f3c5099aed",
            iso_data->sha256sum);
    assert_int_equal(1642631168, iso_data->size);
}

static void eq_good(void **state)
{
    assert_true(eq("a", "a"));
}

static void eq_bad(void **state)
{
    assert_false(eq("a", "b"));
}

static void eq_NULL(void **state)
{
    assert_null(eq(NULL, "a"));
    assert_null(eq("a", NULL));
    assert_null(eq(NULL, NULL));
}

static void lt_good(void **state)
{
    assert_true(lt("a", "b"));
}

static void lt_bad(void **state)
{
    assert_false(lt("a", "a"));
}

static void lt_NULL(void **state)
{
    assert_false(lt(NULL, "a"));
    assert_false(lt("a", NULL));
    assert_false(lt(NULL, NULL));
}

static void get_good(void **state)
{
    json_object *root = json_tokener_parse("{'key': 'value'}");
    assert_non_null(get(root, "key"));
}

static void get_NULL(void **state)
{
    assert_null(get(NULL, NULL));
    assert_null(get(NULL, "key"));
    json_object *root = json_tokener_parse("{'key': 'value'}");
    assert_null(get(root, NULL));
}

static void str_good(void **state)
{
    json_object *root = json_tokener_parse("{'key': 'value'}");
    assert_string_equal("value", str(get(root, "key")));
}

static void str_NULL(void **state)
{
    assert_null(str(NULL));
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(find_largest_NULL),
        cmocka_unit_test(find_largest_simple),
        cmocka_unit_test(find_largest_reversed),

        cmocka_unit_test(newest_product_NULL),
        cmocka_unit_test(newest_product_basic),
        cmocka_unit_test(newest_product_first),
        cmocka_unit_test(newest_product_second),

        cmocka_unit_test(read_NULL),
        cmocka_unit_test(read_not_exist),
        cmocka_unit_test(read_empty_obj),
        cmocka_unit_test(read_ubuntu_server),
        cmocka_unit_test(read_ubuntu_server_releases),

        cmocka_unit_test(eq_NULL),
        cmocka_unit_test(eq_good),
        cmocka_unit_test(eq_bad),
        cmocka_unit_test(lt_NULL),
        cmocka_unit_test(lt_good),
        cmocka_unit_test(lt_bad),

        cmocka_unit_test(get_NULL),
        cmocka_unit_test(get_good),
        cmocka_unit_test(str_NULL),
        cmocka_unit_test(str_good),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
