#!/bin/sh
set -e
. ./build.sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp sysroot/boot/redos.kernel isodir/boot/redos.kernel
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "redos" {
	multiboot /boot/redos.kernel
}
EOF
grub-mkrescue -o redos.iso isodir
