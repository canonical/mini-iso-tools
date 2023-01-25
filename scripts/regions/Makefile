
default: test lint

.PHONY: lint
lint:
	shellcheck get_memmap_directive test/test.bats

.PHONY: test
test:
	bats test/test.bats
