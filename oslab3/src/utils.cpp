#include "utils.h"

MyConfig::MyConfig(int _argc, char** _argv)
{
    m_output_option = false; // O
    m_page_table_option = false; // P
    m_frame_table_option = false; // F
    m_process_statistics_option = false; // S
    char c;
    while ((c = getopt(_argc,_argv,"f:a:o:")) != -1 )
	{
        // printf("opt get : %c\n", c);
		switch(c) 
        {
            case 'f': // frame num
            {
                // printf("case f, optarg: %s", optarg);
                m_frame_num = atoi(optarg);
                break;
            }
            case 'a': // replacement policy
            {
                // printf("optarg: %s\n", optarg);
                switch(optarg[0])
                {
                    case 'f':
                        m_policy = FIFO;
                        break;
                    case 'r':
                        m_policy = RANDOM;
                        break;
                    case 'c':
                        m_policy = CLOCK;
                        break;
                    case 'e':
                        m_policy = NRU;
                        break;
                    case 'a':
                        m_policy = AGING;
                        break;
                    case 'w':
                        m_policy = WORKINGSET;
                        break;
                    default:
                        fprintf(stderr, "invalid -a option\n");
                        break;
                }
                break;
            }
            case 'o':
            {
                int opt_num = strlen(optarg);
                for(int i = 0; i < opt_num; ++ i)
                {
                    switch(optarg[i])
                    {
                        case 'O':
                            m_output_option = true;
                            break;
                        case 'P':
                            m_page_table_option = true;
                            break;
                        case 'F':
                            m_frame_table_option = true;
                            break;
                        case 'S':
                            m_process_statistics_option = true;
                        default:
                            break;
                    }
                }
                break;
            }
            default:
                break;
		}
	}

}


MyInput::MyInput(char* _path)
{
    m_infile = fopen(_path, "r");
    m_newline = true;
}

MyInput::~MyInput()
{
    fclose(m_infile);
}

char* MyInput::read_token()
{
    char* token_p = NULL;
    char line[1000];
    while(!token_p)
    {
        if(m_newline)
        {
            if(!fgets(line, 1000, m_infile))
            {
                //EOF
                return NULL;
            }
            if(line[0] == '#')
                continue;
            token_p = strtok(line, " \n\t");
            m_newline = false;
        }
        else
            token_p = strtok(NULL, " \n\t");
        if(!token_p)
            m_newline = true;      
    }
    // printf("a token read: %s\n",token_p);
    return token_p;
}

unsigned MyInput::read_unsigned()
{
    char* token = read_token();
    return atoi(token);
}

bool MyInput::get_next_instruction(char* _op, unsigned* _val)
{
    char* token = read_token();
    if(!token)
    {
        // EOF
        return false;
    }
    *_op = token[0];
    token = read_token();
    *_val = atoi(token);
    return true;
}

MyRand::MyRand(unsigned _modulo, char* _path)
{
    m_modulo = _modulo;
    m_rand_offset = 0;
    std::string line;
    std::ifstream rfile(_path);
    getline(rfile,line); // total number
    m_rand_num = atoi(line.c_str());
    while(getline(rfile, line))
    {
        m_randvals.push_back(atoi(line.c_str()));
    }
}

MyRand::~MyRand()
{

}

unsigned MyRand::myrandom() 
{ 
    unsigned res = m_randvals[m_rand_offset] % m_modulo; 
    m_rand_offset = (m_rand_offset + 1) % m_rand_num;
    return res;
}
