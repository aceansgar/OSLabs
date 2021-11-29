#include "utils.h"

#define DEFAULT_MAXPRIO 4
#define DEFAULT_QUANTUM 100000
#define DEBUG



int main(int argc, char** argv)
{
    read_options(argc, argv);
    char* input_path = argv[argc-2];
    char* rand_path = argv[argc-1];
    read_rfile(rand_path);
    read_inputfile(input_path);
    return 0;
}