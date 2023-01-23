#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <string.h>

static void simple(void **state)
{
	assert_int_equal(2, 1 + 1);
}

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(simple),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
