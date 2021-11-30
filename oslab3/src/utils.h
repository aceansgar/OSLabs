#ifndef _UTILS_H
#define _UTILS_H
#include "myenv.h"

class MyConfig
{
public:
    unsigned m_frame_num;
    page_policy_t m_policy;
    bool m_output_option; // O
    bool m_page_table_option; // P
    bool m_frame_table_option; // F
    bool m_process_statistics_option; // S
    char m_infile_path[FILE_PATH_MAX_LEN];
    char m_randfile_path[FILE_PATH_MAX_LEN];
    MyConfig(int _argc, char** _argv);
};

class MyInput
{
public:
    FILE* m_infile;
    bool m_newline; // whether need a new line
    MyInput(char* _path);
    ~MyInput();
    char* read_token(); 
    unsigned read_unsigned(); // read process infos
    bool get_next_instruction(char* _op, unsigned* _val);
};

class MyRand
{
private:
    unsigned m_rand_num;
    unsigned m_rand_offset;
    unsigned m_modulo; // frame num
    std::vector<int> m_randvals;
public:
    // MyRand();
    MyRand(unsigned _modulo, char* _path);
    ~MyRand();
    unsigned myrandom();
};

#endif