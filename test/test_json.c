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

static void criteria_for_content_id_NULL(void **state)
{
    assert_null(criteria_for_content_id(NULL));
}

static void criteria_for_content_id_invalid(void **state)
{
    assert_null(criteria_for_content_id("invalid"));
}

static void criteria_for_content_id_server_cdimage(void **state)
{
    criteria_t *criteria = criteria_for_content_id(
            "com.ubuntu.cdimage.daily:ubuntu-server");
    assert_non_null(criteria);
    assert_string_equal(
            "com.ubuntu.cdimage.daily:ubuntu-server",
            criteria->content_id);
    assert_string_equal("ubuntu-server", criteria->os);
    assert_string_equal("daily-live", criteria->image_type);
    assert_string_equal("https://cdimage.ubuntu.com", criteria->urlbase);
    assert_string_equal("Ubuntu Server", criteria->descriptor);
}

extern criteria_t content_id_to_criteria[];

static void criteria_all_initialized(void **state)
{
    for(int i = 0; ; i++) {
        criteria_t *cur = &content_id_to_criteria[i];
        if(!cur->content_id) break;

        assert_non_null(cur->os);
        assert_non_null(cur->image_type);
        assert_non_null(cur->urlbase);
        assert_non_null(cur->descriptor);
    }
}

static void _test_isodata(
        int index,
        const char *filename,
        const char *arch,
        const char *expected_label,
        const char *expected_url,
        const char *expected_sha256sum,
        int64_t expected_size)
{
    choices_t *choices = choices_create(2);
    choices_extend_from_json(choices, filename, arch);
    iso_data_t *iso_data = choices->values[index];
    assert_string_equal(expected_label, iso_data->label);
    assert_string_equal(expected_url, iso_data->url);
    assert_string_equal(expected_sha256sum, iso_data->sha256sum);
    assert_int_equal(expected_size, iso_data->size);
}

static void read_ubuntu_server_cdimage(void **state)
{
    _test_isodata(
            0,
            "test/data/com.ubuntu.cdimage.daily:ubuntu-server.json",
            "amd64",
            "Ubuntu Server 23.04 (Lunar Lobster)",
            "https://cdimage.ubuntu.com/ubuntu-server/daily-live/20230122/lunar-live-server-amd64.iso",
            "b67e566f6b7ff5d314173a2b55bb413cf4ab2b1b94c59f1ff8b65b862c1d7de7",
            1762381824);
}

static void read_ubuntu_server_releases(void **state)
{
    _test_isodata(
            1,
            "test/data/com.ubuntu.releases:ubuntu-server.json",
            "amd64",
            "Ubuntu Server 22.10 (Kinetic Kudu)",
            "https://releases.ubuntu.com/kinetic/ubuntu-22.10-live-server-amd64.iso",
            "874452797430a94ca240c95d8503035aa145bd03ef7d84f9b23b78f3c5099aed",
            1642631168);
}

static void read_ubuntu_desktop_cdimage(void **state)
{
    _test_isodata(
            0,
            "test/data/com.ubuntu.cdimage.daily:ubuntu.json",
            "amd64",
            "Ubuntu Desktop 23.04 (Lunar Lobster)",
            "https://cdimage.ubuntu.com/daily-live/20230209/lunar-desktop-amd64.iso",
            "2d2a0e0894fa8c98cc564223bf41d6bf2dd9d27449ac4b30f7d42edfeb77de67",
            5877311488);
}

static void read_ubuntu_desktop_releases(void **state)
{
    _test_isodata(
            1,
            "test/data/com.ubuntu.releases:ubuntu.json",
            "amd64",
            "Ubuntu Desktop 22.10 (Kinetic Kudu)",
            "https://releases.ubuntu.com/kinetic/ubuntu-22.10-desktop-amd64.iso",
            "b98f13cd86839e70cb7757d46840230496b3febea309dd73bd5f81383474e47b",
            4071903232);
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

        cmocka_unit_test(read_ubuntu_server_cdimage),
        cmocka_unit_test(read_ubuntu_server_releases),
        cmocka_unit_test(read_ubuntu_desktop_cdimage),
        cmocka_unit_test(read_ubuntu_desktop_releases),

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

        cmocka_unit_test(criteria_for_content_id_NULL),
        cmocka_unit_test(criteria_for_content_id_invalid),
        cmocka_unit_test(criteria_for_content_id_server_cdimage),
        cmocka_unit_test(criteria_all_initialized),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
