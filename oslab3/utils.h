#ifndef _UTILS_H
#define _UTILS_H
#include "myenv.h"

void read_schedule_config(int _argc, char** _argv);
void read_rfile(char* _path);
void read_inputfile(char* _path);
int myrandom(int _burst);

#endif