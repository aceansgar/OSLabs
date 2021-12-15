#include "utils.h"

policy_t get_policy(int _argc, char** _argv)
{
    char c;
    policy_t policy;
    while ((c = getopt(_argc,_argv,"s:vqf")) != -1 )
    {
        switch(c)
        {
            case 's':
            {
                switch(optarg[0])
                {
                    case 'i':
                    {
                        policy = FIFO;
                        break;
                    }
                    case 'j':
                    {
                        policy = SSTF;
                        break;
                    }
                    case 's':
                    {
                        policy = LOOK;
                        break;
                    }
                    case 'c':
                    {
                        policy = CLOOK;
                        break;
                    }
                    case 'f':
                    {
                        policy = FLOOK;
                        break;
                    }
                    default:
                    {
                        fprintf(stderr, "incorrect policy\n");
                        break;
                    }
                }
                break;
            }
            default:
            {
                break;
            }
        }
    }
    return policy;
}

MyInput::MyInput(char* _path)
{
    FILE* infile = fopen(_path, "r");
    char line[1000];
    char* token_p;
    int req_i = 0;
    while(fgets(line, 1000, infile))
    {
        if(line[0]=='#')
            continue;
        token_p = strtok(line, " \n\t");
        m_req_arrive_times[req_i] = atoi(token_p);
        token_p = strtok(NULL, " \n\t");
        m_req_tracks[req_i] = atoi(token_p);
        ++ req_i;
    }
    m_req_num = req_i;
    m_next_req = 0;
    fclose(infile);
}

MyInput::~MyInput()
{

}

bool MyInput::all_processed()
{
    return m_next_req >= m_req_num;
}

int MyInput::get_next_req_arrive_time()
{
    return m_req_arrive_times[m_next_req];
}

void MyInput::get_req(int* _req_i_ptr, int* _arrive_time_ptr, int* _track_ptr)
{
    *_req_i_ptr = m_next_req;
    *_arrive_time_ptr = m_req_arrive_times[m_next_req];
    *_track_ptr = m_req_tracks[m_next_req];
    ++ m_next_req;
}


