#include "utils.h"
#include "manager.h"

int main(int argc, char** argv)
{
    assert(argc >= 5);
    MyConfig* myconfig = new MyConfig(argc, argv);
    char* input_path = argv[argc-2];
    char* rand_path = argv[argc-1];
    MyInput* myinput = new MyInput(input_path);
    MyRand* myrand = new MyRand(myconfig->m_frame_num, rand_path);
    Simulator* simulator = new Simulator(myconfig, myinput, myrand);
    // printf("frame num: %u\n", myconfig->m_frame_num);
    // printf("call simulate\n");
    simulator->simulate();
    simulator->print_summary();
    delete simulator;
    delete myinput;
    delete myrand;
    delete myconfig;
    return 0;
}