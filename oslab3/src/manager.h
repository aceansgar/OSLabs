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
    unsigned int pagedout:1; // or "swapped", once set to 1 then cannot be changed
    unsigned int frame_id:MAX_FRAME_NUM_LOG2; // frame id, in case the pte is present/valid
    unsigned int file_mapped:1;
    // unsigned int in_vma:1; // whether this page is in a vma (1) or in a hole (0)
}; 

struct frame_t
{
    unsigned int pid:MAX_PROCESS_NUM_LOG2; // reverse mapping to process id
    unsigned int vpage_id:PAGE_TABLE_SIZE_LOG2; // reverse mapping to page id in page table
    unsigned int occupied:1;
};

// VMA
struct vma_t
{
    unsigned int start_vpage: PAGE_TABLE_SIZE_LOG2;
    unsigned int end_vpage: PAGE_TABLE_SIZE_LOG2;
    unsigned int write_protected: 1;
    unsigned int file_mapped: 1;
};

frame_t frame_table[MAX_FRAME_NUM]; // only pager can access frame table

class Process
{
public:
    unsigned m_pid;
    unsigned m_vma_num;
    vma_t m_vmas[MAX_VMA_NUM]; 
    pte_t m_page_table[PAGE_TABLE_SIZE]; // a per process array of fixed size=64 of pte_t not pte_t pointers !
    // for statistics print
    unsigned long long m_unmaps;
    unsigned long long m_maps;
    unsigned long long m_ins;
    unsigned long long m_outs;
    unsigned long long m_fins;
    unsigned long long m_fouts;
    unsigned long long m_zeros;
    unsigned long long m_segvs;
    unsigned long long m_segprots;
    Process(unsigned _pid, unsigned _vma_num);
    bool page_in_vmas(unsigned _page_i);
    // check if FOUT
    void check_unmap_on_exit_process(unsigned _page_i); 
    // unmap and check if OUT/FOUT
    void check_unmap_during_process(unsigned _page_i);
    // map and check if IN/FIN/ZERO
    void check_map_during_process(unsigned _page_i, unsigned _frame_i);
    void read_page(unsigned _page_i);
    void write_page(unsigned _page_i);
};

// page fault handler
class Pager 
{
public:
    unsigned m_frame_num;
    unsigned m_process_num;
    std::queue<unsigned> m_free_frame_id_pool;
    Pager(unsigned _frame_num, unsigned _process_num);
    ~Pager();
    void unmap_frame(unsigned _frame_i);
    void map_frame(unsigned _frame_i, unsigned _pid, unsigned _page_i);
    virtual unsigned select_victim_frame() = 0; 
    unsigned get_frame(); // select a free frame or a victim otherwise
    bool is_occupied_frame(unsigned _frame_i);
};

class FIFOPager: public Pager
{
public:
    unsigned m_handle_frame_i;
    FIFOPager(unsigned _frame_num, unsigned _process_num);
    ~FIFOPager();
    unsigned select_victim_frame();
};

class Simulator
{
private:
    MyConfig* m_myconfig;
    unsigned m_frame_num;
    unsigned m_process_num;
    MyInput* m_myinput;
    MyRand* m_myrand;
    Pager* m_pager;
    std::vector<Process*> m_processes;
    Process* m_curr_proc;
    unsigned long long m_inst_count;
    unsigned long long m_ctx_switches;
    unsigned long long m_process_exits;
public:
    Simulator(MyConfig* _myconfig, MyInput* _myinput, MyRand* _myrand);
    ~Simulator();
    void simulate();
    void print_summary();
};

#endif