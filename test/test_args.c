#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>

#include "args.h"

static void args_no_infiles(void **state)
{
    char *argv[] = {"program", "outfile", NULL};
    args_t *args = args_create(2, argv);
    assert_null(args);
}

static void args_one_infile(void **state)
{
    char *argv[] = {"program", "outfile", "test/data/empty-obj.json", NULL};
    args_t *args = args_create(3, argv);
    assert_non_null(args);
    assert_int_equal(1, args->num_infiles);
    assert_string_equal(argv[1], args->outfile);
    assert_string_equal(argv[2], args->infiles[0]);
}

static void args_two_infiles(void **state)
{
    char *argv[] = {
        "program",
        "outfile",
        "test/data/com.ubuntu.releases:ubuntu.json",
        "test/data/com.ubuntu.cdimage.daily:ubuntu-server.json",
        NULL
    };
    args_t *args = args_create(4, argv);
    assert_non_null(args);
    assert_int_equal(2, args->num_infiles);
    assert_string_equal(argv[1], args->outfile);
    assert_string_equal(argv[2], args->infiles[0]);
    assert_string_equal(argv[3], args->infiles[1]);
}

static void args_infile_missing(void **state)
{
    char *argv[] = {"program", "outfile", "/not/exist", NULL};
    args_t *args = args_create(3, argv);
    assert_null(args);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(args_no_infiles),
        cmocka_unit_test(args_one_infile),
        cmocka_unit_test(args_two_infiles),
        cmocka_unit_test(args_infile_missing),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
