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
    // unsigned int age_set:1;
};

// VMA
struct vma_t
{
    unsigned int start_vpage: PAGE_TABLE_SIZE_LOG2;
    unsigned int end_vpage: PAGE_TABLE_SIZE_LOG2;
    unsigned int write_protected: 1;
    unsigned int file_mapped: 1;
};

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
    bool active;
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
    // if unmap when exit, then call this to put frame into free pool
    void put_frame_into_free_pool(unsigned _frame_i);
    void unmap_frame(unsigned _frame_i, unsigned* _evicted_frame_pid, unsigned* _evicted_frame_page_i);
    virtual void set_frame_config_on_map(unsigned _frame_i) = 0;
    void map_frame(unsigned _frame_i, unsigned _pid, unsigned _page_i);
    virtual unsigned select_victim_frame() = 0; 
    unsigned get_frame(); // select a free frame or a victim otherwise
    bool is_occupied_frame(unsigned _frame_i);
    void print_frame_table(); // for F option
};

class FIFOPager: public Pager
{
public:
    unsigned m_handle_frame_i;
    FIFOPager(unsigned _frame_num, unsigned _process_num);
    ~FIFOPager();
    unsigned select_victim_frame();
    void set_frame_config_on_map(unsigned _frame_i);
};

class RandomPager: public Pager
{
public:
    MyRand* m_myrand;
    RandomPager(unsigned _frame_num, unsigned _process_num, MyRand* _myrand);
    ~RandomPager();
    unsigned select_victim_frame();
    void set_frame_config_on_map(unsigned _frame_i);
};

class ClockPager: public Pager
{
public:
    unsigned m_handle_frame_i;
    ClockPager(unsigned _frame_num, unsigned _process_num);
    ~ClockPager();
    unsigned select_victim_frame();
    void set_frame_config_on_map(unsigned _frame_i);
};

// Algorithm "E", Enhanced Second Chance / NRU
class NRUPager: public Pager
{
public:
    unsigned m_handle_frame_i;
    int m_last_reset_instr_i;
    int m_candidate_frame_i_by_class[4]; // candidate for 4 classes, class id: 2*R + M 
    NRUPager(unsigned _frame_num, unsigned _process_num);
    ~NRUPager();
    unsigned select_victim_frame();
    void set_frame_config_on_map(unsigned _frame_i);
};

class AgingPager: public Pager
{
public:
    unsigned m_handle_frame_i;
    unsigned m_ages[MAX_FRAME_NUM];
    AgingPager(unsigned _frame_num, unsigned _process_num);
    ~AgingPager();
    void update_age(unsigned _frame_i);
    unsigned select_victim_frame();
    void set_frame_config_on_map(unsigned _frame_i);
};

class WorkingSetPager: public Pager
{
public:
    unsigned m_handle_frame_i;
    unsigned long long m_last_mapped_time_list[MAX_FRAME_NUM];
    WorkingSetPager(unsigned _frame_num, unsigned _process_num);
    ~WorkingSetPager();
    unsigned select_victim_frame();
    void set_frame_config_on_map(unsigned _frame_i);
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
    unsigned long long m_inst_count;
    unsigned long long m_ctx_switches;
    unsigned long long m_process_exits;
    unsigned long long m_readwrites;
public:
    Simulator(MyConfig* _myconfig, MyInput* _myinput, MyRand* _myrand);
    ~Simulator();
    void simulate();
    void print_summary();
};

#endif