#include "term.h"
#include "gdt.h"
#include "pmm.h"
#include "multiboot.h"
#include "vmm.h"
#include "testtasks.h"
#include "interrupt.h"
#include "cpu_state.h"
#include "pci.h"
#include "util.h"
#include "version.h"

void kernel_main(struct multiboot_mbs_info * mbs_info) 
{
	vk_setup_io(term_putc, term_puts);
	term_init();
	
	term_setcolor(15, 0);

	vk_printf("initializing physical memory manager\n");
	pmm_init(mbs_info);
	
	vk_printf("initializing paging...\n");
	vmm_init();

	vk_printf("initializing GDT\n");
	gdt_init();
	
	pci_enum();

	interrupt_init();

	testtasks_init(mbs_info);

	term_setcolor(14, 0);
	vk_printf("welcome to vlOS version %s\n", VERSION);
	vk_printf("keys:\n1 - 6 : Toggle test tasks (multiboot elf modules)\n");
	vk_printf("7     : Show physical memory map\n");
	vk_printf("8     : process list / stats\n");
	term_setcolor(7, 0);
	
	interrupt_enable();
	
	
	for(;;);
}
