new: clean compile

clean:
	rm -fr builddir builddir_coverage

compile: builddir
	cd $^
	meson compile

run: compile
	./builddir/iso-chooser-menu /dev/null \
		test/data/com.ubuntu.releases*json

test: builddir_coverage
	cd $^
	meson test
	ninja coverage
	cat meson-logs/coverage.txt

builddir:
	meson setup $@

builddir_coverage:
	meson setup -Db_coverage=true $@

.PHONY: new clean compile run test
.ONESHELL:
