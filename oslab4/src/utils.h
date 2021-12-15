#ifndef _UTILS_H
#define _UTILS_H
#include "myenv.h"

policy_t get_policy(int _argc, char** _argv);

class MyInput
{
public:
    int m_req_arrive_times[MAX_REQ_NUM];
    int m_req_tracks[MAX_REQ_NUM];
    int m_req_num;
    int m_next_req;
    MyInput(char* _path);
    ~MyInput();
    bool all_processed();
    int get_next_req_arrive_time();
    void get_req(int* _req_i_ptr, int* _arrive_time_ptr, int* _track_ptr);
};

#endif