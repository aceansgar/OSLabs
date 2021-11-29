#ifndef _MANAGER_H
#define _MANAGER_H

#include "utils.h"

/* 
* Page Table Entry
* can only be total of 32-bit size and will check on this typedef struct { … } frame_t;
*/
struct pte_t
{
    unsigned int valid:1; // or "present"
    unsigned int referenced:1;
    unsigned int modified:1;
    unsigned int write_pretected:1;
    unsigned int pagedout:1; // or "swapped" 
    unsigned int frame_id:MAX_FRAME_NUM_LOG2; // frame id, in case the pte is present/valid
    unsigned int file_mapped:1;
    unsigned int in_vma:1; // whether this page is in a vma (1) or in a hole (0)
}; 

struct frame_t
{
    unsigned int pid:MAX_PROCESS_NUM_LOG2; // reverse mapping to process id
    unsigned int vpage_id:PAGE_TABLE_SIZE_LOG2; // reverse mapping to page id in page table
};

class Process
{
public:
    int pid;
    pte_t page_table[PAGE_TABLE_SIZE]; // a per process array of fixed size=64 of pte_t not pte_t pointers !
    unsigned long long unmaps;
    unsigned long long maps;
    unsigned long long ins;
    unsigned long long outs;
    unsigned long long fins;
    unsigned long long fouts;
    unsigned long long zeros;
    unsigned long long segvs;
    unsigned long long segprots;
    Process();
};

frame_t frame_table[MAX_FRAMES]; 

// page fault handler
class Pager 
{
    virtual int select_victim_frame() = 0; 
};

class Simulator
{
    std::deque<frame_t> free_frame_pool;
};

frame_t *get_frame() {
frame_t *frame = allocate_frame_from_free_list();
if (frame == NULL) frame = THE_PAGER->select_victim_frame();
return frame;
}
while (get_next_instruction(&operation, &vpage)) {
// handle special case of “c” and “e” instruction
// now the real instructions for read and write
pte_t *pte = &current_process->page_table[vpage];
if ( ! pte->present) {
// this in reality generates the page fault exception and now you execute // verify this is actually a valid page in a vma if not raise error and next inst
frame_t *newframe = get_frame();
//-> figure out if/what to do with old frame if it was mapped
// see general outline in MM-slides under Lab3 header and writeup below
// see whether and how to bring in the content of the access page.
} // check write protection
// simulate instruction execution by hardware by updating the R/M PTE bits
update_pte(read/modify) bits based on operations.
}

#endif