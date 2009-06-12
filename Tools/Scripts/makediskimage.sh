#!/bin/bash
# This script is intended to be ran once ArcOS is built.
# It will create a bootable disk image that can be ran by any good
# simulator.
# The path to ArcOS source must be given as argument.

#$Id: makediskimage.sh 349 2008-05-29 01:54:02Z hro $

GRUB_DIR=$1
BIN_DIR=$2

GRUB_FILES="stage1 stage2 menu.lst"
BIN_FILES=`find $BIN_DIR -type f`

dd if=/dev/zero of=fdimage.img bs=1024 count=2880

echo 'drive a: file="fdimage.img"' > mtoolsrc
MTOOLSRC=./mtoolsrc mformat -f 2880 a:
MTOOLSRC=./mtoolsrc mmd a:/boot
MTOOLSRC=./mtoolsrc mmd a:/boot/grub

for file in $GRUB_FILES; do
    MTOOLSRC=./mtoolsrc mcopy $GRUB_DIR/$file a:/boot/grub
done

for module in $BIN_FILES; do
    MTOOLSRC=./mtoolsrc mcopy $module a:
done

rm mtoolsrc

echo '(fd0) fdimage.img' > bmap
echo 'root (fd0)\nsetup (fd0)\nquit' | /usr/sbin/grub --batch --device-map=bmap
rm bmap

