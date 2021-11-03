#include <cstdlib>
#include <cstdio>
#include <ctype.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string.h>
#include <regex>
#include <set>
#include <map>
// #define DEBUG_TOKEN_READER
// #define DEBUG_SYMBOL_TABLE

enum listtype {deflist, uselist, programtext};



class TokenReader
{
public:
    static int line_i_;
    static int offset_;
    static int line_len_;
    static char* input_file_name_;
    static std::ifstream* infile_;
    static bool newline_; // need new line
    static char line_[1000];
    // int cur_list_type_; 
    // int token_num_remain; 
    static void reset_reader();
    static char* read_token();
    static void close_ifstream();
};

class SymbolTable
{
public:
    static std::map<std::string, int> sym_table_; // sym to addr
    static std::map<std::string, int> sym_to_id_;
    static std::vector<std::string> sym_list_;
    static std::map<std::string, bool> sym_to_used_;
    static std::map<std::string, int> sym_to_module_;
    static std::set<std::string> multi_def_sym_set_;
    static int sym_num_;
    static void create_symbol(char* _sym, int _abs_addr, int _module_i);
    static void arrange_symbol_table();
    static void print_table();
    static int get_symbol_addr(std::string _symbol);
};

int read_int();
int token_to_int(char* _token_p);
char* read_symbol();
char read_IAER();
bool pair_cmp(std::pair<std::string, int> _pair_1, std::pair<std::string, int> _pair_2);
std::string int_to_3digit_str(int _num);

//pass1 stores symbol table
void pass1();
void pass2();

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        printf("invalid arg num\n");
        exit(0);
    }
    TokenReader::input_file_name_ = argv[1];
#ifdef DEBUG_TOKEN_READER
    TokenReader::reset_reader();
    while(1)
    {
        char* token_p = TokenReader::read_token();
        if(!token_p)
            break;
        printf("%s\n", token_p);
    }
#endif
    pass1();
    SymbolTable::print_table();
    pass2();
    return 0;
}

int TokenReader::line_i_ = 0;
int TokenReader::offset_ = 0;
int TokenReader::line_len_ = 0;
char* TokenReader::input_file_name_ = NULL;
std::ifstream* TokenReader::infile_ = NULL;
bool TokenReader::newline_ = true;
char TokenReader::line_[1000];

void TokenReader::reset_reader()
{
    if(!infile_)
        delete infile_;
    infile_ = new std::ifstream(input_file_name_);
    newline_ = true;
}

char* TokenReader::read_token()
{
    char* token_p = NULL;
    while(!token_p)
    {
        if(newline_)
        {
            if(!infile_->getline(line_, 1000))
            {
                //EOF
                offset_ = line_len_ + 1;
                return NULL;
            }
            ++ line_i_;
            line_len_ = (int)strlen(line_);
            newline_ = false;
            token_p = strtok(line_, " \n\t");
        }
        else
            token_p = strtok(NULL, " \n\t");
        if(!token_p)
            newline_ = true;
        offset_ = (int)(token_p - line_) + 1;
        
    }
    return token_p;
}

void TokenReader::close_ifstream()
{
    infile_->close();
}

std::map<std::string, int> SymbolTable::sym_table_ = std::map<std::string, int>();
std::map<std::string, int> SymbolTable::sym_to_id_ = std::map<std::string, int>();
std::vector<std::string> SymbolTable::sym_list_ = std::vector<std::string>();
std::map<std::string, bool> SymbolTable::sym_to_used_ = std::map<std::string, bool>();
std::map<std::string, int> SymbolTable::sym_to_module_ = std::map<std::string, int>();
std::set<std::string> SymbolTable::multi_def_sym_set_ = std::set<std::string>();
int SymbolTable::sym_num_ = 0;

void SymbolTable::create_symbol(char* _sym, int _abs_addr, int _module_i)
{
    std::string symbol = std::string(_sym);
    std::map<std::string, int>::iterator iter = sym_table_.find(symbol);
    if(iter != sym_table_.end())
    {
        // multi definition
        multi_def_sym_set_.insert(symbol);
    }
    else
    {
        sym_table_.insert(std::pair<std::string, int>(symbol, _abs_addr));
        sym_to_module_.insert(std::pair<std::string,int>(symbol, _module_i));
    }
}

void SymbolTable::arrange_symbol_table()
{
    sym_num_ = SymbolTable::sym_table_.size();
    std::vector<std::pair<std::string, int>> table_vec;
    for(std::map<std::string, int>::iterator iter = sym_table_.begin(); iter!= sym_table_.end(); ++iter)
    {
        table_vec.push_back(std::pair<std::string,int>(iter->first, iter->second));
    }
    std::sort(table_vec.begin(), table_vec.end(), pair_cmp);
    for(int i = 0; i < sym_num_; ++ i)
    {
        std::string symbol = table_vec[i].first;
        int addr = table_vec[i].second;
        sym_to_id_[symbol] = i;
        sym_list_.push_back(symbol);
        sym_to_used_[symbol] = false;
    }
}

void SymbolTable::print_table()
{
    printf("Symbol Table\n");
    for(int i = 0; i < sym_num_; ++ i)
    {
        std::string symbol = sym_list_[i];
        printf("%s=%d", sym_list_[i].c_str(), sym_table_[symbol]);
        if(multi_def_sym_set_.find(symbol) != multi_def_sym_set_.end())
            printf(" Error: This variable is multiple times defined; first value used");
        printf("\n");
    }
    
}

int SymbolTable::get_symbol_addr(std::string _symbol)
{
    return sym_table_[_symbol];
}

int read_int()
{
    char* token_p = TokenReader::read_token();
    return token_to_int(token_p);
}

int token_to_int(char* _token_p)
{
    if(!_token_p || !std::regex_match(_token_p, std::regex("[0-9]+")))
    {
        printf("Parse Error line %d offset %d: NUM_EXPECTED\n", TokenReader::line_i_, TokenReader::offset_);
        exit(0);
    }
    return atoi(_token_p);
}

char* read_symbol()
{
    char* token_p = TokenReader::read_token();
    if(!token_p || !std::regex_match(token_p, std::regex("[a-zA-Z][a-zA-Z0-9]*")))
    {
        // printf("error: %s is not a symbol\n", token_p);
        printf("Parse Error line %d offset %d: SYM_EXPECTED\n", TokenReader::line_i_, TokenReader::offset_);
        exit(0);
    }
    return token_p;
}

char read_IAER()
{
    // printf("reading IAER\n");
    char* token_p = TokenReader::read_token();
    // printf("a token read %s\n", token_p);
    if(!token_p || !std::regex_match(token_p, std::regex("I|A|E|R")))
    {
        printf("Parse Error line %d offset %d: ADDR_EXPECTED\n", TokenReader::line_i_, TokenReader::offset_);
        exit(0);
    }
    return token_p[0];
}

bool pair_cmp(std::pair<std::string, int> _pair_1, std::pair<std::string, int> _pair_2)
{
    return _pair_1.second < _pair_2.second;
}

std::string int_to_3digit_str(int _num)
{
    std::string str = "000";
    int x = _num;
    str[0] = x/100 + '0';
    x = x % 100;
    str[1] = x/10 + '0';
    x = x % 10;
    str[2] = x + '0';
    return str;
}

void pass1()
{
    TokenReader::reset_reader();
    int module_base_addr = 0;
    int module_i = 1;
    while(true)
    {
        char* token_p = TokenReader::read_token();
        if(!token_p)
        {
            // no token remained
            break;
        }
        int def_num = token_to_int(token_p);
        if(def_num > 16)
        {
            printf("Parse Error line %d offset %d: TOO_MANY_DEF_IN_MODULE\n", TokenReader::line_i_, TokenReader::offset_);
            exit(0);
        }
        std::vector<std::string> def_list;
        std::vector<int> def_rel_addr_list;
        for(int i = 0; i < def_num; ++i)
        {
            char* symbol_defined = read_symbol();
            int rel_addr = read_int();
            SymbolTable::create_symbol(symbol_defined, module_base_addr + rel_addr, module_i);
            def_list.push_back(std::string(symbol_defined));
            def_rel_addr_list.push_back(rel_addr);
        }
        int use_num = read_int();
        if(use_num > 16)
        {
            printf("Parse Error line %d offset %d: TOO_MANY_USE_IN_MODULE\n", TokenReader::line_i_, TokenReader::offset_);
            exit(0);
        }
        for(int i = 0; i < use_num; ++i)
        {
            char* symbol_used = read_symbol();
            // don't do anything here
        }
        int inst_num = read_int();
        if(inst_num + module_base_addr >= 512)
        {
            printf("Parse Error line %d offset %d: TOO_MANY_INSTR\n", TokenReader::line_i_, TokenReader::offset_);
            exit(0);
        }
        for(int i = 0; i < inst_num; ++i)
        {
            char inst_type = read_IAER();
            int instr = read_int();
        }
        // check if all symbols defined in this module are in the address limit
        for(int def_i = 0; def_i < def_num; ++ def_i)
        {
            std::string symbol = def_list[def_i];
            if(SymbolTable::multi_def_sym_set_.find(symbol) != SymbolTable::multi_def_sym_set_.end())
                continue;
            int rel_addr = def_rel_addr_list[def_i];
            if(rel_addr >= inst_num)
            {
                printf("Warning: Module %d: %s too big %d (max=%d) assume zero relative\n", module_i, symbol.c_str(), rel_addr, inst_num -1);
                SymbolTable::sym_table_[symbol] = module_base_addr;
            }
        }
        module_base_addr += inst_num;
        ++ module_i;
    }
    TokenReader::close_ifstream();
    SymbolTable::arrange_symbol_table();
}

void pass2()
{
    TokenReader::reset_reader();
    int module_base_addr = 0;
    printf("\nMemory Map\n");
    int module_i = 1;
    while(true)
    {
        char* token_p = TokenReader::read_token();
        if(!token_p)
        {
            // no token remained
            break;
        }
        int def_num = token_to_int(token_p);
        for(int i = 0; i < def_num; ++i)
        {
            char* symbol_defined = read_symbol();
            int rel_addr = read_int();
        }
        int use_num = read_int();
        std::string* used_list = new std::string[use_num];
        std::set<std::string> used_set;
        for(int i = 0; i < use_num; ++i)
        {
            char* symbol_used = read_symbol();
            std::string symbol = std::string(symbol_used);
            used_list[i] = symbol;
            SymbolTable::sym_to_used_[symbol] = true;
        }
        int inst_num = read_int();
        for(int i = 0; i < inst_num; ++i)
        {
            char inst_type = read_IAER();
            int inst = read_int();
            int new_inst;
            std::string inst_id = int_to_3digit_str(module_base_addr + i);
            int opcode = inst/1000;
            if(inst_type != 'I' && opcode >= 10)
            {
                printf("%s: 9999 Error: Illegal opcode; treated as 9999\n", inst_id.c_str());
                continue;
            }
            if(inst_type == 'I')
            {
                if(opcode >= 10)
                {
                    printf("%s: 9999 Error: Illegal immediate value; treated as 9999\n", inst_id.c_str());
                    continue;
                }
                new_inst = inst;
                printf("%s: %04d\n", inst_id.c_str(), new_inst);
            }
            else if(inst_type == 'A')
            {
                if(inst % 1000 >= 512)
                {
                    new_inst = (inst/1000)*1000;
                    printf("%s: %04d Error: Absolute address exceeds machine size; zero used\n", inst_id.c_str(), new_inst);
                    continue;
                }
                new_inst = inst;
                printf("%s: %04d\n", inst_id.c_str(), new_inst);
            }
            else if(inst_type == 'E')
            {
                int operand = inst%1000;
                if(operand >= use_num)
                {
                    printf("%s: %04d Error: External address exceeds length of uselist; treated as immediate\n", inst_id.c_str(), inst);
                    continue;
                }
                std::string symbol = used_list[operand];
                if(SymbolTable::sym_table_.find(symbol) == SymbolTable::sym_table_.end())
                {
                    printf("%s: %04d Error: %s is not defined; zero used\n", inst_id.c_str(), inst, symbol.c_str());
                    used_set.insert(symbol);
                    continue;
                }
                used_set.insert(symbol);
                int symbol_addr = SymbolTable::get_symbol_addr(symbol);
                new_inst = (inst/1000)*1000 + symbol_addr;
                printf("%s: %04d\n", inst_id.c_str(), new_inst);
            }
            else if(inst_type == 'R')
            {
                int rel_addr = inst % 1000;
                if(rel_addr >= inst_num)
                {
                    new_inst = (inst/1000)*1000 + module_base_addr;
                    printf("%s: %04d Error: Relative address exceeds module size; zero used\n", inst_id.c_str(), new_inst);
                    continue;
                }
                new_inst = inst + module_base_addr;
                printf("%s: %04d\n", inst_id.c_str(), new_inst);
            }
        }
        // check if all symbols in use list used
        for(int use_i = 0; use_i < use_num; ++ use_i)
        {
            std::string symbol = used_list[use_i];
            if(used_set.find(symbol) == used_set.end())
                printf("Warning: Module %d: %s appeared in the uselist but was not actually used\n", module_i, symbol.c_str());
        }
        module_base_addr += inst_num;
        delete []used_list;
        ++ module_i;
    }
    // check unused symbols
    printf("\n");
    for(int sym_i = 0; sym_i < SymbolTable::sym_num_; ++ sym_i)
    {
        std::string symbol = SymbolTable::sym_list_[sym_i];
        if(!SymbolTable::sym_to_used_[symbol])
        {
            int module_i = SymbolTable::sym_to_module_[symbol];
            printf("Warning: Module %d: %s was defined but never used\n", module_i, symbol.c_str());
        }
    }
    TokenReader::close_ifstream();
}