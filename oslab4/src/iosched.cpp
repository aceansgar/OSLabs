#include "manager.h"
#include "utils.h"

int main(int argc, char** argv)
{
    assert(argc >= 3);
    char* input_path = argv[argc-1];
    MyInput* myinput = new MyInput(input_path);
    policy_t policy = get_policy(argc, argv);
    Simulator* simulator = new Simulator(myinput, policy);
    simulator->simulate();
    simulator->print_summary();
    delete simulator;
    delete myinput;
    return 0;
}