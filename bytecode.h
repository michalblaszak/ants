#ifndef BYTECODE_H
#define BYTECODE_H

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <string.h>
#include <errno.h>

#include "array.h"

enum EInstrCodes {
    NOP             = 0,    // No attributes
    DATA            = 1,    // Datatype (unsigned char array (zero-terminated): 's', 'i', 'f', 'b')
    PUTADDR         = 2,    // Variable index from the DATA section (1 unsigned int - 4 bytes) 
    PUTINDADDR      = 3,    // unsigned char (1 byte) - the number of indexes. Takes indexes from the stack, takes an array address from the stack and puts the address of the resulting variable to the stack
    PUTMEMBERADDR   = 4,    // Variable index from the class DATA section (1 ushort - 2 bytes). Takes the variable address from the stack and looks for its member, puts the resulting address to the stack 
    PUTINT          = 5,    // Int literal (8 bytes: long long int)
    PUTFLOAT        = 6,    // Float literal (12 bytes: long double)
    PUTSTRING       = 7,    // String literal (the array of char + ending 0)
    PUTBOOLEAN      = 8,    // boolean literal (1 byte: 0 - false, 1 - true)
    MOVE            = 9,    // No attributes
    MOVEADD         = 10,   // No attributes
    MOVESUBTR       = 11,   // No attributes
    MOVEMUL         = 12,   // No attributes
    MOVEDIV         = 13,   // No attributes
    EQUAL           = 14,   // No attributes
    NOTEQUAL        = 15,   // No attributes
    LESSEQUAL       = 16,   // No attributes
    GREATEREQUAL    = 17,   // No attributes
    LESS            = 18,   // No attributes
    GREATER         = 19,   // No attributes
    JUMPIFFALSE     = 20,   // Instruction address (uint - 4 bytes)
    JUMP            = 21,   // Instruction address (uint - 4 bytes)
    MUL             = 22,   // No attributes
    DIV             = 23,   // No Attributes
    ADD             = 24,   // No attributes
    SUB             = 25,   // No attributes
    NEG             = 26,   // No attributes
    END             = 27,   // No attributes
    CALL            = 28,   // Function index from the DATA section (1 unsigned int - 4 bytes) 
    FUN             = 29,   // Function address in the code (1 unsigned int - 4 bytes), Datatype (unsigned char array (zero-terminated): 's', 'i', 'f', 'b')
    SYSCALL         = 30,   // The parameter idx, The system function name: String literal (the array of char + ending 0)
    RETURN          = 31,   // The DATA index for the FUNCTION variable holding the returned value; Gets the function variable value and puts to the stack.
                            // Get's off the callback stack's top and releases all internal resources from the function's scope

    INITVAR         = 32,   // Variable index from the DATA section (1 unsigned int - 4 bytes) - initializes the variable to its default value
    PUTDADDR        = 33,   // Variable index from the DATA section (1 unsigned int - 4 bytes) - calculates the dynamic variable address.
                            // The DATA section variable contains the index of the variable which is on the current call-stack
    ALLOCVAR        = 34,   // Variable index from the DATA section (1 unsigned int - 4 bytes) - allocates the variable on the call-stack.
                            // The datatype of the variable is taken from the DATA section variable of the provided index
    DDATA           = 35,   // Variable index from the DATA section (1 unsigned int - 4 bytes) - variable instance index on the call-stack; 
                            // Datatype (unsigned char array (zero-terminated): 's', 'i', 'f', 'b')
    ALLOCVARS       = 36,   // Variable index from the DATA section (1 unsigned int - 4 bytes) - allocates the variable on the call-stack and initializes with the value taken from the stack.
                            // The datatype of the variable is taken from the DATA section variable of the provided index
                            // Used allocating and initializing function parameters. This statement must be provided in a sequence revert to the actual parameter values provided to the stack. Variables created on the call-stack have to be created in one-more-time reverted order.
};

class Datatype
{
public:
    enum class EVariableTypes { UNDEFINED, VARIABLE, DYNAMIC_VARIABLE, FUNCTION };
private:
    enum EVariableTypes variableType;
    char* datatype;
    char* funParamDatatype;
    void* address;
    int function_ref; // function address in the code
    unsigned int dynamic_idx; // dynamic variable index on the call-stack
    unsigned int callstack_pos; // the position of the dynamically allocated instance of this variable in the callstack entry
    char* name;
    char* scope;

public:
    Datatype();
    Datatype(const char* name, const char* scope, const char* datatype, EVariableTypes variableType);
    Datatype(const char* name, const char* scope, const char* datatype, unsigned int pos, EVariableTypes variableType);
    Datatype(const char* name, const char* scope, const char* datatype, const char* funParamDatatype, EVariableTypes variableType, int function_ref);
    Datatype(const Datatype& other);
    ~Datatype();
    bool operator==(const Datatype& other) const;
    void setName(const char* name);
    void setScope(const char* scope);
    void setDatatype(const char* datatype);
    void setFunParamDatatype(const char* funParamDatatype);
    const char* getName() const;
    const char* getScope() const;
    const char* getDatatype() const;
    const char* getFunParamDatatype() const;
    enum EVariableTypes getVariableType() const;
    void* makeAddress();
    void* makeDynamicAddress();
    void* getAddress();
    void disposeAddress();
    void printVariable();
    unsigned int getFunRef() const;
    unsigned int getDynamicIdx() const;
    void setToDefault();
    void setCallStackPos(unsigned int callstack_pos);
    unsigned int getCallStackPos();
};

struct FunctionRef
{
    unsigned int variable_index;
    unsigned int function_pos;

    FunctionRef(unsigned int variable_index, unsigned int function_pos) : variable_index(variable_index), function_pos(function_pos) {
    }
};

class Bytecode
{
private:
    std::vector<Datatype> variables;
    std::vector<unsigned char> code;
    std::vector<unsigned int> jumps;
    std::vector<FunctionRef> function_refs;

public:
    Bytecode();
    void clear() {
        variables.clear();
        code.clear();
        jumps.clear();
        function_refs.clear();
    }
    
    std::vector<unsigned char>& getCode() { return code; }
    std::vector<Datatype>& getVariables() { return variables; }

    Bytecode& operator+=(Bytecode& other) {
        std::vector<unsigned int>::iterator i_jumps;
        std::vector<FunctionRef>::iterator i_funs;
        unsigned int addr;
        unsigned char* paddr = (unsigned char*)&addr;
        unsigned int code_size = code.size();
        
        // Adjust addresses
        for (i_jumps = other.jumps.begin(); i_jumps < other.jumps.end(); i_jumps++) {
            unsigned int j = *i_jumps;

            jumps.push_back(j+code_size);  // Add updated jump addresses

            *paddr = other.code[j];
            *(paddr+1) = other.code[j+1];
            *(paddr+2) = other.code[j+2];
            *(paddr+3) = other.code[j+3];

            addr += code_size;

            other.code[j] = *paddr;
            other.code[j+1] = *(paddr+1);
            other.code[j+2] = *(paddr+2);
            other.code[j+3] = *(paddr+3);
        }

        for (i_funs = other.function_refs.begin(); i_funs < other.function_refs.end(); i_funs++) {
            function_refs.push_back(FunctionRef(i_funs->variable_index, i_funs->function_pos+code_size));  // Add updated function reference
        }

        // Move code
        code.insert(
            code.end(),
            std::make_move_iterator(other.code.begin()),
            std::make_move_iterator(other.code.end())
        );

        return *this;
    }

    void addJump(unsigned int addr) {
        jumps.push_back(addr);
    }

    void addFunction(unsigned int fun_var_pos, unsigned int fun_code_pos) {
        function_refs.push_back(FunctionRef(fun_var_pos, fun_code_pos));
    }

    const std::vector<FunctionRef>& getFunctionRefs() const {
        return function_refs;
    }

    void setAddress(unsigned int pos, unsigned int val) { 
        void* addr = (void*)&val;
        code[pos]   = ( *((unsigned char*)addr) );
        code[pos+1] = ( *((unsigned char*)addr+1) );
        code[pos+2] = ( *((unsigned char*)addr+2) );
        code[pos+3] = ( *((unsigned char*)addr+3) );
    }
    unsigned int DATA(const char* name, const char* scope, const char* datatype) {
        variables.push_back(Datatype(name, scope, datatype, Datatype::EVariableTypes::VARIABLE));
        return variables.size()-1;
    }
    unsigned int DDATA(const char* name, const char* scope, unsigned int pos, const char* datatype) {
        variables.push_back(Datatype(name, scope, datatype, pos, Datatype::EVariableTypes::DYNAMIC_VARIABLE));
        return variables.size()-1;
    }
    unsigned int INITVAR(unsigned int index) {
        code.push_back(EInstrCodes::INITVAR);
        unsigned int pos = code.size()-1;
        void* addr = (void*)&index;
        code.push_back( *((unsigned char*)addr) );
        code.push_back( *((unsigned char*)addr+1) );
        code.push_back( *((unsigned char*)addr+2) );
        code.push_back( *((unsigned char*)addr+3) );
        return pos;
    }
    unsigned int ALLOCVAR(unsigned int index) {
        code.push_back(EInstrCodes::ALLOCVAR);
        unsigned int pos = code.size()-1;
        void* addr = (void*)&index;
        code.push_back( *((unsigned char*)addr) );
        code.push_back( *((unsigned char*)addr+1) );
        code.push_back( *((unsigned char*)addr+2) );
        code.push_back( *((unsigned char*)addr+3) );
        return pos;
    }
    unsigned int ALLOCVARS(unsigned int index) {
        code.push_back(EInstrCodes::ALLOCVARS);
        unsigned int pos = code.size()-1;
        void* addr = (void*)&index;
        code.push_back( *((unsigned char*)addr) );
        code.push_back( *((unsigned char*)addr+1) );
        code.push_back( *((unsigned char*)addr+2) );
        code.push_back( *((unsigned char*)addr+3) );
        return pos;
    }
    unsigned int FUN(const char* name, const char* scope, const char* datatype, const char* funParamDatatype, int function_ref) {
        variables.push_back(Datatype(name, scope, datatype, funParamDatatype, Datatype::EVariableTypes::FUNCTION, function_ref));

        return variables.size()-1;
    }
    unsigned int NOP() { code.push_back(EInstrCodes::NOP); return code.size()-1; }
    unsigned int PUTADDR(unsigned int index) { 
        code.push_back(EInstrCodes::PUTADDR);
        unsigned int pos = code.size()-1;
        void* addr = (void*)&index;
        code.push_back( *((unsigned char*)addr) );
        code.push_back( *((unsigned char*)addr+1) );
        code.push_back( *((unsigned char*)addr+2) );
        code.push_back( *((unsigned char*)addr+3) );
        return pos;
    }
    unsigned int PUTDADDR(unsigned int index) { 
        code.push_back(EInstrCodes::PUTDADDR);
        unsigned int pos = code.size()-1;
        void* addr = (void*)&index;
        code.push_back( *((unsigned char*)addr) );
        code.push_back( *((unsigned char*)addr+1) );
        code.push_back( *((unsigned char*)addr+2) );
        code.push_back( *((unsigned char*)addr+3) );
        return pos;
    }
    unsigned int CALL(unsigned int index) { 
        code.push_back(EInstrCodes::CALL);
        unsigned int pos = code.size()-1;
        void* addr = (void*)&index;
        code.push_back( *((unsigned char*)addr) );
        code.push_back( *((unsigned char*)addr+1) );
        code.push_back( *((unsigned char*)addr+2) );
        code.push_back( *((unsigned char*)addr+3) );
        return pos;
    }
    unsigned int PUTINDADDR(unsigned char index_count) {
        code.push_back(EInstrCodes::PUTINDADDR);
        unsigned int pos = code.size()-1;
        void* addr = (void*)&index_count;
        code.push_back( *((unsigned char*)addr) );
        return pos;
    }
    // TODO: needs to be revised
    unsigned int PUTMEMBERADDR(unsigned int index) {
        code.push_back(EInstrCodes::PUTMEMBERADDR);
        unsigned int pos = code.size()-1;
        void* addr = (void*)&index;
        code.push_back( *((unsigned char*)addr) );
        code.push_back( *((unsigned char*)addr+1) );
        code.push_back( *((unsigned char*)addr+2) );
        code.push_back( *((unsigned char*)addr+3) );
        return pos;
    }
    unsigned int PUTINT(long long int val) {
        code.push_back(EInstrCodes::PUTINT);
        unsigned int pos = code.size()-1;
        void* addr = (void*)&val;
        code.push_back( *((unsigned char*)addr) );
        code.push_back( *((unsigned char*)addr+1) );
        code.push_back( *((unsigned char*)addr+2) );
        code.push_back( *((unsigned char*)addr+3) );
        code.push_back( *((unsigned char*)addr+4) );
        code.push_back( *((unsigned char*)addr+5) );
        code.push_back( *((unsigned char*)addr+6) );
        code.push_back( *((unsigned char*)addr+7) );
        return pos;
    }
    unsigned int PUTFLOAT(long double val) {
        code.push_back(EInstrCodes::PUTFLOAT);
        unsigned int pos = code.size()-1;
        void* addr = (void*)&val;
        code.push_back( *((unsigned char*)addr) );
        code.push_back( *((unsigned char*)addr+1) );
        code.push_back( *((unsigned char*)addr+2) );
        code.push_back( *((unsigned char*)addr+3) );
        code.push_back( *((unsigned char*)addr+4) );
        code.push_back( *((unsigned char*)addr+5) );
        code.push_back( *((unsigned char*)addr+6) );
        code.push_back( *((unsigned char*)addr+7) );
        code.push_back( *((unsigned char*)addr+8) );
        code.push_back( *((unsigned char*)addr+9) );
        code.push_back( *((unsigned char*)addr+10) );
        code.push_back( *((unsigned char*)addr+11) );
        return pos;
    }
    // val: unicode bytearray
    unsigned int PUTSTRING(const char* val) {
        code.push_back(EInstrCodes::PUTSTRING);
        unsigned int pos = code.size()-1;
        int i = 0;
        do {
            code.push_back( val[i] );
        } while(val[i++] != 0);
        return pos;
    }
    unsigned int PUTBOOLEAN(bool val) {
        code.push_back(EInstrCodes::PUTBOOLEAN);
        unsigned int pos = code.size()-1;
        code.push_back(val ? (unsigned char)1 : (unsigned char)0);
        return pos;
    }
    unsigned int MOVE() { code.push_back(EInstrCodes::MOVE); return code.size()-1; }
    unsigned int MOVEADD() { code.push_back(EInstrCodes::MOVEADD); return code.size()-1; }
    unsigned int MOVESUBTR() { code.push_back(EInstrCodes::MOVESUBTR); return code.size()-1; }
    unsigned int MOVEMUL() { code.push_back(EInstrCodes::MOVEMUL); return code.size()-1; }
    unsigned int MOVEDIV() { code.push_back(EInstrCodes::MOVEDIV); return code.size()-1; }
    unsigned int EQUAL() { code.push_back(EInstrCodes::EQUAL); return code.size()-1; }
    unsigned int NOTEQUAL() { code.push_back(EInstrCodes::NOTEQUAL); return code.size()-1; }
    unsigned int LESSEQUAL() { code.push_back(EInstrCodes::LESSEQUAL); return code.size()-1; }
    unsigned int GREATEREQUAL() { code.push_back(EInstrCodes::GREATEREQUAL); return code.size()-1; }
    unsigned int LESS() { code.push_back(EInstrCodes::LESS); return code.size()-1; }
    unsigned int GREATER() { code.push_back(EInstrCodes::GREATER); return code.size()-1; }
    unsigned int JUMPIFFALSE(unsigned int val) { 
        code.push_back(EInstrCodes::JUMPIFFALSE);
        unsigned int pos = code.size()-1;
        void* addr = (void*)&val;
        code.push_back( *((unsigned char*)addr) );
        code.push_back( *((unsigned char*)addr+1) );
        code.push_back( *((unsigned char*)addr+2) );
        code.push_back( *((unsigned char*)addr+3) );
        return pos;
    }
    unsigned int JUMP(unsigned int val) { 
        code.push_back(EInstrCodes::JUMP);
        unsigned int pos = code.size()-1;
        void* addr = (void*)&val;
        code.push_back( *((unsigned char*)addr) );
        code.push_back( *((unsigned char*)addr+1) );
        code.push_back( *((unsigned char*)addr+2) );
        code.push_back( *((unsigned char*)addr+3) );
        return pos;
    }
    unsigned int MUL() { code.push_back(EInstrCodes::MUL); return code.size()-1; }
    unsigned int DIV() { code.push_back(EInstrCodes::DIV); return code.size()-1; }
    unsigned int ADD() { code.push_back(EInstrCodes::ADD); return code.size()-1; }
    unsigned int SUB() { code.push_back(EInstrCodes::SUB); return code.size()-1; }
    unsigned int NEG() { code.push_back(EInstrCodes::NEG); return code.size()-1; }
    unsigned int END() { code.push_back(EInstrCodes::END); return code.size()-1; }
    // val: unicode bytearray
    unsigned int SYSCALL(unsigned int var_idx, const char* val) {
        code.push_back(EInstrCodes::SYSCALL);
        unsigned int pos = code.size()-1;
        void* addr = (void*)&var_idx;
        code.push_back( *((unsigned char*)addr) );
        code.push_back( *((unsigned char*)addr+1) );
        code.push_back( *((unsigned char*)addr+2) );
        code.push_back( *((unsigned char*)addr+3) );

        // Store function name
        int i = 0;
        do {
            code.push_back( val[i] );
        } while(val[i++] != 0);
        return pos;
    }
    unsigned int RETURN(unsigned int fun_var_idx) { 
        code.push_back(EInstrCodes::RETURN);
        
        unsigned int pos = code.size()-1;
        void* addr = (void*)&fun_var_idx;
        code.push_back( *((unsigned char*)addr) );
        code.push_back( *((unsigned char*)addr+1) );
        code.push_back( *((unsigned char*)addr+2) );
        code.push_back( *((unsigned char*)addr+3) );
        return pos;
    }

    bool print(const std::string& filename) {
        std::ofstream s;

        std::map<unsigned int, unsigned int> functions; // Contains the list of function addresses <function code idx, function variable idx>

        unsigned char c;
        s.open(filename, std::ofstream::out | std::ofstream::trunc);
        if (!s.is_open()) {
            char errmsg[201];
            strerror_s(errmsg, 200, errno);

            std::cout << "Error opening " << filename << ": " << errmsg << std::endl;
            return false;
        }

        // Generate the DATA section
        for(unsigned int i = 0; i<variables.size(); i++) {
            const Datatype& v = variables.at(i);

            if (v.getVariableType() == Datatype::EVariableTypes::VARIABLE) {
                s << i << " DATA " << v.getDatatype() << "\t\t; " << v.getScope() << "." << v.getName() << std::endl;
            } else if (v.getVariableType() == Datatype::EVariableTypes::DYNAMIC_VARIABLE) {
                s << i << " DDATA " << v.getDynamicIdx() << " " << v.getDatatype() << "\t\t; " << v.getScope() << "." << v.getName() << std::endl;
            } else if (v.getVariableType() == Datatype::EVariableTypes::FUNCTION) {
                functions[v.getFunRef()] = i;
                s << i << " FUN " << v.getFunRef() << " " << v.getDatatype() << ":" << v.getFunParamDatatype() << "\t\t; " << v.getScope() << "." << v.getName() << std::endl;
            }
        }

        // Generate the CODE section
        for(unsigned int i = 0; i<code.size(); i++) {
            std::string s_fun_name;

            auto fun_map_iterator { functions.find(i) };

            if (fun_map_iterator != std::end(functions)) {
                const Datatype& v = variables.at(fun_map_iterator->second);
                s << "; " << v.getScope() << "." << v.getName() << "(" << v.getDatatype() << ")" << std::endl;
            }

            c = code[i];

            s << i << " ";

            switch(c) {
                case EInstrCodes::NOP: s << "NOP" << std::endl; break;
                case EInstrCodes::DATA: break; // It's handled separately
                case EInstrCodes::DDATA: break; // It's handled separately
                case EInstrCodes::FUN: break; // It's handled separately
                case EInstrCodes::INITVAR: {
                    s << "INITVAR ";
                    unsigned int addr;
                    unsigned char* paddr = (unsigned char*)&addr;
                    *paddr = code[++i];
                    *(paddr+1) = code[++i];
                    *(paddr+2) = code[++i];
                    *(paddr+3) = code[++i];

                    const Datatype& v = variables.at(addr);

                    s << addr << "\t\t; " << v.getScope() << "." << v.getName() << std::endl;
                    break;
                }
                case EInstrCodes::ALLOCVAR: {
                    s << "ALLOCVAR ";
                    unsigned int addr;
                    unsigned char* paddr = (unsigned char*)&addr;
                    *paddr = code[++i];
                    *(paddr+1) = code[++i];
                    *(paddr+2) = code[++i];
                    *(paddr+3) = code[++i];

                    const Datatype& v = variables.at(addr);
                    
                    s << addr << "\t\t; " << v.getScope() << "." << v.getName() << std::endl;
                    break;
                }
                case EInstrCodes::ALLOCVARS: {
                    s << "ALLOCVARS ";
                    unsigned int addr;
                    unsigned char* paddr = (unsigned char*)&addr;
                    *paddr = code[++i];
                    *(paddr+1) = code[++i];
                    *(paddr+2) = code[++i];
                    *(paddr+3) = code[++i];

                    const Datatype& v = variables.at(addr);
                    
                    s << addr << "\t\t; " << v.getScope() << "." << v.getName() << std::endl;
                    break;
                }
                case EInstrCodes::PUTADDR: {
                    s << "PUTADDR ";
                    unsigned int addr;
                    unsigned char* paddr = (unsigned char*)&addr;
                    *paddr = code[++i];
                    *(paddr+1) = code[++i];
                    *(paddr+2) = code[++i];
                    *(paddr+3) = code[++i];

                    const Datatype& v = variables.at(addr);
                    
                    s << addr << "\t\t; " << v.getScope() << "." << v.getName() << std::endl;
                    break;
                }
                case EInstrCodes::PUTDADDR: {
                    s << "PUTDADDR ";
                    unsigned int addr;
                    unsigned char* paddr = (unsigned char*)&addr;
                    *paddr = code[++i];
                    *(paddr+1) = code[++i];
                    *(paddr+2) = code[++i];
                    *(paddr+3) = code[++i];

                    const Datatype& v = variables.at(addr);
                    
                    s << addr << "\t\t; " << v.getScope() << "." << v.getName() << std::endl;
                    break;
                }
                case EInstrCodes::CALL: {
                    s << "CALL ";
                    unsigned int addr;
                    unsigned char* paddr = (unsigned char*)&addr;
                    *paddr = code[++i];
                    *(paddr+1) = code[++i];
                    *(paddr+2) = code[++i];
                    *(paddr+3) = code[++i];

                    const Datatype& v = variables.at(addr);
                    
                    s << addr << "\t\t; " << v.getScope() << "." << v.getName() << std::endl;
                    break;
                }
                case EInstrCodes::PUTINDADDR: {
                    s << "PUTINDADDR " << (unsigned char)code[++i] << std::endl;
                    break;
                }
                case EInstrCodes::PUTMEMBERADDR: break; // TODO
                case EInstrCodes::PUTINT: {
                    s << "PUTINT ";
                    long long int val;
                    unsigned char* pval = (unsigned char*)&val;
                    *pval = code[++i];
                    *(pval+1) = code[++i];
                    *(pval+2) = code[++i];
                    *(pval+3) = code[++i];
                    *(pval+4) = code[++i];
                    *(pval+5) = code[++i];
                    *(pval+6) = code[++i];
                    *(pval+7) = code[++i];

                    s << val << std::endl;
                    break;
                }
                case EInstrCodes::PUTFLOAT: {
                    s << "PUTFLOAT ";
                    long double val;
                    unsigned char* pval = (unsigned char*)&val;
                    *pval = code[++i];
                    *(pval+1) = code[++i];
                    *(pval+2) = code[++i];
                    *(pval+3) = code[++i];
                    *(pval+4) = code[++i];
                    *(pval+5) = code[++i];
                    *(pval+6) = code[++i];
                    *(pval+7) = code[++i];
                    *(pval+8) = code[++i];
                    *(pval+9) = code[++i];
                    *(pval+10) = code[++i];
                    *(pval+11) = code[++i];

                    std::string std_string = std::to_string(val);
                    s << std_string.c_str() << std::endl;
                    break;
                }
                case EInstrCodes::PUTSTRING: {
                    s << "PUTSTRING ";

                    unsigned char c;
                    std::string str;

                    do {
                        c = code[++i];

                        if (c == 0) {
                            break;
                        }

                        str += c;
                    } while(true);

                    s << str << std::endl;
                    break;
                }
                case EInstrCodes::PUTBOOLEAN: {
                    s << "PUTBOOLEAN ";
                    unsigned char val = code[++i];

                    s << (val ? "true" : "false") << std::endl;
                    break;
                }
                case EInstrCodes::MOVE: { s << "MOVE" << std::endl; break; }
                case EInstrCodes::MOVEADD: { s << "MOVEADD" << std::endl; break; }
                case EInstrCodes::MOVESUBTR: { s << "MOVESUBTR" << std::endl; break; }
                case EInstrCodes::MOVEMUL: { s << "MOVEMU" << std::endl; break; }
                case EInstrCodes::MOVEDIV: { s << "MOVEDIV" << std::endl; break; }
                case EInstrCodes::EQUAL: { s << "EQUA" << std::endl; break; }
                case EInstrCodes::NOTEQUAL: { s << "NOTEQUA" << std::endl; break; }
                case EInstrCodes::LESSEQUAL: { s << "LESSEQUA" << std::endl; break; }
                case EInstrCodes::GREATEREQUAL: { s << "GREATEREQUA" << std::endl; break; }
                case EInstrCodes::LESS: { s << "LESS" << std::endl; break; }
                case EInstrCodes::GREATER: { s << "GREATER" << std::endl; break; }
                case EInstrCodes::JUMPIFFALSE: {
                    s << "JUMPIFFALSE ";
                    unsigned int addr;
                    unsigned char* paddr = (unsigned char*)&addr;
                    *paddr = code[++i];
                    *(paddr+1) = code[++i];
                    *(paddr+2) = code[++i];
                    *(paddr+3) = code[++i];

                    s << addr << std::endl;
                    break;
                }
                case EInstrCodes::JUMP: {
                    s << "JUMP ";
                    unsigned int addr;
                    unsigned char* paddr = (unsigned char*)&addr;
                    *paddr = code[++i];
                    *(paddr+1) = code[++i];
                    *(paddr+2) = code[++i];
                    *(paddr+3) = code[++i];

                    s << addr << std::endl;
                    break;
                }
                case EInstrCodes::MUL: { s << "MU" << std::endl; break; }
                case EInstrCodes::DIV: { s << "DIV" << std::endl; break; }
                case EInstrCodes::ADD: { s << "ADD" << std::endl; break; }
                case EInstrCodes::SUB: { s << "SUB" << std::endl; break; }
                case EInstrCodes::NEG: { s << "NEG" << std::endl; break; }
                case EInstrCodes::END: { s << "END" << std::endl; break; }
                case EInstrCodes::SYSCALL: {
                    s << "SYSCALL ";

                    // Parameter variable idx
                    unsigned int addr;
                    unsigned char* paddr = (unsigned char*)&addr;
                    *paddr = code[++i];
                    *(paddr+1) = code[++i];
                    *(paddr+2) = code[++i];
                    *(paddr+3) = code[++i];

                    s << addr << " ";

                    // Function name
                    unsigned char c;
                    std::string str;

                    do {
                        c = code[++i];

                        if (c == 0) {
                            break;
                        }

                        str += c;
                    } while (true);

                    s << str << std::endl;
                    break;
                }
                case EInstrCodes::RETURN: {
                    s << "RETURN ";
                    
                    unsigned int addr;
                    unsigned char* paddr = (unsigned char*)&addr;
                    *paddr = code[++i];
                    *(paddr+1) = code[++i];
                    *(paddr+2) = code[++i];
                    *(paddr+3) = code[++i];

                    s << addr << std::endl;
                    break; 
                }
            } // ~switch
        } // ~for

        s.close();

        return true;
    }

    void makeAddresses()
    {
        for(int i=0; i< variables.size(); i++) {
            if (variables[i].getVariableType() == Datatype::EVariableTypes::VARIABLE) {
                variables[i].makeAddress();
            }
        }
    }

    void disposeAddresses()
    {
        for(int i=0; i< variables.size(); i++) {
            if (variables[i].getVariableType() == Datatype::EVariableTypes::VARIABLE) {
                variables[i].disposeAddress();
            }
        }
    }

    void printVariables()
    {
        for(int i=0; i< variables.size(); i++) {
            printf("%d ", i);
            variables[i].printVariable();
        }
    }
};

#endif // BYTECODE_H
