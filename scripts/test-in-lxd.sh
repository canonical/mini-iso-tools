#!/bin/bash
set -eux

IMAGE=$1
SCRIPT=$2
SERIES=${IMAGE##*:}
TESTER=mini-iso-tools-$SERIES

lxd init --auto

lxc delete --force $TESTER || true

if [ -z "$(lxc list -f csv -c n ^${TESTER}\$)" ]
then
    lxc launch $IMAGE $TESTER
    lxc config device add $TESTER code disk source=`pwd` path=/src
else
    lxc start $TESTER
fi

lxc exec $TESTER -- sh -ec "
    cd ~
    cp -a /src .
    [ -d ~/src ]
    "

attempts=0
while ! lxc file pull $TESTER/etc/resolv.conf - 2> /dev/null | grep -q ^nameserver; do
    sleep 1
    attempts=$((attempts+1))
    if [ $attempts -ge 30 ]; then
        lxc file pull $TESTER/etc/resolv.conf
        lxc exec $TESTER -- ps aux
        echo "Network failed to come up after 30 seconds"
        exit 1
    fi
done
if ! lxc file pull $TESTER/etc/resolv.conf - 2> /dev/null | grep ^nameserver | grep -qv 127.0.0.53
then
    echo "systemd-resolved"
    while ! lxc file pull $TESTER/run/systemd/resolve/resolv.conf - 2> /dev/null | grep -v fe80 | grep -q ^nameserver; do
        sleep 1
        attempts=$((attempts+1))
        if [ $attempts -ge 30 ]; then
            echo "Network failed to come up after 30 seconds"
            exit 1
        fi
    done
fi

lxc exec $TESTER -- cloud-init status --wait

# Filthy build-deps list. mk-build-deps would be cleaner but it causes ~250
# extra packages to be installed, and then we would want to remove them.
build_deps=$(
    sed -n -e '/Build-Depends/,/^[^ ]/p' debian/control \
    | grep -E '^ ' | grep -v debhelper-compat | cut -d, -f1 | xargs
)

lxc exec $TESTER -- sh -ec "
    cd ~/src
    DEBIAN_FRONTEND=noninteractive apt-get update
    DEBIAN_FRONTEND=noninteractive apt-get install -y build-essential debhelper $build_deps
    $SCRIPT"

lxc stop $TESTER
