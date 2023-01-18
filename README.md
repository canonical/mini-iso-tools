
# Ubuntu mini ISO tools

## What is this?

A special package that, when included in a livecd-rootfs build similar to how
casper is included, adds a menu system presenting a list of potential other
installation ISOs that can be chain-booted to.

This package is not intended to be included on an end-user system and is only
heading to the Ubuntu archive for architectural reasons in how live builds
operate.

## What does it do?

* Has initramfs hooks to add prerequisites
* Includes casper-compatible scripts to start the process
* Downloads simplestreams JSON data to get the latest ISO information
* Presents a ncurses menu

## Contents

* `hooks` - initramfs hooks to stage in the initrd the things we need
* `scripts` - capser integration and wrapper scripts to get the menu showing
* `share` - other stuff, currently just the Subiquity font
* root dir - ncurses application styled to be visually similar to Subiquity
