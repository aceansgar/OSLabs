#include "manager.h"

Process::Process(unsigned _pid, unsigned _vma_num)
{
    m_pid = _pid;
    m_vma_num = _vma_num;
    memset(m_vmas, 0, _vma_num * sizeof(vma_t));
    memset(m_page_table, 0, PAGE_TABLE_SIZE * sizeof(pte_t));
    m_unmaps = 0;
    m_maps = 0;
    m_ins = 0;
    m_outs = 0;
    m_fins = 0;
    m_fouts = 0;
    m_zeros = 0;
    m_segvs = 0;
    m_segprots = 0;
}

bool Process::page_in_vmas(unsigned _page_i)
{
    for(unsigned vma_i = 0; vma_i < m_vma_num; ++ vma_i)
    {
        if(_page_i >= m_vmas[vma_i].start_vpage && _page_i <= m_vmas[vma_i].end_vpage)
        {
            m_page_table[_page_i].write_pretected = m_vmas[vma_i].write_protected;
            m_page_table[_page_i].file_mapped = m_vmas[vma_i].file_mapped;
            return true;
        }
    }
    ++ m_segvs;
    printf(" SEGV\n");
    return false;
}

void Process::check_unmap_on_exit_process(unsigned _page_i)
{
    ++ m_unmaps;
    if(m_page_table[_page_i].file_mapped && m_page_table[_page_i].modified)
    {
        printf(" FOUT\n");
        ++ m_fouts;
    }
}

void Process::check_unmap_during_process(unsigned _page_i)
{
    ++ m_unmaps;
    m_page_table[_page_i].valid = 0;
    if(m_page_table[_page_i].modified)
    {
        if(m_page_table[_page_i].file_mapped)
        {
            printf(" FOUT\n");
            ++ m_fouts;
        }
        else
        {
            printf(" OUT");
            ++ m_outs;
            m_page_table[_page_i].pagedout = 1;
        }
    }
}

void Process::check_map_during_process(unsigned _page_i, unsigned _frame_i)
{
    ++ m_maps;
    m_page_table[_page_i].valid = 1;
    m_page_table[_page_i].frame_id = _frame_i;
    // reset bits
    m_page_table[_page_i].referenced = 0;
    m_page_table[_page_i].modified = 0;
    if(m_page_table[_page_i].file_mapped)
    {
        printf(" FIN\n");
        ++ m_fins;
    }
    else if(m_page_table[_page_i].pagedout)
    {
        printf(" IN\n");
        ++ m_ins;
    }
    else
    {
        printf(" ZERO\n");
        ++ m_zeros;
    }
}

void Process::read_page(unsigned _page_i)
{
    m_page_table[_page_i].referenced = 1;
}

void Process::write_page(unsigned _page_i)
{
    if(m_page_table[_page_i].write_pretected)
    {
        m_page_table[_page_i].referenced = 1;
        ++ m_segprots;
        printf(" SEGPROT\n");
    }
    else
    {
        m_page_table[_page_i].referenced = 1;
        m_page_table[_page_i].modified = 1;
    }
}

Pager::Pager(unsigned _frame_num, unsigned _process_num)
{
    m_frame_num = _frame_num;
    m_process_num = _process_num;
    for(int i = 0; i < _frame_num; ++ i)
        m_free_frame_id_pool.push(i);
    memset(frame_table, 0, _frame_num * sizeof(frame_t));
}

Pager::~Pager()
{

}

void Pager::unmap_frame(unsigned _frame_i)
{
    unsigned pid = frame_table[_frame_i].pid;
    unsigned page_i = frame_table[_frame_i].vpage_id;
    printf(" UNMAP %u:%u\n", pid, page_i);
    frame_table[_frame_i].occupied = 0;
    m_free_frame_id_pool.push(_frame_i);
}

void Pager::map_frame(unsigned _frame_i, unsigned _pid, unsigned _page_i)
{
    printf(" MAP %u\n", _frame_i);
    frame_table[_frame_i].occupied = 1;
    // set reverse mapping to page table
    frame_table[_frame_i].pid = _pid;
    frame_table[_frame_i].vpage_id = _page_i;
}

unsigned Pager::get_frame()
{
    if(!m_free_frame_id_pool.empty())
    {
        unsigned frame_i = m_free_frame_id_pool.front();
        m_free_frame_id_pool.pop();
        return frame_i;
    }
    // all frames are occupied
    return select_victim_frame();
}

bool Pager::is_occupied_frame(unsigned _frame_i)
{
    return frame_table[_frame_i].occupied;
}

FIFOPager::FIFOPager(unsigned _frame_num, unsigned _process_num): Pager(_frame_num, _process_num)
{
    m_handle_frame_i = 0;
}

FIFOPager::~FIFOPager()
{

}

unsigned FIFOPager::select_victim_frame()
{
    unsigned frame_i = m_handle_frame_i;
    m_handle_frame_i = (m_handle_frame_i + 1) % m_frame_num;
    return frame_i;
}

/*
* set configs, input object and rand object. 
* create correspongding pager object
* read infile and create processes
*/
Simulator::Simulator(MyConfig* _myconfig, MyInput* _myinput, MyRand* _myrand)
{
    m_myconfig = _myconfig;
    m_frame_num = _myconfig -> m_frame_num;
    m_myinput = _myinput;
    m_myrand = _myrand;

    // create pager object
    switch(_myconfig->m_policy)
    {
        case FIFO:
            m_pager = new FIFOPager(m_frame_num, m_process_num);
            break;
        case RANDOM:
            break;
        case CLOCK:
            break;
        case NRU:
            break;
        case AGING:
            break;
        case WORKINGSET:
            break;
        default:
            fprintf(stderr, "no Pager class for this policy \n");
            exit(0);
            break;
    }

    // read infile, create processes
    m_process_num = _myinput -> read_unsigned();
    for(unsigned pid = 0; pid < m_process_num; ++ pid)
    {
        int vma_num = m_myinput -> read_unsigned();
        Process* proc = new Process(pid, vma_num);
        proc -> m_vma_num = vma_num;
        for(unsigned vma_i = 0; vma_i < vma_num; ++ vma_i)
        {
            proc -> m_vmas[vma_i].start_vpage = m_myinput -> read_unsigned();
            proc -> m_vmas[vma_i].end_vpage = m_myinput -> read_unsigned();
            proc -> m_vmas[vma_i].write_protected = m_myinput -> read_unsigned();
            proc -> m_vmas[vma_i].file_mapped = m_myinput -> read_unsigned();
        }
        m_processes.push_back(proc);
    }

    m_curr_proc = NULL;

    m_inst_count = 0;
    m_ctx_switches = 0;
    m_process_exits = 0;
}

Simulator::~Simulator()
{
    delete m_pager;
}

void Simulator::simulate()
{
    char operation;
    unsigned val;
    unsigned long long instr_i = 0;
    while (m_myinput -> get_next_instruction(&operation, &val)) 
    {
        printf("%lu: ==> %c %u\n", instr_i, operation, val);
        switch(operation)
        {
            case 'c':
                ++ m_ctx_switches;
                m_curr_proc = m_processes[val];
                break;
            case 'e':
                ++ m_process_exits;
                printf("EXIT current process %u\n", m_curr_proc -> m_pid);
                for(unsigned page_id = 0; page_id < PAGE_TABLE_SIZE, ++ page_id)
                {
                    if(m_curr_proc -> m_page_table[page_id].valid)
                    {
                        // unmap
                        unsigned frame_i = m_curr_proc -> m_page_table[page_id].frame_id;
                        m_pager -> unmap_frame(frame_i);
                        m_curr_proc -> check_unmap_on_exit_process(page_id);
                    }
                }
                break;
            case 'r': case 'w':
                unsigned page_i = val;
                if(!m_curr_proc -> m_page_table[page_i].valid)
                {
                    // page fault, check if page is in a vma first
                    if(m_curr_proc -> page_in_vmas(page_i))
                    {
                        unsigned frame_i = m_pager -> get_frame(); // free frame or occupied frame
                        if(m_pager -> is_occupied_frame(frame_i))
                        {
                            // unmap, hardware has no access to frame table
                            m_pager -> unmap_frame(frame_i);
                            m_curr_proc -> check_unmap_during_process(page_i);
                        }
                        m_curr_proc -> check_map_during_process(page_i, frame_i);
                        m_pager -> map_frame(frame_i, m_curr_proc -> m_pid, page_i);
                    }
                }
                // now the PTE has a corresponding frame prepared
                if(operation == 'r')
                {
                    m_curr_proc -> read_page(page_i);
                }
                else
                {
                    // write
                    m_curr_proc -> write_page(page_i);
                }
                break;
            default:
                fprintf(stderr, "wrong operation\n");
                break;
        }
        ++ instr_i;
    }
}

void Simulator::print_summary()
{
    unsigned long long cost = 0;
    if(m_myconfig->m_page_table_option)
    {
        for(int pid = 0; pid < m_process_num; ++ pid)
        {
            Process* proc = m_processes[pid];
            printf("PROC[%d]: U=%lu M=%lu I=%lu O=%lu FI=%lu FO=%lu Z=%lu SV=%lu SP=%lu\n", \
            proc->m_pid, \
            proc->m_unmaps, proc->m_maps, proc->m_ins, proc->m_outs, \
            proc->m_fins, proc->m_fouts, proc->m_zeros, \
            proc->m_segvs, proc->m_segprots);
            cost += (proc -> m_unmaps * UNMAP_COST + proc -> m_maps * MAP_COST \
            + proc -> m_ins * IN_COST + proc -> m_outs * OUT_COST \
            + proc -> m_fins * FIN_COST + proc -> m_fouts * FOUT_COST + proc -> m_zeros * ZERO_COST \
            + proc -> m_segvs * SEGV_COST + proc -> m_segprots * SEGPROT_COST);
        }
    }
    cost += (m_ctx_switches * CONTEXT_SWITCH_COST + m_process_exits * PROCESS_EXIT_COST);
    printf("TOTALCOST %lu %lu %lu %llu %lu\n", \
    m_inst_count, m_ctx_switches, m_process_exits, cost, sizeof(pte_t));
}