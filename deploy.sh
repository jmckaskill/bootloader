#!/bin/sh

DISK=$1

./build.sh bootloader.MLO || exit 1
sudo dd if=raw-mmc-header.img of=$DISK || exit 1
sudo dd if=bootloader.MLO of=$DISK seek=1 || exit 1
