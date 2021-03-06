#include "pmm.h"
#include "util.h"
#include "term.h"

static uint32_t * pmm_bitmap;
static uint32_t pmm_bitmap_size;
static uint32_t pmm_bitmap_mem_size;
static volatile uint32_t pmm_bottom_used;
static int pmm_inited = 0;

extern const void kernel_start;
extern const void kernel_end;

static void pmm_mark_used(void * page) 
{
	uint32_t bm_index = ((uintptr_t)page) / PMM_PAGE_SIZE / 32;
	uint32_t bm_bitindex = (((uintptr_t)page) / PMM_PAGE_SIZE) & 31;
	
	pmm_bitmap[bm_index] &= ~(1 << bm_bitindex);
}

static void pmm_mark_free(void * page) 
{
	uint32_t bm_index = ((uintptr_t)page) / PMM_PAGE_SIZE / 32;
	uint32_t bm_bitindex = (((uintptr_t)page) / PMM_PAGE_SIZE) & 31;
	
	pmm_bitmap[bm_index] |= (1 << bm_bitindex);
}

void pmm_init(struct multiboot_mbs_info * mbs_info) 
{
	uint32_t tmp;
	uint32_t phys_ram_size = 0;
	struct multiboot_mbs_mmap * mmap;// = mbs_info->mbs_mmap_addr;
	struct multiboot_mbs_mmap * mmap_end = (void *)((uintptr_t *)mbs_info->mbs_mmap_addr + mbs_info->mbs_mmap_length);

	uintptr_t addr, addr_end;
	uint32_t i;


	// find highest end address which is the amount of usable physical 
	// ram
	for (mmap = (struct multiboot_mbs_mmap *)mbs_info->mbs_mmap_addr; mmap < mmap_end; mmap++) {
		if (mmap->type == 1) {
			tmp = mmap->base + mmap->length;
			if (tmp > phys_ram_size)
				phys_ram_size = tmp;
		}
	}
	
	pmm_bitmap_mem_size = phys_ram_size / PMM_PAGE_SIZE / 8;
	pmm_bitmap_size = pmm_bitmap_mem_size / sizeof(uint32_t);
	vk_printf("pmm: found %d Bytes (%dMiB) physical memory.\n   pmm_bitmap size=%d Bytes\n", phys_ram_size, phys_ram_size / 1024 / 1024, pmm_bitmap_mem_size);
	
	
	// place pmm bitmap right after kernel (for now)
	pmm_bitmap = (uint32_t *)&kernel_end;

	for (addr = 0; addr < pmm_bitmap_size; addr++) 
		pmm_bitmap[addr] = 0;

	vk_printf("test: pmm bitmap addr: 0x%08x\n", pmm_bitmap);
	vk_printf("pmm: freeing memory ...\n");

	for (mmap = (struct multiboot_mbs_mmap *)mbs_info->mbs_mmap_addr; mmap < mmap_end; mmap++) {

		if (mmap->type == 1) {
			addr = (uintptr_t)mmap->base;
			addr_end = (uintptr_t)(addr + mmap->length);

			vk_printf("     freeing                : %08x - %08x\n", addr, addr_end);
			
			while (addr < addr_end) {
				pmm_mark_free((void *)addr);
				addr += PMM_PAGE_SIZE;
			}
		}
	}

	// mark pages used by bitmap itself
	vk_printf("pmm: preserving pmm bitmap  : %08x - %08x\n", pmm_bitmap, (char*)pmm_bitmap + pmm_bitmap_mem_size);
	for (addr = 0; addr < pmm_bitmap_mem_size; addr += PMM_PAGE_SIZE)
		pmm_mark_used((void *)((char*)pmm_bitmap) + addr);
	
	vk_printf("pmm: preserving kernel mem  : %08x - %08x\n", (unsigned int)&kernel_start, (unsigned int)&kernel_end);
	addr = (uintptr_t) &kernel_start;
	while (addr < (uintptr_t)&kernel_end) {
		pmm_mark_used((void *)addr);
		addr += PMM_PAGE_SIZE;
	}
	
	// preserve mutliboot structure and modules
	//term_puts("pmm: preserving multiboot structure : ");
	struct multiboot_module * mb_modules = mbs_info->mbs_mods_addr;
	pmm_mark_used(mbs_info);
	pmm_mark_used(mb_modules);
	vk_printf("pmm: preserving memory for multiboot modules...\n");
	for (i = 0; i < mbs_info->mbs_mods_count; i++) {
		addr = mb_modules[i].mod_start;
		while (addr < mb_modules[i].mod_end) {
			pmm_mark_used((void *)addr);
			addr += PMM_PAGE_SIZE;
		}

		vk_printf("     module %-16s: %08x - %08x\n", mb_modules[i].string, mb_modules[i].mod_start, mb_modules[i].mod_end);
	}
	
	pmm_bottom_used = 32;
	pmm_inited = 1;
}

int pmm_is_page_free(void * page)
{
	uint32_t bm_index = ((uintptr_t)page) / PMM_PAGE_SIZE / 32;
	uint32_t bm_bitindex = (((uintptr_t)page) / PMM_PAGE_SIZE) & 31;

	return (pmm_bitmap[bm_index] & (1 << bm_bitindex));
}


void * pmm_alloc_page() 
{
	uintptr_t i, j;
	uintptr_t page;
	
	for (i=pmm_bottom_used; i<pmm_bitmap_size; i++) {
		if (pmm_bitmap[i]) {
			for (j=0; j<32; j++) {
				if (pmm_bitmap[i] & (1<<j)) {
					pmm_bitmap[i] &= ~(1<<j);
					page = PMM_PAGE_SIZE * ((i << 5) + j);
					pmm_bottom_used = i;
					return (void *)page;
				}
			}
		}
	}
	return 0;
}

void * pmm_alloc_page_base(void * base) 
{
	uintptr_t i, j;
	uintptr_t page;
	uint32_t bm_index = ((uintptr_t)base) / PMM_PAGE_SIZE / 32;
	uint32_t bm_bitindex = (((uintptr_t)base) / PMM_PAGE_SIZE) & 31;
	
	
	
	if (base == 0)
		bm_index = pmm_bottom_used;
		
	for (i=bm_index; i<pmm_bitmap_size; i++) {
		if (pmm_bitmap[i]) {
			for (j=(i==0?bm_bitindex:0); j<32; j++) {
				if (pmm_bitmap[i] & (1<<j)) {
					pmm_bitmap[i] &= ~(1<<j);
					page = PMM_PAGE_SIZE * ((i << 5) + j);
					pmm_bottom_used = i;
					return (void *)page;
				}
			}
		}
	}
	return 0;
}

void pmm_free_page(void * page) 
{
	uint32_t bm_index = ((uintptr_t)page) / PMM_PAGE_SIZE / 32;
	uint32_t bm_bitindex = (((uintptr_t)page) / PMM_PAGE_SIZE) & 31;
	
	if (bm_index < pmm_bottom_used) {
		if (bm_index >= 32)
			pmm_bottom_used = bm_index;
	}
	
	pmm_bitmap[bm_index] |= (1 << bm_bitindex);
}

void pmm_show_bitmap(uint32_t start, uint32_t limit) 
{
	uint32_t i, tmp;
	int j, idx;
	
	const char charset[] = "fedcba9876543210";
		
	if (limit > pmm_bitmap_size || limit == 0)
		limit = pmm_bitmap_size;

	vk_printf("\nphysmap pages 0x%08x - 0x%08x\n", start * PMM_PAGE_SIZE * sizeof(uint32_t) * 8, (start + limit) * PMM_PAGE_SIZE * sizeof(uint32_t) * 8);

	for (i = start; i < limit; i++) {

		if ((i&7) == 0) {
			term_setcolor(7, 0);
			vk_printf("\n%08x: ", i * PMM_PAGE_SIZE * sizeof(uint32_t) * 8);
		}
		
		tmp = pmm_bitmap[i];
		for (j = 0; j < 8; j++) {
			idx = tmp & 0xf;
			switch (idx) {
			case 15:
				term_setcolor(10, 0);
				break;
			case 0: 
				term_setcolor(12, 0);
				break;
			default:
				term_setcolor(14, 0);
				break;
			}
			vk_printf("%c", charset[idx]);
			tmp >>= 4;
		}
			
		
	}
	term_setcolor(7, 0);
	vk_printf("\n");
}

