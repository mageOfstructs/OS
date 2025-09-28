#!/bin/bash

qemu_flags=("$@")
qemu_flags_after=("${qemu_flags[@]:1}")

com="qemu-system-i386 -cdrom "$1" -m 1G -chardev file,id=klog,path=./kernel.log -serial chardev:klog -drive file=./testdisk.img,format=raw,index=0 ${qemu_flags_after[@]}"

echo $com
echo "$($com)"
