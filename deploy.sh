#!/bin/sh

DISK=$1

./build.sh bootloader.MLO || exit 1
sudo dd if=bootloader.MLO of=$DISK || exit 1
