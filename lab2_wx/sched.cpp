#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <vector>
#include <queue>
#include <fstream>
#include <iostream>

#define DEFAULT_MAXPRIO 4
#define DEFAULT_QUANTUM 100000

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
    int cpu_t_remain_;
    int static_priority_;
    int dynamic_priority_;
    int state_begin_t_;
    process_state_t state_;

    Process(int _pid, int _at, int _tc, int _cb, int _io, int _maxprio);
};

class Scheduler
{
public:
    int quantum_;
    // std::vector<Process*> ready_list_, expire_list_;
    virtual void add_ready_process(Process* _p);
    virtual Process* get_next_process();
    virtual bool test_preempt(Process* p, int curtime);
    Scheduler(int _quantum);
};

class FCFS_Scheduler: public Scheduler
{
public:
    std::queue<Process*> ready_queue_, expire_queue_; 
    void add_ready_process(Process* p);
    Process* get_next_process();
    bool test_preempt(Process* p, int curtime);
    FCFS_Scheduler(int _quantum);
};

class Event
{
public:
    int timestamp_;
    Process* proc_ptr_;
    trans_type_t trans_type_;

    Event(int _timestamp, Process* _proc_ptr, trans_type_t _trans_type);
};

class event_comp 
{
public:
    bool operator()(Event* _a, Event* _b){return (_a->timestamp_) > (_b->timestamp_);}
};

// Discrete Event Simulation
class DES
{
public:
    std::priority_queue<Event*, std::vector<Event*>, event_comp> events_;
    void put_event(Event* _event_ptr);
    Event* get_event();
    void rm_event();
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

int main(int argc, char** argv)
{
    des = new DES();
    read_schedule_config(argc, argv);
    char* input_path = argv[argc-2];
    char* rand_path = argv[argc-1];
    read_rfile(rand_path);
    read_inputfile(input_path);
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
    cpu_t_remain_ = _tc;
    static_priority_ = myrandom(_maxprio);
    dynamic_priority_ = static_priority_ - 1;
    state_begin_t_ = _at;
    state_ = CREATED;
}

Scheduler::Scheduler(int _quantum)
{
    quantum_ = _quantum;
}

FCFS_Scheduler::FCFS_Scheduler(int _quantum):Scheduler(_quantum)
{

}

void FCFS_Scheduler::add_ready_process(Process* _p)
{
    ready_queue_.push(_p);
}

Process* FCFS_Scheduler::get_next_process()
{
    Process* p = ready_queue_.front();
    ready_queue_.pop();
    return p;
}

Event::Event(int _timestamp, Process* _proc_ptr, trans_type_t _trans_type)
{
    timestamp_ = _timestamp;
    proc_ptr_ = _proc_ptr;
    trans_type_ = _trans_type;
}

void DES::put_event(Event* _event_ptr)
{
    events_.push(_event_ptr);
}

Event* DES::get_event()
{
    Event* evt = events_.top();
    events_.pop();
}

void DES::rm_event()
{
    
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
            break;
        case 'S':
            sched_type = SRTF;
            break;
        case 'R':
            sched_type = RR;
            quantum = atoi(param_str.substr(1).c_str());
            break;
        case 'P':
            break;
    }
}

void read_rfile(char* _path)
{
    std::string line;
    std::ifstream rfile(_path);
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
    std::string line;
    std::ifstream inputfile(_path);
    int line_id = 0;
    while(getline(inputfile, line))
    {
        int start_i = 0;
        int end_i = line.find(' ');
        int AT = atoi(line.substr(start_i, end_i - start_i).c_str());
        start_i = end_i + 1;
        end_i = line.find(' ', start_i);
        int TC = atoi(line.substr(start_i, end_i - start_i).c_str());
        start_i = end_i + 1;
        end_i = line.find(' ', start_i);
        int CB = atoi(line.substr(start_i, end_i - start_i).c_str());
        start_i = end_i + 1;
        int IO = atoi(line.substr(start_i, end_i - start_i).c_str());
        Process* proc_ptr = new Process(line_id, AT, TC, CB, IO, maxprio);
        Event* event = new Event(AT, proc_ptr, TRANS_TO_READY);
        proc_ptr->state_ = READY;
        des->put_event(event);
        ++ line_id;
    }

}

void Simulation() 
{
    Event* event_ptr;
    Process* running_proc_ptr = NULL;
    while( (event_ptr = des->get_event()) ) 
    {
        Process *proc_ptr = event_ptr->proc_ptr_; 
        int curr_time = event_ptr->timestamp_;
        process_state_t prev_state = proc_ptr -> state_;
        int prev_state_time = curr_time - proc_ptr->state_begin_t_;
        proc_ptr->state_begin_t_ = curr_time;
        bool if_call_sched = false;
        switch(event_ptr->trans_type_) 
        { 
            case TRANS_TO_READY:
                // conditional on whether something is run
                if_call_sched = true; 
                sched_ptr -> add_ready_process(proc_ptr);
                proc_ptr -> state_ = READY;
                break;
            case TRANS_TO_RUN:
                // create event for either preemption or blocking
                proc_ptr -> state_ = RUNNING;
                
                int quantum = sched_ptr -> quantum_;
                int cpuburst = myrandom(proc_ptr->CB_);
                int proc_cpu_t_remain = proc_ptr -> cpu_t_remain_;

                if(proc_cpu_t_remain <= quantum && proc_cpu_t_remain <= cpuburst)
                {
                    // done
                    des -> put_event(new Event(curr_time + proc_cpu_t_remain, proc_ptr, TRANS_TO_DONE));
                }
                else if(cpuburst <= quantum)
                {
                    // cpuburst min
                    // to block after cpu burst
                    des -> put_event(new Event(curr_time + cpuburst, proc_ptr, TRANS_TO_BLOCK));
                }
                else
                {
                    // quantum min trans to preempted
                    des-> put_event(new Event(curr_time + quantum, proc_ptr, TRANS_TO_PREEMPT));
                }
                break;
            case TRANS_TO_BLOCK:
                //create an event for when process becomes READY again
                proc_ptr -> state_
                CALL_SCHEDULER = true;
                break; 
            case TRANS_TO_PREEMPT:
                // add to runqueue (no event is generated)
                CALL_SCHEDULER = true;
                break;
        }
        // remove current event object from Memory
        delete evt; evt = nullptr;
        if(CALL_SCHEDULER) 
        {
            if (get_next_event_time() == CURRENT_TIME)
            continue; //process next event from Event queue
            CALL_SCHEDULER = false; // reset global flag
            if (CURRENT_RUNNING_PROCESS == nullptr) 
            {
                CURRENT_RUNNING_PROCESS = THE_SCHEDULER->get_next_process();
                if (CURRENT_RUNNING_PROCESS == nullptr)
                    continue;
                // create event to make this process runnable for same time.
            } 
        } 
    } 
}