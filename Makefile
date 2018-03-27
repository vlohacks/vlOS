KERNEL_DIR		= kernel
USERLAND_DIR	= userland

.PHONY: kernel
.PHONY: userland


kernel: 
	$(MAKE) -C $(KERNEL_DIR)
	cp $(KERNEL_DIR)/kernel.bin .
	
userland:
	$(MAKE) -C $(USERLAND_DIR)

iso: kernel userland
	./setup_iso.sh	

run: kernel userland iso
	qemu-system-i386 -D ./qemu.log -d cpu_reset -m 8M -soundhw ac97 -cdrom image.iso
	#qemu-system-i386 -kernel kernel.bin

clean:
	$(MAKE) -C $(KERNEL_DIR) clean
	$(MAKE) -C $(USERLAND_DIR) clean
	rm -f kernel.bin
	rm -fr iso
	rm -f image.iso
