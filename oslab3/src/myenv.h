#ifndef _MYENV_H
#define _MYENV_H

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <vector>
#include <queue>
#include <list>
#include <stack>
#include <fstream>
#include <iostream>
#include <assert.h>

// page number of each process is 64
#define PAGE_TABLE_SIZE 64
#define PAGE_TABLE_SIZE_LOG2 6
// maximum frame number supported is 128
#define MAX_FRAME_NUM 128
#define MAX_FRAME_NUM_LOG2 7
#define MAX_PROCESS_NUM 10
#define MAX_PROCESS_NUM_LOG2 4
#define MAX_VMA_NUM 8
#define READ_WRITE_COST 1
#define CONTEXT_SWITCH_COST 130
#define PROCESS_EXIT_COST 1250
#define MAP_COST 300
#define UNMAP_COST 400
#define IN_COST 3100
#define OUT_COST 2700
#define FIN_COST 2800
#define FOUT_COST 2400
#define ZERO_COST 140
#define SEGV_COST 340
#define SEGPROT_COST 420
#define FILE_PATH_MAX_LEN 200

enum page_policy_t {FIFO, RANDOM, CLOCK, NRU, AGING, WORKINGSET};

#endif