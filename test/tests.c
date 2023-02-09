#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>

#include <json-c/json.h>

#include "json.h"

json_object *find_obj_of_biggest_key(json_object *obj);

static void find_largest_NULL(void **state)
{
    assert_null(find_obj_of_biggest_key(NULL));
}

static void find_largest_simple(void **state)
{
    json_object *root = json_object_from_file(
            "test/find-largest-simple.json");
    assert_non_null(root);
    json_object *largest = find_obj_of_biggest_key(root);
    assert_non_null(largest);
    assert_int_equal(3, json_object_get_int(largest));
}

static void find_largest_reversed(void **state)
{
    json_object *root = json_object_from_file(
            "test/find-largest-reversed.json");
    assert_non_null(root);
    json_object *largest = find_obj_of_biggest_key(root);
    assert_non_null(largest);
    assert_int_equal(3, json_object_get_int(largest));
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
    choices_t *choices = read_iso_choices("test/ubuntu-server.json");
    assert_non_null(choices);
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

static void eq_good(void **state)
{
    assert_true(eq("a", "a"));
}

static void eq_bad(void **state)
{
    assert_false(eq("a", "b"));
}

static void lt_good(void **state)
{
    assert_true(lt("a", "b"));
}

static void lt_bad(void **state)
{
    assert_false(lt("a", "a"));
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(find_largest_NULL),
        cmocka_unit_test(find_largest_simple),
        cmocka_unit_test(find_largest_reversed),

        cmocka_unit_test(read_NULL),
        cmocka_unit_test(read_not_exist),
        cmocka_unit_test(read_empty_obj),
        cmocka_unit_test(read_ubuntu_server),

        cmocka_unit_test(eq_good),
        cmocka_unit_test(eq_bad),
        cmocka_unit_test(lt_good),
        cmocka_unit_test(lt_bad),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
