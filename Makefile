KERNEL_DIR		= kernel
TESTTASK_DIR	= testtask

.PHONY: kernel
.PHONY: testtask


kernel: 
	$(MAKE) -C $(KERNEL_DIR)
	cp $(KERNEL_DIR)/kernel.bin .
	
testtask:
	$(MAKE) -C $(TESTTASK_DIR)

iso: kernel testtask
	./setup_iso.sh	

run: kernel testtask iso
	qemu-system-i386 -m 8M -soundhw ac97 -cdrom image.iso
	#qemu-system-i386 -kernel kernel.bin

clean:
	$(MAKE) -C $(KERNEL_DIR) clean
	$(MAKE) -C $(TESTTASK_DIR) clean
	rm -f kernel.bin
	rm -fr iso
	rm -f image.iso
