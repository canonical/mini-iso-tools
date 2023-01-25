#!/bin/sh

setup() {
    load '/usr/lib/bats/bats-support/load.bash'
    load '/usr/lib/bats/bats-assert/load.bash'

    tmpfile=$(mktemp)
}

teardown() {
    rm -f "$tmpfile"
}

@test "can run with file arg" {
    run ./get_memmap_directive 1 test/proc-iomem
    assert_success
    assert_output '4M!4G'
}

@test "notices invalid file" {
    run ./get_memmap_directive 1 not-exist
    assert_failure
    assert_output "input file not found"
}

@test "no 4G range" {
    echo "100000001-103f37ffff : System RAM" > "$tmpfile"
    run ./get_memmap_directive 1 "$tmpfile"
    assert_failure
    assert_output "range at address 4GiB not found"
}

@test "too small" {
    # skip
    echo "100000000-1000fffff : System RAM" > "$tmpfile"
    run ./get_memmap_directive $((1 * 1024 * 1024 + 1)) "$tmpfile"
    assert_failure
    assert_output "\
available memory insufficient
requested 4MiB
available 1MiB"
}

@test "realistic size live-server" {
    run ./get_memmap_directive 1748099072 test/proc-iomem
    assert_success
    assert_output '1668M!4G'
}

@test "no bc" {
    directive="100000000-103f37ffff : System RAM"
    expected="65485144063"
    start=100000000
    end=$(echo "$directive" | busybox awk '-F[- ]' '{print $2}')
    start_baseten=$(printf "%d" "0x$start")
    end_baseten=$(printf "%d" "0x$end")
    actual=$((end_baseten - start_baseten))
    assert_equal "$actual" "$expected"
}

@test "round up 4M" {
    expected="4"

    assert_equal "$(((1 + 3) & ~3))" "$expected"
    assert_equal "$(((2 + 3) & ~3))" "$expected"
    assert_equal "$(((3 + 3) & ~3))" "$expected"
    assert_equal "$(((4 + 3) & ~3))" "$expected"
}
