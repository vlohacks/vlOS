#!/bin/bash
GRUBCFG="iso/boot/grub/grub.cfg"
USERLAND_DIR="userland"
mkdir -p iso/boot/grub
echo 'menuentry "vlOS" {' > $GRUBCFG
echo '	multiboot /boot/kernel.bin' >> $GRUBCFG
cp kernel.bin iso/boot/
for I in $(ls $USERLAND_DIR/*.elf); do 
	FN=$(basename $I)
	cp $I iso/boot/$FN
	echo "	module /boot/$FN $FN" >> $GRUBCFG 
done
echo '}' >> $GRUBCFG
grub-mkrescue -d /usr/lib/grub/i386-pc/ -o image.iso iso/
