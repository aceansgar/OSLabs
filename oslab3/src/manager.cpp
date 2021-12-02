#include "manager.h"

frame_t frame_table[MAX_FRAME_NUM]; // only pager can access frame table
bool output_option = false;
Process* curr_proc;
std::vector<Process*> processes;
unsigned long long instr_i;

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
    active = true;
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
    if(output_option)
        printf(" SEGV\n");
    return false;
}

void Process::check_unmap_on_exit_process(unsigned _page_i)
{
    ++ m_unmaps;
    if(m_page_table[_page_i].file_mapped && m_page_table[_page_i].modified)
    {
        if(output_option)
            printf(" FOUT\n");
        ++ m_fouts;
    }
}

void Process::check_unmap_during_process(unsigned _page_i)
{
    ++ m_unmaps;
    m_page_table[_page_i].valid = 0;
    // printf("unmap page: modified bit: %u\n", m_page_table[_page_i].modified);
    if(m_page_table[_page_i].modified)
    {
        // printf("modified page unmap\n");
        if(m_page_table[_page_i].file_mapped)
        {
            if(output_option)
                printf(" FOUT\n");
            ++ m_fouts;
        }
        else
        {
            if(output_option)
                printf(" OUT\n");
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
        if(output_option)
            printf(" FIN\n");
        ++ m_fins;
    }
    else if(m_page_table[_page_i].pagedout)
    {
        if(output_option)
            printf(" IN\n");
        ++ m_ins;
    }
    else
    {
        if(output_option)
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
        if(output_option)
            printf(" SEGPROT\n");
    }
    else
    {
        // printf("write page _page_i: %u\n", _page_i);
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

void Pager::put_frame_into_free_pool(unsigned _frame_i)
{
    m_free_frame_id_pool.push(_frame_i);
}

void Pager::unmap_frame(unsigned _frame_i, unsigned* _evicted_frame_pid, unsigned* _evicted_frame_page_i)
{
    unsigned pid = frame_table[_frame_i].pid;
    unsigned page_i = frame_table[_frame_i].vpage_id;
    if(output_option)
        printf(" UNMAP %u:%u\n", pid, page_i);
    frame_table[_frame_i].occupied = 0;
    *_evicted_frame_pid = pid;
    *_evicted_frame_page_i = page_i;
    // m_free_frame_id_pool.push(_frame_i);
}

void Pager::map_frame(unsigned _frame_i, unsigned _pid, unsigned _page_i)
{
    if(output_option)
        printf(" MAP %u\n", _frame_i);
    frame_table[_frame_i].occupied = 1;
    // set reverse mapping to page table
    frame_table[_frame_i].pid = _pid;
    frame_table[_frame_i].vpage_id = _page_i;
    set_frame_config_on_map(_frame_i);
}

unsigned Pager::get_frame()
{
    // printf("pager get frame begin:\n");
    if(!m_free_frame_id_pool.empty())
    {
        unsigned frame_i = m_free_frame_id_pool.front();
        // printf("take frame %d\n", frame_i);
        m_free_frame_id_pool.pop();
        return frame_i;
    }
    // printf("no free frame, select a victim\n");
    // all frames are occupied
    return select_victim_frame();
}

bool Pager::is_occupied_frame(unsigned _frame_i)
{
    return frame_table[_frame_i].occupied;
}

void Pager::print_frame_table()
{
    printf("FT:");
    for(int frame_i = 0; frame_i < m_frame_num; ++ frame_i)
    {
        if(frame_table[frame_i].occupied)
            printf(" %u:%u", frame_table[frame_i].pid, frame_table[frame_i].vpage_id);
        else
            printf(" *");
    }
    printf("\n");
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
    // printf("victim frame selected: %u\n", frame_i);
    return frame_i;
}

void FIFOPager::set_frame_config_on_map(unsigned _frame_i)
{

}

RandomPager::RandomPager(unsigned _frame_num, unsigned _process_num, MyRand* _myrand): Pager(_frame_num, _process_num)
{
    m_myrand = _myrand;
}

RandomPager::~RandomPager()
{

}

unsigned RandomPager::select_victim_frame()
{
    return m_myrand->myrandom();
}

void RandomPager::set_frame_config_on_map(unsigned _frame_i)
{
    
}

ClockPager::ClockPager(unsigned _frame_num, unsigned _process_num): Pager(_frame_num, _process_num)
{
    m_handle_frame_i = 0;
}

ClockPager::~ClockPager()
{

}

unsigned ClockPager::select_victim_frame()
{
    unsigned frame_i = m_handle_frame_i;
    while(true)
    {
        unsigned pid = frame_table[frame_i].pid;
        unsigned page_i = frame_table[frame_i].vpage_id;
        if(processes[pid]->m_page_table[page_i].referenced)
        {
            // second chance
            processes[pid]->m_page_table[page_i].referenced = 0;
            frame_i = (frame_i + 1) % m_frame_num;
        }
        else
            break;
    }
    m_handle_frame_i = (frame_i + 1) % m_frame_num;
    // printf("victim frame selected: %u\n", frame_i);
    return frame_i;
}

void ClockPager::set_frame_config_on_map(unsigned _frame_i)
{
    
}

NRUPager::NRUPager(unsigned _frame_num, unsigned _process_num): Pager(_frame_num, _process_num)
{
    m_handle_frame_i = 0;
    m_last_reset_instr_i = -1;
}

NRUPager::~NRUPager()
{

}

unsigned NRUPager::select_victim_frame()
{
    // reset candidates of 4 classes to -1
    for(unsigned i = 0; i < 4; ++ i)
    {
        m_candidate_frame_i_by_class[i] = -1;
    }
    bool reset = false;
    if((int)instr_i - m_last_reset_instr_i >= 50)
    {
        reset = true;
        m_last_reset_instr_i = instr_i;
    }
    int selected_frame_i = -1;
    for(unsigned i = 0; i < m_frame_num; ++ i)
    {
        unsigned pid = frame_table[m_handle_frame_i].pid;
        unsigned page_i = frame_table[m_handle_frame_i].vpage_id;
        Process* proc = processes[pid];
        unsigned class_i = proc->m_page_table[page_i].referenced * 2 + proc->m_page_table[page_i].modified;
        if(class_i == 0)
        {
            if(selected_frame_i == -1)
            {
                // select this
                selected_frame_i = m_handle_frame_i;
                m_candidate_frame_i_by_class[0] = m_handle_frame_i;
            }
        }
        else
        {
            if(m_candidate_frame_i_by_class[class_i] == -1)
                m_candidate_frame_i_by_class[class_i] = m_handle_frame_i;
        }
        if(reset)
        {
            // printf("===== reset! \n");
            proc->m_page_table[page_i].referenced = 0;
        }
        if(selected_frame_i != -1 && !reset)
            break;
        m_handle_frame_i = (m_handle_frame_i + 1) % m_frame_num;
    }
    for(unsigned class_i = 0; class_i < 4; ++ class_i)
    {
        int cand_frame_i = m_candidate_frame_i_by_class[class_i];
        if(cand_frame_i != -1)
        {
            // select this
            m_handle_frame_i = (cand_frame_i + 1) % m_frame_num;
            return cand_frame_i;
        }
    }
    fprintf(stderr, "error: must exist a candidate\n");
    return -1;
}

void NRUPager::set_frame_config_on_map(unsigned _frame_i)
{
    
}

AgingPager::AgingPager(unsigned _frame_num, unsigned _process_num): Pager(_frame_num, _process_num)
{
    m_handle_frame_i = 0;
    memset(m_ages, 0, m_frame_num * sizeof(unsigned));
}

AgingPager::~AgingPager()
{

}

void AgingPager::update_age(unsigned _frame_i)
{
    unsigned pid = frame_table[_frame_i].pid;
    unsigned page_i = frame_table[_frame_i].vpage_id;
    unsigned referenced = processes[pid]->m_page_table[page_i].referenced;
    // printf("referenced: %u\n", referenced);
    processes[pid]->m_page_table[page_i].referenced = 0;
    // printf("====== frame %u prev age: %x, referenced: %u\n", _frame_i, m_ages[_frame_i], referenced);
    unsigned new_age = (m_ages[_frame_i] >> 1) + (referenced << 31);
    m_ages[_frame_i] = new_age;
    // if(instr_i == 83)
    // printf("====== frame %u new age: %x\n", _frame_i, new_age);
}

unsigned AgingPager::select_victim_frame()
{
    unsigned frame_cnt = 0;
    unsigned min_age = ~(unsigned)0;
    unsigned selected_frame_i = m_handle_frame_i;
    while(true)
    {
        update_age(m_handle_frame_i);
        if(m_ages[m_handle_frame_i] < min_age)
        {
            min_age = m_ages[m_handle_frame_i];
            selected_frame_i = m_handle_frame_i;
        }
        m_handle_frame_i = (m_handle_frame_i + 1) % m_frame_num;
        ++ frame_cnt;
        if(frame_cnt == m_frame_num)
            break;
    }
    m_handle_frame_i = (selected_frame_i + 1) % m_frame_num;
    return selected_frame_i;
}

void AgingPager::set_frame_config_on_map(unsigned _frame_i)
{
    m_ages[_frame_i] = 0;
}

WorkingSetPager::WorkingSetPager(unsigned _frame_num, unsigned _process_num): Pager(_frame_num, _process_num)
{
    m_handle_frame_i = 0;
    memset(m_last_mapped_time_list, 0, m_frame_num * sizeof(unsigned long long));   
}

WorkingSetPager::~WorkingSetPager()
{

}

unsigned WorkingSetPager::select_victim_frame()
{
    unsigned long long oldest_last_used_time = instr_i;
    unsigned oldest_frame_i = m_handle_frame_i;
    bool victim_found = false;
    unsigned victim_frame_i;
    for(unsigned i = 0; i < m_frame_num; ++ i)
    {
        unsigned pid = frame_table[m_handle_frame_i].pid;
        unsigned page_i = frame_table[m_handle_frame_i].vpage_id;
        unsigned referenced = processes[pid]->m_page_table[page_i].referenced;
        if(referenced)
        {
            m_last_mapped_time_list[m_handle_frame_i] = instr_i;
            processes[pid]->m_page_table[page_i].referenced = 0;
        }
        else if(instr_i - m_last_mapped_time_list[m_handle_frame_i] > 49)
        {
            victim_found = true;
            victim_frame_i = m_handle_frame_i;
            break;
        }
        if(m_last_mapped_time_list[m_handle_frame_i] < oldest_last_used_time)
        {
            oldest_last_used_time = m_last_mapped_time_list[m_handle_frame_i];
            oldest_frame_i = m_handle_frame_i;
        }
        m_handle_frame_i = (m_handle_frame_i + 1) % m_frame_num;
    }
    if(!victim_found)
        victim_frame_i = oldest_frame_i;
    m_handle_frame_i = (victim_frame_i + 1) % m_frame_num;
    return victim_frame_i;
}

void WorkingSetPager::set_frame_config_on_map(unsigned _frame_i)
{
    m_last_mapped_time_list[_frame_i] = instr_i;
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
    // set global output option
    output_option = _myconfig -> m_output_option;

    // create pager object
    switch(_myconfig->m_policy)
    {
        case FIFO:
            m_pager = new FIFOPager(m_frame_num, m_process_num);
            break;
        case RANDOM:
            m_pager = new RandomPager(m_frame_num, m_process_num, m_myrand);
            break;
        case CLOCK:
            m_pager = new ClockPager(m_frame_num, m_process_num);
            break;
        case NRU:
            m_pager = new NRUPager(m_frame_num, m_process_num);
            break;
        case AGING:
            m_pager = new AgingPager(m_frame_num, m_process_num);
            break;
        case WORKINGSET:
            m_pager = new WorkingSetPager(m_frame_num, m_process_num);
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
        processes.push_back(proc);
    }

    curr_proc = NULL;

    m_inst_count = 0;
    m_ctx_switches = 0;
    m_process_exits = 0;
    m_readwrites = 0;
}

Simulator::~Simulator()
{
    delete m_pager;
}

void Simulator::simulate()
{
    char operation;
    unsigned val;
    instr_i = 0;
    while (m_myinput -> get_next_instruction(&operation, &val)) 
    {
        if(output_option)
            printf("%llu: ==> %c %u\n", instr_i, operation, val);
        switch(operation)
        {
            case 'c':
            {
                ++ m_ctx_switches;
                curr_proc = processes[val];
                break;
            }
            case 'e':
            {
                ++ m_process_exits;
                curr_proc -> active = false;
                if(output_option)
                    printf("EXIT current process %u\n", curr_proc -> m_pid);
                for(unsigned page_id = 0; page_id < PAGE_TABLE_SIZE; ++ page_id)
                {
                    if(curr_proc -> m_page_table[page_id].valid)
                    {
                        // unmap
                        unsigned frame_i = curr_proc -> m_page_table[page_id].frame_id;
                        unsigned evicted_frame_pid, evicted_frame_page_i;
                        m_pager -> unmap_frame(frame_i, &evicted_frame_pid, &evicted_frame_page_i);
                        m_pager -> put_frame_into_free_pool(frame_i);
                        curr_proc -> check_unmap_on_exit_process(page_id); 
                    }
                }
                break;
            }
            case 'r': case 'w':
            {
                ++ m_readwrites;
                // printf("read/write instruction: \n");
                unsigned page_i = val;
                if(!curr_proc -> m_page_table[page_i].valid)
                {
                    // printf("page fault\n");
                    // page fault, check if page is in a vma first
                    if(curr_proc -> page_in_vmas(page_i))
                    {
                        // printf("the page is in a VMA\n");
                        unsigned frame_i = m_pager -> get_frame(); // free frame or occupied frame
                        // printf("get frame from pager done, frame %u\n", frame_i);
                        if(m_pager -> is_occupied_frame(frame_i))
                        {
                            // unmap, hardware has no access to frame table
                            unsigned evicted_frame_pid, evicted_frame_page_i;
                            m_pager -> unmap_frame(frame_i, &evicted_frame_pid, &evicted_frame_page_i);
                            processes[evicted_frame_pid] -> check_unmap_during_process(evicted_frame_page_i);
                        }
                        curr_proc -> check_map_during_process(page_i, frame_i);
                        m_pager -> map_frame(frame_i, curr_proc -> m_pid, page_i);
                    }
                }
                // printf("PTE prepared\n");
                // now the PTE has a corresponding frame prepared
                if(operation == 'r')
                {
                    curr_proc -> read_page(page_i);
                }
                else
                {
                    // write
                    curr_proc -> write_page(page_i);
                }
                break;
            }
            default:
                fprintf(stderr, "wrong operation\n");
                break;
        }
        // printf("referenced of page 3: %u\n", processes[0]->m_page_table[3].referenced);
        ++ instr_i;
    }
    m_inst_count = instr_i;
}

void Simulator::print_summary()
{
    // P option
    if(m_myconfig -> m_page_table_option)
    {
        for(int pid = 0; pid < m_process_num; ++ pid)
        {
            printf("PT[%u]:", pid);
            Process* proc = processes[pid];
            if(!proc -> active)
            {
                for(int page_i = 0; page_i < PAGE_TABLE_SIZE; ++ page_i)
                    printf(" *");
                printf("\n");
                continue;
            }
            for(int page_i = 0; page_i < PAGE_TABLE_SIZE; ++ page_i)
            {
                if(proc -> m_page_table[page_i].valid)
                {
                    printf(" %u:", page_i);
                    if(proc -> m_page_table[page_i].referenced)
                        printf("R");
                    else
                        printf("-");
                    if(proc -> m_page_table[page_i].modified)
                        printf("M");
                    else
                        printf("-");
                    if(proc -> m_page_table[page_i].pagedout)
                        printf("S");
                    else
                        printf("-");
                }
                else
                {
                    // not valid
                    if(proc -> m_page_table[page_i].pagedout)
                        printf(" #");
                    else
                        printf(" *");
                }
            }
            printf("\n");
        }
    }

    // F option
    m_pager -> print_frame_table();

    unsigned long long cost = 0;
    // S option
    if(m_myconfig->m_process_statistics_option)
    {
        for(int pid = 0; pid < m_process_num; ++ pid)
        {
            Process* proc = processes[pid];
            printf("PROC[%d]: U=%llu M=%llu I=%llu O=%llu FI=%llu FO=%llu Z=%llu SV=%llu SP=%llu\n", \
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
    cost += (m_ctx_switches * CONTEXT_SWITCH_COST + m_process_exits * PROCESS_EXIT_COST \
    + m_readwrites * READ_WRITE_COST);
    printf("TOTALCOST %llu %llu %llu %llu %lu\n", \
    m_inst_count, m_ctx_switches, m_process_exits, cost, sizeof(pte_t));
}