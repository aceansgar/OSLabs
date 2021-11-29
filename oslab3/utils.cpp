#include "utils.h"

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
        {
            sched_type = PREPRIO;
            sscanf(param_str.substr(1).c_str(), "%d:%d", &quantum, &maxprio);
            // printf("quantum: %d, maxprio: %d\n", quantum, maxprio);
            sched_ptr = new PREPRIO_Scheduler(quantum, maxprio);
            break;
        }
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


int myrandom(int _burst) 
{ 
    int res = 1 + (randvals[ofs] % _burst); 
    ++ ofs;
    return res;
}
