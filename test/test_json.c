#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>

#include <json-c/json.h>

#include "json.h"

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

char *find_largest_subkey(json_object *obj);

static void find_largest_simple(void **state)
{
    json_object *root = json_tokener_parse("{'a': 1, 'b': 2, 'c': 3}");
    assert_non_null(root);
    char *actual = find_largest_subkey(root);
    assert_string_equal("c", actual);
}

static void find_largest_reversed(void **state)
{
    json_object *root = json_tokener_parse("{'c': 3, 'b': 2, 'a': 1}");
    assert_non_null(root);
    char *actual = find_largest_subkey(root);
    assert_string_equal("c", actual);
}

static void read_NULL(void **state)
{
    assert_null(read_iso_choices(NULL));
}

static void read_not_exist(void **state)
{
    assert_null(read_iso_choices("/not/exist"));
}

static void read_empty_obj(void **state)
{
    assert_null(read_iso_choices("test/emtpy-obj.json"));
}

static void read_ubuntu_server(void **state)
{
    choices_t *choices = read_iso_choices("test/data/com.ubuntu.cdimage.daily:ubuntu-server.json");
    assert_int_equal(2, choices->len);

    iso_data_t *first = choices->values[0];
    assert_string_equal("Ubuntu Server 22.10 (Kinetic Kudu)", first->label);
    assert_string_equal(
            "https://releases.ubuntu.com/kinetic/ubuntu-22.10-live-server-amd64.iso",
            first->url);
    assert_string_equal(
            "874452797430a94ca240c95d8503035aa145bd03ef7d84f9b23b78f3c5099aed",
            first->sha256sum);
    assert_int_equal(1642631168, first->size);

    iso_data_t *second = choices->values[1];
    assert_string_equal("Ubuntu Server 23.04 (Lunar Lobster)", second->label);
    assert_string_equal(
            "https://cdimage.ubuntu.com/ubuntu-server/daily-live/20230122/lunar-live-server-amd64.iso",
            second->url);
    assert_string_equal(
            "b67e566f6b7ff5d314173a2b55bb413cf4ab2b1b94c59f1ff8b65b862c1d7de7",
            second->sha256sum);
    assert_int_equal(1762381824, second->size);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(find_largest_simple),
        cmocka_unit_test(find_largest_reversed),

        cmocka_unit_test(read_NULL),
        cmocka_unit_test(read_not_exist),
        cmocka_unit_test(read_empty_obj),
        cmocka_unit_test(read_ubuntu_server),

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
