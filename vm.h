#ifndef VM_H
#define VM_H

#include <stack>
#include "bytecode.h"

enum class EDataTypes {
    UNKNOWN = 0, INT, FLOAT, STRING, BOOLEAN, ADDRESS, ARRAY
};

extern char datatypes_string[];

class Element
{
private:
    EDataTypes datatype;
    union UValues {
        long long int intVal;
        long double floatVal;
        std::string stringVal;
        bool booleanVal;
        Array arrayVal;
        struct SAddr {
            EDataTypes datatype;
            void* addr;

            SAddr(EDataTypes datatype, void* addr) : datatype(datatype), addr(addr) {}
        } addrVal;

        UValues() {}
        UValues(long long int v) : intVal(v) {}
        UValues(long double v) : floatVal(v) {}
        UValues(const std::string& v) : stringVal(v) {}
        UValues(bool v) : booleanVal(v) {}
        UValues(const Array& v) : arrayVal(v) {}
        UValues(EDataTypes datatype, void* v) : addrVal(datatype, v) {}
        ~UValues() {}
    } value;

public:
    Element();
    Element(long long int val);
    Element(long double val);
    Element(const std::string& val);
    Element(bool val);
    Element(const Array& v);
    Element(EDataTypes datatype, void* val);
    Element(const Element& other);

    Element& operator=(const Element& other);

    ~Element();

    EDataTypes getDatatype();
    EDataTypes getFinalDatatype() const;

    char getDatatypeString() const;
    char getFinalDatatypeString() const;

    const union UValues& getValue();
    void* getVariablePhysicalAddress();
    bool isDatatypeMatch(unsigned char d);
};

enum class EExecStatus {
    OK_RUN = 0, 
    OK_STOP, 
    EXEC_ERROR,
    EXEC_ERROR_INVALID_INSTRUCTION,
    EXEC_ERROR_MOVE_EXPECTED_ADDRESS,
    EXEC_ERROR_MOVE_INCONSISTEND_DATATYPES,
    EXEC_ERROR_MOVEADD_EXPECTED_ADDRESS,
    EXEC_ERROR_MOVEADD_INCONSISTEND_DATATYPES,
    EXEC_ERROR_MOVESUBTR_EXPECTED_ADDRESS,
    EXEC_ERROR_MOVESUBTR_INCONSISTEND_DATATYPES,
    EXEC_ERROR_MOVESUBTR_BOOLEAN_NOT_SUPPORTED,
    EXEC_ERROR_MOVEMUL_EXPECTED_ADDRESS,
    EXEC_ERROR_MOVEMUL_INCONSISTEND_DATATYPES,
    EXEC_ERROR_MOVEMUL_STRING_NOT_SUPPORTED,
    EXEC_ERROR_MOVEDIV_EXPECTED_ADDRESS,
    EXEC_ERROR_MOVEDIV_INCONSISTEND_DATATYPES,
    EXEC_ERROR_MOVEDIV_DIV_BY_0,
    EXEC_ERROR_MOVEDIV_STRING_NOT_SUPPORTED,
    EXEC_ERROR_MOVEDIV_BOOLEAN_NOT_SUPPORTED,
    EXEC_ERROR_EQUAL_INCONSISTEND_DATATYPES,
    EXEC_ERROR_NOTEQUAL_INCONSISTEND_DATATYPES,
    EXEC_ERROR_LESSEQUAL_INCONSISTEND_DATATYPES,
    EXEC_ERROR_GREATEREQUAL_INCONSISTEND_DATATYPES,
    EXEC_ERROR_LESS_INCONSISTEND_DATATYPES,
    EXEC_ERROR_GREATER_INCONSISTEND_DATATYPES,
    EXEC_ERROR_JUMPIFFALSE_CONDITION_EXPECTED_BOOLEAN,
    EXEC_ERROR_MUL_INCONSISTEND_DATATYPES,
    EXEC_ERROR_MUL_STRING_NOT_SUPPORTED,
    EXEC_ERROR_DIV_INCONSISTEND_DATATYPES,
    EXEC_ERROR_DIV_DIV_BY_0,
    EXEC_ERROR_DIV_STRING_NOT_SUPPORTED,
    EXEC_ERROR_DIV_BOOLEAN_NOT_SUPPORTED,
    EXEC_ERROR_ADD_INCONSISTEND_DATATYPES,
    EXEC_ERROR_SUB_INCONSISTEND_DATATYPES,
    EXEC_ERROR_NEG_INCONSISTEND_DATATYPES,
    EXEC_ERROR_NEG_STRING_NOT_SUPPORTED,
    EXEC_ERROR_SYSCALL_SIN_STRING_NOT_SUPPORTED,
    EXEC_ERROR_SYSCALL_SIN_BOOLEAN_NOT_SUPPORTED,
    EXEC_ERROR_SYSCALL_SIN_INCONSISTENT_DATATYPES,
    EXEC_ERROR_SYSCALL_COS_STRING_NOT_SUPPORTED,
    EXEC_ERROR_SYSCALL_COS_BOOLEAN_NOT_SUPPORTED,
    EXEC_ERROR_SYSCALL_COS_INCONSISTENT_DATATYPES,
    EXEC_ERROR_SYSCALL_UNKNOWN_FUNCTION,
    EXEC_ERROR_RETURN_NO_RETURN_POINT,
    EXEC_ERROR_PUTINDADDR_EXPECTED_ADDRESS,
    EXEC_ERROR_PUTINDADDR_EXPECTED_ARRAY,
    EXEC_ERROR_PUTINDADDR_INCONSISTENT_INDEX_DATATYPES,
    EXEC_ERROR_MOVE_INCONSISTEND_ARRAY_DATATYPES,
    EXEC_ERROR_MOVEADD_ARRAY_NOT_SUPPORTED,
    EXEC_ERROR_LESS_ARRAY_NOT_SUPPORTED,
    EXEC_ERROR_GREATEREQUAL_ARRAY_NOT_SUPPORTED,
    EXEC_ERROR_LESSEQUAL_ARRAY_NOT_SUPPORTED,
    EXEC_ERROR_GREATER_ARRAY_NOT_SUPPORTED,
    EXEC_ERROR_MULL_ARRAY_NOT_SUPPORTED,
    EXEC_ERROR_DIV_ARRAY_NOT_SUPPORTED,
    EXEC_ERROR_MOVEDIV_ARRAY_NOT_SUPPORTED,
    EXEC_ERROR_MOVESUBTR_ARRAY_NOT_SUPPORTED,
    EXEC_ERROR_MOVEMUL_ARRAY_NOT_SUPPORTED,
    EXEC_ERROR_ADD_ARRAY_NOT_SUPPORTED,
    EXEC_ERROR_SUB_ARRAY_NOT_SUPPORTED,
    EXEC_ERROR_NEG_ARRAY_NOT_SUPPORTED
};

extern const char* exec_status_descriptions[];

struct CallStackValuePointer
{
    EDataTypes datatype;
    void* value;

    CallStackValuePointer() : datatype(EDataTypes::UNKNOWN), value(NULL) {}
    CallStackValuePointer(EDataTypes datatype, void* value) : datatype(datatype), value(value) {}
    ~CallStackValuePointer() {
        if (value != NULL) {
            switch (datatype) {
                case EDataTypes::INT:
                    delete static_cast<long long int*>(value);
                    break;
                case EDataTypes::FLOAT:
                    delete static_cast<long double*>(value);
                    break;
                case EDataTypes::BOOLEAN:
                    delete static_cast<bool*>(value);
                    break;
                case EDataTypes::STRING:
                    delete static_cast<std::string*>(value);
                    break;
                case EDataTypes::ARRAY:
                    delete static_cast<Array*>(value);
                    break;
            } // ~switch
        } // ~if
    }
};

class CallStackEntry
{
    private:
        unsigned int return_idx;    // The bytecode address to move control back at RETURN
        std::vector<CallStackValuePointer*> variables;

        CallStackValuePointer DUMMY_VALUE;

    public:
        CallStackEntry() : return_idx(-1) {}
        CallStackEntry(unsigned int return_idx): return_idx(return_idx) {}
        ~CallStackEntry() {
            for(int i=0; i<variables.size(); i++) {
                delete variables[i];
            }
        }

        unsigned int getReturnIdx() {
            return return_idx;
        }

        unsigned int addVariable(EDataTypes datatype, void* p_var) {
            variables.push_back(new CallStackValuePointer(datatype, p_var));
            return variables.size()-1;
        }

        CallStackValuePointer* getVariable(unsigned int idx) {
            if (idx >= variables.size()) {
                return &DUMMY_VALUE;
            }

            return variables[idx];
        }
};

class VM
{
private:
    std::vector<Element> stack;
    std::vector<CallStackEntry*> callstack;

public:
    VM();
    ~VM();

    static EDataTypes decodeDatatype(const Datatype& variable);
    bool isDatatypeConsistent(EDataTypes datatype_1, EDataTypes datatype_2);
    bool isDatatypeConsistentAssignment(EDataTypes datatype_1, EDataTypes datatype_2);
    void init();
    bool moveValue(void* lvar, EDataTypes ldatatype, Element& rval, EDataTypes rdatatype, EDataTypes rfinal_datatype, EExecStatus& status);
    void execute(Bytecode& bytecode, EExecStatus& status);
    bool interpret(Bytecode& bytecode, unsigned int& idx, EExecStatus& status);
};

#endif // VM_H
