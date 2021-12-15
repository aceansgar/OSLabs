#include "manager.h"

bool curr_direction;

Request::Request(int _req_i, int _arrive_time, int _track)
{
    m_req_i = _req_i;
    m_arrive_time = _arrive_time;
    m_track = _track;
}

Scheduler::Scheduler()
{

}

Scheduler::~Scheduler()
{

}

FIFOScheduler::FIFOScheduler(): Scheduler()
{

}

FIFOScheduler::~FIFOScheduler()
{

}

void FIFOScheduler::add_req(Request* _req)
{
    m_pending_reqs.push(_req);
}

bool FIFOScheduler::exist_pending_reqs()
{
    return !m_pending_reqs.empty();
}

Request* FIFOScheduler::get_next_req(int _curr_track)
{
    Request* req = m_pending_reqs.front();
    m_pending_reqs.pop();
    return req;
}


SSTFScheduler::SSTFScheduler(): Scheduler()
{

}

SSTFScheduler::~SSTFScheduler()
{

}

void SSTFScheduler::add_req(Request* _req)
{
    m_pending_reqs.push_back(_req);
}

bool SSTFScheduler::exist_pending_reqs()
{
    return !m_pending_reqs.empty();
}

Request* SSTFScheduler::get_next_req(int _curr_track)
{
    int min_dist = INT_MAX;
    Request* req_selected = NULL;
    std::list<Request*>::iterator iter_selected;
    for(std::list<Request*>::iterator it = m_pending_reqs.begin(); it != m_pending_reqs.end(); ++ it)
    {
        Request* req = *it;
        if(abs(req->m_track - _curr_track) < min_dist)
        {
            min_dist = abs(req->m_track - _curr_track);
            req_selected = req;
            iter_selected = it;
        }
    }
    m_pending_reqs.erase(iter_selected);
    return req_selected;
}

LOOKScheduler::LOOKScheduler(): Scheduler()
{

}

LOOKScheduler::~LOOKScheduler()
{

}

void LOOKScheduler::add_req(Request* _req)
{
    // insert in order
    if(m_pending_reqs.empty())
    {
        m_pending_reqs.push_back(_req);
        return;
    }
    std::list<Request*>::iterator it;
    for(it = m_pending_reqs.begin(); it != m_pending_reqs.end(); ++ it)
    {
        Request* req = *it;
        if(req->m_track > _req->m_track)
            break;
    }
    m_pending_reqs.insert(it, _req);
}

bool LOOKScheduler::exist_pending_reqs()
{
    return !m_pending_reqs.empty();
}

Request* LOOKScheduler::get_next_req(int _curr_track)
{
    std::list<Request*>::iterator it;
    if(curr_direction) // to right
    {
        for(it = m_pending_reqs.begin(); it != m_pending_reqs.end(); ++ it)
        {
            Request* req = *it;
            if(req->m_track >= _curr_track)
                break;
        }
        if(it == m_pending_reqs.end()) // no reqs on the right
        {
            curr_direction = false; // change direction to left
            return get_next_req(_curr_track);
        }
        else
        {
            // found
            Request* req = *it;
            m_pending_reqs.erase(it);
            return req;
        }
    }
    else // to left
    {
        for(it = m_pending_reqs.begin(); it != m_pending_reqs.end(); ++ it)
        {
            Request* req = *it;
            if(req->m_track > _curr_track)
                break;
        }
        if(it == m_pending_reqs.begin()) // no reqs on the left
        {
            curr_direction = true; // change direction to right
            return get_next_req(_curr_track);
        }
        -- it; // the rightmost iterator of selected track
        int track_selected = (*it)->m_track;
        // find the leftmost iterator of selected track
        for(std::list<Request*>::iterator iter = m_pending_reqs.begin(); iter != m_pending_reqs.end(); ++ iter)
        {
            Request* req = *iter;
            if(req->m_track == track_selected)
            {
                m_pending_reqs.erase(iter);
                return req;
            }
        }
        fprintf(stderr, "error in get_next_req");
        return NULL;
    }
}

CLOOKScheduler::CLOOKScheduler(): Scheduler()
{

}

CLOOKScheduler::~CLOOKScheduler()
{

}

void CLOOKScheduler::add_req(Request* _req)
{
    // insert in order
    if(m_pending_reqs.empty())
    {
        m_pending_reqs.push_back(_req);
        return;
    }
    std::list<Request*>::iterator it;
    for(it = m_pending_reqs.begin(); it != m_pending_reqs.end(); ++ it)
    {
        Request* req = *it;
        if(req->m_track > _req->m_track)
            break;
    }
    m_pending_reqs.insert(it, _req);
}

bool CLOOKScheduler::exist_pending_reqs()
{
    return !m_pending_reqs.empty();
}

Request* CLOOKScheduler::get_next_req(int _curr_track)
{
    std::list<Request*>::iterator it;
    for(it = m_pending_reqs.begin(); it != m_pending_reqs.end(); ++ it)
    {
        Request* req = *it;
        if(req->m_track >= _curr_track)
            break;
    }
    if(it == m_pending_reqs.end()) // no reqs on the right
    {
        // jump to leftmost
        Request* req = *(m_pending_reqs.begin());
        m_pending_reqs.erase(m_pending_reqs.begin());
        return req;
    }
    else
    {
        // found
        Request* req = *it;
        m_pending_reqs.erase(it);
        return req;
    } 
}

FLOOKScheduler::FLOOKScheduler(): Scheduler()
{

}

CLOOKScheduler::~CLOOKScheduler()
{

}

void CLOOKScheduler::add_req(Request* _req)
{
    // insert in order
    if(m_pending_reqs.empty())
    {
        m_pending_reqs.push_back(_req);
        return;
    }
    std::list<Request*>::iterator it;
    for(it = m_pending_reqs.begin(); it != m_pending_reqs.end(); ++ it)
    {
        Request* req = *it;
        if(req->m_track > _req->m_track)
            break;
    }
    m_pending_reqs.insert(it, _req);
}

bool CLOOKScheduler::exist_pending_reqs()
{
    return !m_pending_reqs.empty();
}

Request* CLOOKScheduler::get_next_req(int _curr_track)
{
    std::list<Request*>::iterator it;
    for(it = m_pending_reqs.begin(); it != m_pending_reqs.end(); ++ it)
    {
        Request* req = *it;
        if(req->m_track >= _curr_track)
            break;
    }
    if(it == m_pending_reqs.end()) // no reqs on the right
    {
        // jump to leftmost
        Request* req = *(m_pending_reqs.begin());
        m_pending_reqs.erase(m_pending_reqs.begin());
        return req;
    }
    else
    {
        // found
        Request* req = *it;
        m_pending_reqs.erase(it);
        return req;
    } 
}


Simulator::Simulator(MyInput* _myinput, policy_t _policy)
{
    m_myinput = _myinput;
    switch(_policy)
    {
        case FIFO:
        {
            m_scheduler = new FIFOScheduler;
            break;
        }
        case SSTF:
        {
            m_scheduler = new SSTFScheduler;
            break;
        }
        case LOOK:
        {
            m_scheduler = new LOOKScheduler;
            break;
        }
        case CLOOK:
        {
            m_scheduler = new CLOOKScheduler;
            break;
        }
        default:
        {
            break;
        }
    }
}

Simulator::~Simulator()
{

}

void Simulator::simulate()
{
    int curr_time = 0;
    Request* curr_req = NULL;
    int curr_track = 0;
    m_total_movement = 0;
    curr_direction = true; // initialize to right
    while(true)
    {
        // check if a new I/O arrived to the system at this current time
        if(m_myinput->get_next_req_arrive_time() == curr_time)
        {
            int req_i;
            int arrive_time;
            int track;
            m_myinput->get_req(&req_i, &arrive_time, &track);
            m_scheduler->add_req(new Request(req_i, arrive_time, track));
            // printf("req %d arrived at time %d track: %d\n", req_i, curr_time, track);
        }
        // if an IO is active and completed at this time
        while(true)
        {
            if(curr_req && curr_req->m_track == curr_track)
            {
                // Compute relevant info and store in IO request for final summary
                curr_req->m_end_time = curr_time;
                m_reqs[curr_req->m_req_i] = curr_req;
                curr_req = NULL;
            }
            if(!curr_req) // if no IO request active now
            {
                // if requests are pending
                if(m_scheduler->exist_pending_reqs())
                {
                    // Fetch the next request from IO-queue and start the new IO
                    curr_req = m_scheduler->get_next_req(curr_track);
                    curr_req->m_start_time = curr_time;
                }
                else if(m_myinput->all_processed())
                {
                    // exit simulation
                    m_total_time = curr_time;
                    return;
                }
            }
            if(!curr_req || (curr_req && curr_req->m_track != curr_track))
                break;
        }
        // if an IO is active
        if(curr_req)
        {
            // Move the head by one unit in the direction its going (to simulate seek)
            if(curr_track < curr_req->m_track)
            {
                curr_direction = true; // to right
                ++ curr_track;
            }
            else
            {
                curr_direction = false; // to left
                -- curr_track;
            }
            ++ m_total_movement;
        }
        ++ curr_time;
    }
}

void Simulator::print_summary()
{
    int req_num = m_myinput->m_next_req;
    long long tot_turnaround = 0;
    long long tot_waittime = 0;
    int max_waittime = 0;
    for(int i = 0; i < req_num; ++i)
    {
        Request* req = m_reqs[i];
        printf("%5d: %5d %5d %5d\n",i, req->m_arrive_time, req->m_start_time, req->m_end_time);
        tot_turnaround += req->m_end_time - req->m_arrive_time;
        int waittime = req->m_start_time - req->m_arrive_time;
        tot_waittime += waittime;
        if(waittime > max_waittime)
            max_waittime = waittime;  
    }
    double avg_turnaround = (double)tot_turnaround/req_num;
    double avg_waittime = (double)tot_waittime/req_num;
    printf("SUM: %d %d %.2lf %.2lf %d\n",\
    m_total_time, m_total_movement, avg_turnaround, avg_waittime, max_waittime);
}