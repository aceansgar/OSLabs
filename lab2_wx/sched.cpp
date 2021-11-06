#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <vector>
#include <queue>
#include <list>
#include <stack>
#include <fstream>
#include <iostream>

#define DEFAULT_MAXPRIO 4
#define DEFAULT_QUANTUM 100000
#define DEBUG

typedef enum{RUNNING, BLOCKED, CREATED, READY} process_state_t;
typedef enum{FCFS, LCFS, SRTF, RR, PRIO, PREPRIO} sched_type_t;
typedef enum{TRANS_TO_READY, TRANS_TO_RUN, TRANS_TO_BLOCK, TRANS_TO_PREEMPT, TRANS_TO_DONE} trans_type_t;

class Process
{
public:
    int AT_; //arrival time
    int TC_; //total cpu time
    int CB_; //CPU burst
    int IO_; //IO burst
    int pid_;
    int io_t_sum_;
    int cpu_t_remain_;
    int cpuburst_remain_;
    int static_priority_;
    int dynamic_priority_;
    int state_begin_t_;
    process_state_t old_state_;
    process_state_t state_;

    Process(int _pid, int _at, int _tc, int _cb, int _io, int _maxprio);
    ~Process();
};

class Scheduler
{
public:
    int quantum_;
    std::list<Process*> expired_list_;
    // std::vector<Process*> ready_list_, expire_list_;
    Scheduler(int _quantum);
    ~Scheduler();
    virtual void add_ready_process(Process* _p);
    void add_expired_process(Process* _p);
    virtual Process* get_next_process();
    virtual void add_inactive_process(Process* _p); // for PRIO
    // virtual bool test_preempt(Process* p, int curtime);
};

class FCFS_Scheduler: public Scheduler
{
public:
    std::queue<Process*> ready_queue_;
    FCFS_Scheduler(int _quantum);
    ~FCFS_Scheduler();
    void add_ready_process(Process* p);
    Process* get_next_process();
    // bool test_preempt(Process* p, int curtime);
};

class LCFS_Scheduler: public Scheduler
{
public:
    std::stack<Process*> ready_stack_;
    LCFS_Scheduler(int _quantum);
    ~LCFS_Scheduler();
    void add_ready_process(Process* p);
    Process* get_next_process();
    // bool test_preempt(Process* p, int curtime);
};

class SR_proc_comp 
{
public:
    bool operator()(Process* _a, Process* _b){return (_a->cpu_t_remain_) > (_b->cpu_t_remain_);}
};

class SRTF_Scheduler: public Scheduler
{
public:
    std::priority_queue<Process*, std::vector<Process*>, SR_proc_comp> ready_queue_;
    SRTF_Scheduler(int _quantum);
    ~SRTF_Scheduler();
    void add_ready_process(Process* p);
    Process* get_next_process();
    // bool test_preempt(Process* p, int curtime);
};

class PRIO_Scheduler: public Scheduler
{
public:
    int maxprio_; // levels
    std::vector<std::queue<Process*>> ready_queues_;
    std::vector<std::queue<Process*>> inactive_queues_;
    PRIO_Scheduler(int _quantum, int _maxprio);
    ~PRIO_Scheduler();
    void add_ready_process(Process* _p);
    void add_inactive_process(Process* _p);
    Process* get_next_process();
    // bool test_preempt(Process* p, int curtime);
};

class Event
{
public:
    int timestamp_;
    Process* proc_ptr_;
    trans_type_t trans_type_;

    Event(int _timestamp, Process* _proc_ptr, trans_type_t _trans_type);
    ~Event();
};

// class event_comp 
// {
// public:
//     bool operator()(Event* _a, Event* _b){return (_a->timestamp_) > (_b->timestamp_);}
// };

// Discrete Event Simulation
class DES
{
public:
    // std::priority_queue<Event*, std::vector<Event*>, event_comp> events_;
    int iobusy_t_;
    int last_io_end_t_;
    std::list<Event*> events_;
    DES();
    ~DES();
    void put_event(Event* _event_ptr);
    Event* get_event();
    int get_next_event_time();
    void rm_event();
    void print_events();
};

std::vector<int> randvals;
bool verbose = false;
int maxprio = DEFAULT_MAXPRIO;
int ofs=0;//offset of random seed
sched_type_t sched_type;
Scheduler* sched_ptr;
DES* des;

void read_schedule_config(int _argc, char** _argv);
void read_rfile(char* _path);
int myrandom(int _burst);
void read_inputfile(char* _path);
void Simulation();
void final_output();

int main(int argc, char** argv)
{
    des = new DES();
    read_schedule_config(argc, argv);
    char* input_path = argv[argc-2];
    char* rand_path = argv[argc-1];
    read_rfile(rand_path);
    read_inputfile(input_path);
    des->print_events();
    printf("======begin simulation: \n");
    Simulation();
    printf("======begin final output: \n");
    final_output();
    delete des;
    return 0;
}

Process::Process(int _pid, int _at, int _tc, int _cb, int _io, int _maxprio)
{
    pid_ = _pid;
    AT_ = _at;
    TC_ = _tc;
    CB_ = _cb;
    IO_ = _io;
    io_t_sum_ = 0;
    cpu_t_remain_ = _tc;
    cpuburst_remain_ = 0;
    static_priority_ = myrandom(_maxprio);
    dynamic_priority_ = static_priority_ - 1;
    state_begin_t_ = _at;
    old_state_ = CREATED;
    state_ = CREATED;
}

Process::~Process()
{

}

Scheduler::Scheduler(int _quantum)
{
    quantum_ = _quantum;
}

Scheduler::~Scheduler()
{

}

void Scheduler::add_ready_process(Process* _p)
{

}

void Scheduler::add_expired_process(Process* _p)
{
    if(expired_list_.empty())
    {
        expired_list_.push_back(_p);
        return;
    }
    std::list<Process*>::iterator it;
    int pid = _p->pid_;
    for (it=expired_list_.begin(); it != expired_list_.end(); ++it)
    {
        int tmp_pid = (*it)->pid_;
        if(pid < tmp_pid)
            break;
    }
    expired_list_.insert(it, _p);
}

Process* Scheduler::get_next_process()
{
    return NULL;
}

void Scheduler::add_inactive_process(Process* _p)
{

}

FCFS_Scheduler::FCFS_Scheduler(int _quantum):Scheduler(_quantum)
{

}

FCFS_Scheduler::~FCFS_Scheduler()
{

}

void FCFS_Scheduler::add_ready_process(Process* _p)
{
    ready_queue_.push(_p);
}

Process* FCFS_Scheduler::get_next_process()
{
    if(ready_queue_.empty())
        return NULL;
    Process* p = ready_queue_.front();
    ready_queue_.pop();
    return p;
}

LCFS_Scheduler::LCFS_Scheduler(int _quantum):Scheduler(_quantum)
{

}

LCFS_Scheduler::~LCFS_Scheduler()
{

}

void LCFS_Scheduler::add_ready_process(Process* _p)
{
    ready_stack_.push(_p);
}

Process* LCFS_Scheduler::get_next_process()
{
    if(ready_stack_.empty())
        return NULL;
    Process* p = ready_stack_.top();
    ready_stack_.pop();
    return p;
}

SRTF_Scheduler::SRTF_Scheduler(int _quantum):Scheduler(_quantum)
{

}

SRTF_Scheduler::~SRTF_Scheduler()
{

}

void SRTF_Scheduler::add_ready_process(Process* _p)
{
    ready_queue_.push(_p);
}

Process* SRTF_Scheduler::get_next_process()
{
    if(ready_queue_.empty())
        return NULL;
    Process* p = ready_queue_.top();
    ready_queue_.pop();
    return p;
}

PRIO_Scheduler::PRIO_Scheduler(int _quantum, int _maxprio):Scheduler(_quantum)
{
    maxprio_ = _maxprio;
    ready_queues_ = std::vector<std::queue<Process*>>(_maxprio, std::queue<Process*>());
    inactive_queues_ = std::vector<std::queue<Process*>>(_maxprio, std::queue<Process*>());
}

PRIO_Scheduler::~PRIO_Scheduler()
{

}

void PRIO_Scheduler::add_ready_process(Process* _p)
{
    // put into corresponding level
    ready_queues_[_p->dynamic_priority_].push(_p);
}

void PRIO_Scheduler::add_inactive_process(Process* _p)
{
    inactive_queues_[_p->dynamic_priority_].push(_p);
}

Process* PRIO_Scheduler::get_next_process()
{
    for(int prio_level = maxprio_ - 1; prio_level >=0 ; -- prio_level)
    {
        if(ready_queues_[prio_level].empty())
            continue;
        Process* p = ready_queues_[prio_level].front();
        ready_queues_[prio_level].pop();
        return p;
    }
    ready_queues_ = inactive_queues_;
    inactive_queues_ = std::vector<std::queue<Process*>>(maxprio_, std::queue<Process*>());
    for(int prio_level = maxprio_ - 1; prio_level >=0 ; -- prio_level)
    {
        if(ready_queues_[prio_level].empty())
            continue;
        Process* p = ready_queues_[prio_level].front();
        ready_queues_[prio_level].pop();
        return p;
    }
    return NULL;
}


Event::Event(int _timestamp, Process* _proc_ptr, trans_type_t _trans_type)
{
    timestamp_ = _timestamp;
    proc_ptr_ = _proc_ptr;
    trans_type_ = _trans_type;
}

Event::~Event()
{

}

DES::DES()
{
    iobusy_t_ = 0;
    last_io_end_t_ = 0;
}

DES::~DES()
{

}

void DES::put_event(Event* _event_ptr)
{
    // same time stamp: generated order
    int evt_time = _event_ptr -> timestamp_;
    if(des->events_.empty())
    {
        des->events_.push_back(_event_ptr);
        return;
    }
    std::list<Event*>::iterator it;
    for (it=events_.begin(); it != events_.end(); ++it)
    {
        int tmp_time = (*it)->timestamp_;
        if(evt_time < tmp_time)
            break;
    }
    events_.insert(it, _event_ptr);
}

Event* DES::get_event()
{
    if(events_.empty())
        return NULL;
    Event* evt = events_.front();
    events_.pop_front();
    return evt;
}

int DES::get_next_event_time()
{
    if(events_.empty())
        return -1;
    return events_.front()->timestamp_;
}


void DES::rm_event()
{
    
}

void DES::print_events()
{
    printf("%d events in DES\n", (int)(events_.size()));
}

void read_schedule_config(int _argc, char** _argv)
{
    char c;
    std::string param_str;
    int quantum = DEFAULT_QUANTUM; // large enough for non-preemptive
    while ((c = getopt(_argc,_argv,"vs:")) != -1 )
	{
		switch(c) 
        {
		    case 'v':
                verbose = true;
                break;
            case 's':
                param_str = std::string(optarg);
                break;
            default:
                break;
		}
	}
    switch(param_str[0])
    {
        case 'F':
            sched_type = FCFS;
            sched_ptr = new FCFS_Scheduler(DEFAULT_QUANTUM);
            break;
        case 'L':
            sched_type = LCFS;
            sched_ptr = new LCFS_Scheduler(DEFAULT_QUANTUM);
            break;
        case 'S':
            sched_type = SRTF;
            sched_ptr = new SRTF_Scheduler(DEFAULT_QUANTUM);
            break;
        case 'R':
        {
            sched_type = RR;
            quantum = atoi(param_str.substr(1).c_str());
            // printf("quantum: %d\n",quantum);
            sched_ptr = new FCFS_Scheduler(quantum);
            break;
        }
        case 'P':
        {
            sched_type = PRIO;
            sscanf(param_str.substr(1).c_str(), "%d:%d", &quantum, &maxprio);
            // printf("quantum: %d, maxprio: %d\n", quantum, maxprio);
            sched_ptr = new PRIO_Scheduler(quantum, maxprio);
            break;
        }
        case 'E':
            break;
    }
}

void read_rfile(char* _path)
{
    std::string line;
    std::ifstream rfile(_path);
    getline(rfile,line);
    while(getline(rfile, line))
    {
        randvals.push_back(atoi(line.c_str()));
    }
}

int myrandom(int _burst) 
{ 
    int res = 1 + (randvals[ofs] % _burst); 
    ++ ofs;
    return res;
}

void read_inputfile(char* _path)
{
    char line[1000];
    std::ifstream inputfile(_path);
    int line_id = 0;
    while(inputfile.getline(line, 1000))
    {
        char* token = strtok(line, " ");
        int AT = atoi(token);
        token = strtok(NULL, " ");
        int TC = atoi(token);
        token = strtok(NULL, " ");
        int CB = atoi(token);
        token = strtok(NULL, " ");
        int IO = atoi(token);
        // printf("pid:%d, AT:%d, TC:%d, CB:%d, IO:%d\n", line_id, AT, TC, CB, IO);
        Process* proc_ptr = new Process(line_id, AT, TC, CB, IO, maxprio);
        Event* event = new Event(AT, proc_ptr, TRANS_TO_READY);
        des->put_event(event);
        ++ line_id;
    }

}

void Simulation() 
{
    Event* event_ptr;
    Process* curr_proc = NULL;
    bool if_call_sched = false;
    while( (event_ptr = des->get_event()) ) 
    {
        // printf("get an event from DES\n");
        Process *proc_ptr = event_ptr->proc_ptr_; 
        int curr_time = event_ptr->timestamp_;
        process_state_t prev_state = proc_ptr -> state_;
        int prev_state_time = curr_time - proc_ptr->state_begin_t_;
        proc_ptr->state_begin_t_ = curr_time;
        switch(event_ptr->trans_type_) 
        { 
            case TRANS_TO_READY:
            {
                // except from running (preempted)
                // conditional on whether something is run
                if(verbose)
                {
                    if((prev_state)==CREATED)
                        printf("%d %d %d: CREATED -> READY\n", curr_time, proc_ptr->pid_, 0);
                    else
                        printf("%d %d %d: BLOCK -> READY\n", curr_time, proc_ptr->pid_, prev_state_time);
                }
                proc_ptr -> old_state_ = proc_ptr -> state_;
                proc_ptr -> state_ = READY;
                proc_ptr -> state_begin_t_ = curr_time;
                proc_ptr -> io_t_sum_ += prev_state_time;
                if_call_sched = true; 

                // block/created -> ready 
                proc_ptr -> dynamic_priority_ = proc_ptr-> static_priority_ - 1;
                sched_ptr -> add_ready_process(proc_ptr);

                // printf("ready process added\n");
                break;
            }
            case TRANS_TO_RUN:
            {
                // create event for either preemption or blocking
#ifdef DEBUG
                // printf("CB: %d\n", proc_ptr->CB_);
#endif
                curr_proc = proc_ptr;
                proc_ptr -> old_state_ = proc_ptr -> state_;
                proc_ptr -> state_ = RUNNING;
                proc_ptr -> state_begin_t_ = curr_time;
                int quantum = sched_ptr -> quantum_;
                int cpuburst = proc_ptr -> cpuburst_remain_;
                if(cpuburst == 0)
                    cpuburst = myrandom(proc_ptr->CB_);
                int proc_cpu_t_remain = proc_ptr -> cpu_t_remain_;

                if(proc_cpu_t_remain <= quantum && proc_cpu_t_remain <= cpuburst)
                {
                    // done
                    des -> put_event(new Event(curr_time + proc_cpu_t_remain, proc_ptr, TRANS_TO_DONE));
                    proc_ptr -> cpuburst_remain_ = cpuburst;
                }
                else if(cpuburst <= quantum)
                {
                    // cpuburst min
                    // to block after cpu burst
                    des -> put_event(new Event(curr_time + cpuburst, proc_ptr, TRANS_TO_BLOCK));
                    proc_ptr -> cpuburst_remain_ = cpuburst;
                }
                else
                {
                    // quantum min, trans to preempted
                    des-> put_event(new Event(curr_time + quantum, proc_ptr, TRANS_TO_PREEMPT));
                    proc_ptr -> cpuburst_remain_ = cpuburst;
                }
                if(verbose)
                {
                    printf("%d %d %d: READY -> RUNNG cb=%d rem=%d prio=%d\n", \
                    curr_time, proc_ptr->pid_, prev_state_time, cpuburst, proc_ptr->cpu_t_remain_, proc_ptr->dynamic_priority_);
                }
                break;
            }
            case TRANS_TO_BLOCK:
            {
                //create an event for when process becomes READY again
                curr_proc = NULL;
                if_call_sched = true;
                proc_ptr -> old_state_ = proc_ptr -> state_;
                proc_ptr -> state_ = BLOCKED;
                proc_ptr -> state_begin_t_ = curr_time;
                proc_ptr -> cpuburst_remain_ = 0;
                proc_ptr -> cpu_t_remain_ -= prev_state_time;
                int ioburst = myrandom(proc_ptr -> IO_);
                int io_end_t = curr_time + ioburst;
                des->put_event(new Event(io_end_t, proc_ptr, TRANS_TO_READY));
                if(io_end_t > (des->last_io_end_t_))
                    {
                        if((des->last_io_end_t_) <= curr_time)
                        {
                            des->iobusy_t_ += ioburst;
                        }
                        else
                        {
                            des->iobusy_t_ += (io_end_t - (des->last_io_end_t_)); 
                        }
                        des->last_io_end_t_ = io_end_t;
                    }
                if(verbose)
                {
                    printf("%d %d %d: RUNNG -> BLOCK  ib=%d rem=%d\n", \
                    curr_time, proc_ptr->pid_, prev_state_time, ioburst, proc_ptr -> cpu_t_remain_);
                }
                break; 
            }
            case TRANS_TO_PREEMPT:
            {
                // be preempted, put into ready queue
                // add to runqueue (no event is generated)
                curr_proc = NULL;
                if_call_sched = true;
                proc_ptr -> old_state_ = proc_ptr -> state_;
                proc_ptr -> state_ = READY;
                proc_ptr -> state_begin_t_ = curr_time;
                proc_ptr -> cpuburst_remain_ -= prev_state_time;
                proc_ptr -> cpu_t_remain_ -= prev_state_time;
                
                if(verbose)
                {
                    printf("%d %d %d: RUNNG -> READY  cb=%d rem=%d prio=%d\n", \
                    curr_time, proc_ptr->pid_, prev_state_time, proc_ptr->cpuburst_remain_, proc_ptr->cpu_t_remain_, \
                    proc_ptr->dynamic_priority_);
                }
                int new_dynamic_prio = (proc_ptr -> dynamic_priority_) - 1;
                if(new_dynamic_prio == -1)
                {
                    new_dynamic_prio = (proc_ptr -> static_priority_) - 1;
                    proc_ptr -> dynamic_priority_ = new_dynamic_prio;
                    if(sched_type == PRIO || sched_type == PREPRIO)
                    {
                        sched_ptr -> add_inactive_process(proc_ptr);
                    }
                    else
                    {
                        sched_ptr -> add_ready_process(proc_ptr);
                    }
                }
                else
                {
                    proc_ptr -> dynamic_priority_ = new_dynamic_prio;
                    sched_ptr -> add_ready_process(proc_ptr);
                }
                break;
            }
            case TRANS_TO_DONE:
            {
                // this process is done
                curr_proc = NULL;
                if_call_sched = true;
                // add to expired queue, for final print
                proc_ptr -> state_begin_t_ = curr_time;
                sched_ptr -> add_expired_process(proc_ptr);
                if(verbose)
                {
                    printf("%d %d %d: Done\n", curr_time, proc_ptr->pid_, prev_state_time);
                }
                break;
            }
        }
        // remove current event object from Memory
        delete event_ptr; 
        event_ptr = NULL;
        if(if_call_sched) 
        {
            if (des->get_next_event_time() == curr_time)
                continue; //process next event first
            if_call_sched = false; // reset global flag
            if (curr_proc == NULL) 
            {
                // printf("to get next process from scheduler\n");
                curr_proc = sched_ptr->get_next_process();
                if (curr_proc == NULL)
                    // no ready process at this time
                    continue;
                // create event to make this process runnable for same time.
                des -> put_event(new Event(curr_time, curr_proc, TRANS_TO_RUN));
            } 
        } 
    } 
}

void final_output()
{
    switch(sched_type)
    {
        case FCFS:
            printf("FCFS\n");
            break;
        case LCFS:
            printf("LCFS\n");
            break;
        case SRTF:
            printf("SRTF\n");
            break;
        case RR:
            printf("RR %d\n", sched_ptr->quantum_);
            break;
        case PRIO:
            printf("PRIO %d\n", sched_ptr->quantum_);
            break;
    }
    int last_finish_t=0;
    int proc_num = 0;
    int sum_tt=0; //sum turn around time
    int sum_cw=0; //sum cpu waiting time
    int t_cpubusy = 0;
    int t_iobusy = des->iobusy_t_; 
    for(std::list<Process*>::iterator it = sched_ptr->expired_list_.begin(); it!=sched_ptr->expired_list_.end(); ++it)
    {
        ++ proc_num;
        Process* p = *it;
        int pid = p->pid_;
        int AT = p->AT_;
        int TC = p->TC_;
        int CB = p->CB_;
        int IO = p->IO_;
        int PRIO = p->static_priority_;
        int FT = p->state_begin_t_;
        int TT = FT - AT; //turnaround time
        int IT = p->io_t_sum_;
        int CW = TT - TC - IT;
        printf("%04d: %4d %4d %4d %4d %1d | %5d %5d %5d %5d\n", pid, AT, TC, CB, IO, PRIO, FT, TT, IT, CW);
        if(FT>last_finish_t)
            last_finish_t = FT;
        sum_tt += TT;
        sum_cw += CW;
        t_cpubusy += TC;  
        delete p;
    }
    // printf("sum tt: %d\n", sum_tt);
    double avg_tt = (double)sum_tt/proc_num;
    // printf("avg tt: %.2lf\n", avg_tt);
    double avg_cw = (double)sum_cw/proc_num;
    double cpu_util = 100.0 * (t_cpubusy / (double)last_finish_t);
    double io_util = 100.0 * (t_iobusy / (double)last_finish_t);
    double throughput = 100.0 * (proc_num / (double)last_finish_t);
    printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n", last_finish_t, cpu_util, io_util, avg_tt, avg_cw, throughput);
}