#ifndef _MANAGER_H
#define _MANAGER_H

#include "utils.h"

class Request
{
public:
    int m_req_i;
    int m_arrive_time;
    int m_track;
    int m_start_time;
    int m_end_time;
    Request(int _req_i, int _arrive_time, int _track);
};

class Scheduler
{
public:
    Scheduler();
    virtual ~Scheduler();
    virtual void add_req(Request* _req) = 0;
    virtual bool exist_pending_reqs() = 0;
    virtual Request* get_next_req(int _curr_track) = 0;
};

class FIFOScheduler: public Scheduler
{
public:
    std::queue<Request*> m_pending_reqs;
    FIFOScheduler();
    ~FIFOScheduler();
    void add_req(Request* _req);
    bool exist_pending_reqs();
    Request* get_next_req(int _curr_track);
};

class SSTFScheduler: public Scheduler
{
public:
    std::list<Request*> m_pending_reqs;
    SSTFScheduler();
    ~SSTFScheduler();
    void add_req(Request* _req);
    bool exist_pending_reqs();
    Request* get_next_req(int _curr_track);
};

class LOOKScheduler: public Scheduler
{
public:
    std::list<Request*> m_pending_reqs; // in order
    LOOKScheduler();
    ~LOOKScheduler();
    void add_req(Request* _req);
    bool exist_pending_reqs();
    Request* get_next_req(int _curr_track);
};

class CLOOKScheduler: public Scheduler
{
public:
    std::list<Request*> m_pending_reqs; // in order
    CLOOKScheduler();
    ~CLOOKScheduler();
    void add_req(Request* _req);
    bool exist_pending_reqs();
    Request* get_next_req(int _curr_track);
};

class FLOOKScheduler: public Scheduler
{
public:
    std::list<Request*> m_pending_reqs; // in order, active queue
    std::list<Request*> m_prepare_reqs; // in order, add queue
    FLOOKScheduler();
    ~FLOOKScheduler();
    void add_req(Request* _req);
    bool exist_pending_reqs();
    Request* get_next_req(int _curr_track);
};

class Simulator
{
public:
    Request* m_reqs[MAX_REQ_NUM]; // for final print
    MyInput* m_myinput;
    Scheduler* m_scheduler;
    int m_total_time;
    int m_total_movement;
    Simulator(MyInput* _myinput, policy_t _policy);
    ~Simulator();
    void simulate();
    void print_summary();
};

#endif