#include "x86_desc.h"
#include "lib.h"
#include "paging.h"
#include "terminal.h"

#define PAGING_SIZE 	1024
#define _4KB_SIZE 		4096
#define VIDEO_MEM_ADDR	0x000B8000
#define VIDEO_MEM_BUF		0x000BA000
#define _1MB						0x100000
#define KERNAL_MEM_ADDR 0x00400000
#define MASK_TABLE		0x003FF000
pde_t page_dir[PAGING_SIZE] __attribute__((aligned (4096)));
pte_t page_tbl[PAGING_SIZE] __attribute__((aligned (4096)));
pte_t userpage_tbl[PAGING_SIZE] __attribute__((aligned(4096)));

int32_t video_mem_physical[4] = {VIDEO_MEM_ADDR + 1 * _4KB_SIZE,
																 VIDEO_MEM_ADDR + 2 * _4KB_SIZE,
																 VIDEO_MEM_ADDR + 3 * _4KB_SIZE,
															 	 VIDEO_MEM_ADDR};
																 
// int32_t video_mem_physical[4] = {_1MB + 1 * _4KB_SIZE,
//  																 _1MB + 2 * _4KB_SIZE,
//  																 _1MB + 3 * _4KB_SIZE,
//  															 	 VIDEO_MEM_ADDR};


/*
 * Initialize paging. For this check point, one Page Directory and
 * one Page Table is needed.
 */
void init_paging(){
		uint32_t i;
		// Initialize all the PTE and PDE to not present
		for (i = 0; i < PAGING_SIZE; i++) {
				page_dir[i].val = 0;
				page_tbl[i].val = 0;
		}

		// second PDE in the page directory --> 4MB to 8MB memory
		// Page size is 4MB
		page_dir[1].page_base_addr_m = KERNAL_MEM_ADDR >> 22;
		page_dir[1].present_m = 1;
		page_dir[1].read_write_m = 1;
		page_dir[1].page_size_m = 1;

		// first PDE in the page directory --> 0MB to 4MB memory
		// 4MB is divided into the page table
		page_dir[0].page_tbl_base_addr_k = ((uint32_t) page_tbl >> 12);
		page_dir[0].present_k = 1;
		page_dir[0].read_write_k = 1;
		page_dir[0].page_size_k = 0;
		page_dir[0].user_supervisor_k = 1;
		//PTE for video memory
		page_tbl[(VIDEO_MEM_ADDR & MASK_TABLE) >> 12].page_base_addr = VIDEO_MEM_ADDR >> 12;
		page_tbl[(VIDEO_MEM_ADDR & MASK_TABLE) >> 12].present = 1;
		page_tbl[(VIDEO_MEM_ADDR & MASK_TABLE) >> 12].read_write = 1;
		page_tbl[(VIDEO_MEM_ADDR & MASK_TABLE) >> 12].user_supervisor = 1;

page_dir[_160MB/_4MB].page_base_addr_m = VIDEO_MEM_LARGE >> 22;
page_dir[_160MB/_4MB].present_m	= 1;
page_dir[_160MB/_4MB].read_write_m = 1;
page_dir[_160MB/_4MB].page_size_m = 1;


		scheduler_vidmem_remap(1, 0);
		scheduler_vidmem_remap(0, 0);
		scheduler_vidmem_remap(2, 0);

		/* assembly adapted from Osdev */
		asm volatile(
			 	"movl %0, %%eax;"
				"movl %%eax, %%cr3;"
				"movl %%cr4, %%eax;"
				"orl $0x010, %%eax;"
				"movl %%eax, %%cr4;"
				"movl %%cr0, %%eax;"
				"orl $0x80000000, %%eax;"
				"movl %%eax, %%cr0;"
				:
				: "r" (page_dir)
				: "eax"
		);
}

/*   set_process_pde
 *   Inputs: vir_addr: virtual address of start of the program
 *			 process_num: process number
 *   Return Value: None
 *   Function: Set proper page for user program and flush TLB
 */
void set_process_pde(int32_t vir_addr, int32_t process_num){
		page_dir[vir_addr >> 22].val = 0;
		page_dir[vir_addr >> 22].page_base_addr_m = (KERNAL_MEM_ADDR >> 22) + process_num;
		page_dir[vir_addr >> 22].present_m = 1;
		page_dir[vir_addr >> 22].read_write_m = 1;
		page_dir[vir_addr >> 22].page_size_m = 1;
		page_dir[vir_addr >> 22].user_supervisor_m = 1;
		//flush TLB
		flush_tlb();
		return;
}

/*   scheduler_vidmem_remap
 *   Inputs: terminal_num: the next active terminal index
 *			 vidmap_flag: indicates if the process on the terminal use vidmap
 *   Return Value: 0 when success
 *   Function: Map video mem the process on next active terminal to its physical region.
 */
int32_t scheduler_vidmem_remap(uint32_t terminal_num, int32_t vidmap_flag) {
		/* check the validity of input */
		if (terminal_num < 0 || terminal_num > 2)
				return -1;

		int32_t video_physical, video_vir;

		/* calculate the target physical mem region according to the terminal num */
		if (terminal_num == current_terminal_id) {
				video_physical = video_mem_physical[3];
				video_vir = video_mem_physical[terminal_num];
		} else {
				video_physical = video_mem_physical[terminal_num];
				video_vir = video_physical;
		}
		//PTE for video memory
		page_tbl[(video_vir & MASK_TABLE) >> 12].page_base_addr = (video_physical) >> 12;
		page_tbl[(video_vir & MASK_TABLE) >> 12].present = 1;
		page_tbl[(video_vir & MASK_TABLE) >> 12].read_write = 1;
		page_tbl[(video_vir & MASK_TABLE) >> 12].user_supervisor = 1;

		/* check if the target terminal is using vid map for userprogram */
		if (vidmap_flag) {
			//PTE for video memory
			userpage_tbl[0].page_base_addr = (video_physical) >> 12;
			userpage_tbl[0].present = 1;
			userpage_tbl[0].read_write = 1;
			userpage_tbl[0].user_supervisor = 1;
		}

		//flush TLB
		flush_tlb();
		// if (terminal_num == 1){
		// 	*(uint8_t *)(video_physical) = 0x7e;
		// 	*(uint8_t *)(video_physical + 1) = 0x07;
		// } else if (terminal_num == 2) {
		// 	*(uint8_t *)(video_physical) = 0x40;
		// 	*(uint8_t *)(video_physical + 1) = 0x07;
		// }
		return 0;
}

/*   flush_tlb
 *   Inputs: none
 *   Return Value: none
 *   Function: flush TLB.
 */
void flush_tlb() {
	asm volatile (
		"mov %%cr3, %%eax;"
		"mov %%eax, %%cr3;"
		:                      /* no outputs */
		:                      /* no inputs */
		:"%eax"                /* clobbered register */
    );
    return;
}
