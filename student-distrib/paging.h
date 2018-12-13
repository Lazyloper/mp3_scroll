#ifndef PAGE_H
#define PAGE_H

#define PAGING_SIZE 	1024
#define _4KB_SIZE 		4096
#define VIDEO_MEM_ADDR	0x000B8000
#define VIDEO_MEM_BUF	0x000BA000
#define VIDEO_MEM_LARGE 0xC00000
#define KERNAL_MEM_ADDR 0x00400000
#define MASK_TABLE		0x003FF000
#define _256MB      0x10000000
#define _4MB        0x400000
#define _160MB      0xA000000

// Page Directoty Entry data sturcter
typedef struct pde {
    union {
        uint32_t val;
        struct { //4-kb paging
            uint8_t present_k         : 1;
            uint8_t read_write_k      : 1;
            uint8_t user_supervisor_k : 1;
            uint8_t write_through_k   : 1;
            uint8_t cache_disabled_k  : 1;
            uint8_t accessed_k        : 1;
            uint8_t reserved_k        : 1;
            uint8_t page_size_k       : 1;
            uint8_t global_page_k     : 1;
            uint8_t avail_k           : 3;
            uint32_t page_tbl_base_addr_k : 20;
        } __attribute__ ((packed));
        struct { //4-mb paging
            uint8_t present_m         : 1;
            uint8_t read_write_m      : 1;
            uint8_t user_supervisor_m : 1;
            uint8_t write_through_m   : 1;
            uint8_t cache_disabled_m  : 1;
            uint8_t accessed_m        : 1;
            uint8_t dirty_m           : 1;
            uint8_t page_size_m       : 1;
            uint8_t global_page_m     : 1;
            uint8_t avail_m           : 3;
            uint8_t page_tbl_attr_idx : 1;
            uint16_t reserved_m       : 9;
            uint16_t page_base_addr_m      : 10;
        } __attribute__ ((packed));
    };
} pde_t;

// Page Table Entry data structure
typedef struct pte {
    union {
        uint32_t val;
        struct {
            uint8_t present         : 1;
            uint8_t read_write      : 1;
            uint8_t user_supervisor : 1;
            uint8_t write_through   : 1;
            uint8_t cache_disabled  : 1;
            uint8_t accessed        : 1;
            uint8_t dirty           : 1;
            uint8_t page_tbl_attr_idx : 1;
            uint8_t global_page     : 1;
            uint8_t avail           : 3;
            uint32_t page_base_addr : 20;
        } __attribute__ ((packed));
    };
} pte_t;

pde_t page_dir[PAGING_SIZE] __attribute__((aligned (4096)));
pte_t page_tbl[PAGING_SIZE] __attribute__((aligned (4096)));
pte_t userpage_tbl[PAGING_SIZE] __attribute__((aligned(4096)));
int32_t video_mem_physical[4];

extern void init_paging();

void set_process_pde(int32_t vir_addr, int32_t process_num);
int32_t scheduler_vidmem_remap(uint32_t terminal_num, int32_t vidmap_flag);
void flush_tlb();

#endif
