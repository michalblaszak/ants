#include "vm.h"
#include "parser.h"
#include <cmath>
#include <boost/algorithm/string/erase.hpp>

char datatypes_string[] = {
    'u', 'i', 'f', 's', 'b', 'A', 'a' // 'u' - unknown, 'A' - Address, 'a' - array
};

/***********************************************
 * Element implementation
 ***********************************************/

Element::Element(): datatype(EDataTypes::UNKNOWN)
{
}

Element::Element(long long int val) : datatype(EDataTypes::INT), value(val)
{
}

Element::Element(long double val) : datatype(EDataTypes::FLOAT), value(val)
{
}

Element::Element(const std::string& val) : datatype(EDataTypes::STRING), value(val)
{
}

Element::Element(bool val) : datatype(EDataTypes::BOOLEAN), value(val)
{
}

Element::Element(const Array& val) : datatype(EDataTypes::ARRAY), value(val)
{
}

Element::Element(EDataTypes datatype, void* val) : datatype(EDataTypes::ADDRESS), value(datatype, val)
{
}

Element::Element(const Element& other)
{
    datatype = other.datatype;
    switch(datatype) {
        case EDataTypes::UNKNOWN: break;
        case EDataTypes::INT: value.intVal = other.value.intVal; break;
        case EDataTypes::FLOAT: value.floatVal = other.value.floatVal; break;
        case EDataTypes::STRING: new (&value.stringVal) std::string(other.value.stringVal); break;
        case EDataTypes::BOOLEAN: value.booleanVal = other.value.booleanVal; break;
        case EDataTypes::ADDRESS: value.addrVal = other.value.addrVal; break;
        case EDataTypes::ARRAY: new (&value.arrayVal) Array(other.value.arrayVal); break;
    }
}

Element& Element::operator=(const Element& other) {
    datatype = other.datatype;
    switch(datatype) {
        case EDataTypes::UNKNOWN: break;
        case EDataTypes::INT: value.intVal = other.value.intVal; break;
        case EDataTypes::FLOAT: value.floatVal = other.value.floatVal; break;
        case EDataTypes::STRING: new (&value.stringVal) std::string(other.value.stringVal); break;
        case EDataTypes::BOOLEAN: value.booleanVal = other.value.booleanVal; break;
        case EDataTypes::ADDRESS: value.addrVal = other.value.addrVal; break;
        case EDataTypes::ARRAY: new (&value.arrayVal) Array(other.value.arrayVal); break;
    }

    return *this;
}

Element::~Element()
{
    if (datatype == EDataTypes::STRING) {
        value.stringVal.~basic_string();
    }
    else if (datatype == EDataTypes::ARRAY) {
        value.arrayVal.~Array();
    }
}

EDataTypes Element::getDatatype() {
    return datatype;
}

EDataTypes Element::getFinalDatatype() const {
    return datatype == EDataTypes::ADDRESS ? value.addrVal.datatype : datatype;
}

char Element::getDatatypeString() const {
    return datatypes_string[static_cast<int>(datatype)];
}

char Element::getFinalDatatypeString() const {
    return datatypes_string[static_cast<int>(getFinalDatatype())];
}

const union Element::UValues& Element::getValue() {
    return value;
}

void* Element::getVariablePhysicalAddress() {
    switch(datatype) {
        case EDataTypes::INT:
            return &value.intVal;
        case EDataTypes::FLOAT:
            return &value.floatVal;
        case EDataTypes::STRING:
            return &value.stringVal;
        case EDataTypes::BOOLEAN:
            return &value.booleanVal;
        case EDataTypes::ARRAY:
            return &value.arrayVal;
        case EDataTypes::ADDRESS:
            return value.addrVal.addr;
    }

    return NULL;
}

bool Element::isDatatypeMatch(unsigned char d) {
    EDataTypes final_datatype = (datatype == EDataTypes::ADDRESS) ? value.addrVal.datatype : datatype;
    
    return (d == 'i' && final_datatype == EDataTypes::INT)
        || (d == 'f' && final_datatype == EDataTypes::FLOAT)
        || (d == 'b' && final_datatype == EDataTypes::BOOLEAN)
        || (d == 's' && final_datatype == EDataTypes::STRING)
        || (d == 'a' && final_datatype == EDataTypes::ARRAY);
}

/***********************************************
 * VM implementation
 ***********************************************/

const char* exec_status_descriptions[] = {
    "OK_RUN", 
    "OK_STOP", 
    "EXEC_ERROR",
    "EXEC_ERROR_INVALID_INSTRUCTION",
    "EXEC_ERROR_MOVE_EXPECTED_ADDRESS",
    "EXEC_ERROR_MOVE_INCONSISTEND_DATATYPES",
    "EXEC_ERROR_MOVEADD_EXPECTED_ADDRESS",
    "EXEC_ERROR_MOVEADD_INCONSISTEND_DATATYPES",
    "EXEC_ERROR_MOVESUBTR_EXPECTED_ADDRESS",
    "EXEC_ERROR_MOVESUBTR_INCONSISTEND_DATATYPES",
    "EXEC_ERROR_MOVESUBTR_BOOLEAN_NOT_SUPPORTED",
    "EXEC_ERROR_MOVEMUL_EXPECTED_ADDRESS",
    "EXEC_ERROR_MOVEMUL_INCONSISTEND_DATATYPES",
    "EXEC_ERROR_MOVEMUL_STRING_NOT_SUPPORTED",
    "EXEC_ERROR_MOVEDIV_EXPECTED_ADDRESS",
    "EXEC_ERROR_MOVEDIV_INCONSISTEND_DATATYPES",
    "EXEC_ERROR_MOVEDIV_DIV_BY_0",
    "EXEC_ERROR_MOVEDIV_STRING_NOT_SUPPORTED",
    "EXEC_ERROR_MOVEDIV_BOOLEAN_NOT_SUPPORTED",
    "EXEC_ERROR_EQUAL_INCONSISTEND_DATATYPES",
    "EXEC_ERROR_NOTEQUAL_INCONSISTEND_DATATYPES",
    "EXEC_ERROR_LESSEQUAL_INCONSISTEND_DATATYPES",
    "EXEC_ERROR_GREATEREQUAL_INCONSISTEND_DATATYPES",
    "EXEC_ERROR_LESS_INCONSISTEND_DATATYPES",
    "EXEC_ERROR_GREATER_INCONSISTEND_DATATYPES",
    "EXEC_ERROR_JUMPIFFALSE_CONDITION_EXPECTED_BOOLEAN",
    "EXEC_ERROR_MUL_INCONSISTEND_DATATYPES",
    "EXEC_ERROR_MUL_STRING_NOT_SUPPORTED",
    "EXEC_ERROR_DIV_INCONSISTEND_DATATYPES",
    "EXEC_ERROR_DIV_DIV_BY_0",
    "EXEC_ERROR_DIV_STRING_NOT_SUPPORTED",
    "EXEC_ERROR_DIV_BOOLEAN_NOT_SUPPORTED",
    "EXEC_ERROR_ADD_INCONSISTEND_DATATYPES",
    "EXEC_ERROR_SUB_INCONSISTEND_DATATYPES",
    "EXEC_ERROR_NEG_INCONSISTEND_DATATYPES",
    "EXEC_ERROR_NEG_STRING_NOT_SUPPORTED",
    "EXEC_ERROR_SYSCALL_SIN_STRING_NOT_SUPPORTED",
    "EXEC_ERROR_SYSCALL_SIN_BOOLEAN_NOT_SUPPORTED",
    "EXEC_ERROR_SYSCALL_SIN_INCONSISTENT_DATATYPES",
    "EXEC_ERROR_SYSCALL_COS_STRING_NOT_SUPPORTED",
    "EXEC_ERROR_SYSCALL_COS_BOOLEAN_NOT_SUPPORTED",
    "EXEC_ERROR_SYSCALL_COS_INCONSISTENT_DATATYPES",
    "EXEC_ERROR_SYSCALL_UNKNOWN_FUNCTION",
    "EXEC_ERROR_RETURN_NO_RETURN_POINT",
    "EXEC_ERROR_PUTINDADDR_EXPECTED_ADDRESS",
    "EXEC_ERROR_PUTINDADDR_EXPECTED_ARRAY",
    "EXEC_ERROR_PUTINDADDR_INCONSISTENT_INDEX_DATATYPES",
    "EXEC_ERROR_MOVE_INCONSISTEND_ARRAY_DATATYPES",
    "EXEC_ERROR_MOVEADD_ARRAY_NOT_SUPPORTED",
    "EXEC_ERROR_LESS_ARRAY_NOT_SUPPORTED",
    "EXEC_ERROR_GREATEREQUAL_ARRAY_NOT_SUPPORTED",
    "EXEC_ERROR_LESSEQUAL_ARRAY_NOT_SUPPORTED",
    "EXEC_ERROR_GREATER_ARRAY_NOT_SUPPORTED",
    "EXEC_ERROR_MULL_ARRAY_NOT_SUPPORTED",
    "EXEC_ERROR_DIV_ARRAY_NOT_SUPPORTED",
    "EXEC_ERROR_MOVEDIV_ARRAY_NOT_SUPPORTED",
    "EXEC_ERROR_MOVESUBTR_ARRAY_NOT_SUPPORTED",
    "EXEC_ERROR_MOVEMUL_ARRAY_NOT_SUPPORTED",
    "EXEC_ERROR_ADD_ARRAY_NOT_SUPPORTED",
    "EXEC_ERROR_SUB_ARRAY_NOT_SUPPORTED",
    "EXEC_ERROR_NEG_ARRAY_NOT_SUPPORTED"
};

VM::VM()
{

}

VM::~VM() {
    for(int i=0; i<callstack.size(); i++) {
        delete callstack[i];
    }
}

EDataTypes VM::decodeDatatype(const Datatype& variable) {
    const char* datatype = variable.getDatatype(); // Get variable's datatype
    EDataTypes ret_datatype = (datatype[0] == 'i') ? EDataTypes::INT :
                              (datatype[0] == 'f') ? EDataTypes::FLOAT :
                              (datatype[0] == 's') ? EDataTypes::STRING :
                              (datatype[0] == 'b') ? EDataTypes::BOOLEAN :
                              (datatype[0] == 'a') ? EDataTypes::ARRAY :
                              EDataTypes::UNKNOWN;
                              
    return ret_datatype;
}

bool VM::isDatatypeConsistent(EDataTypes datatype_1, EDataTypes datatype_2)
{
    if (datatype_1 == datatype_2) 
        return true;
    else if ( (datatype_1 == EDataTypes::INT || datatype_1 == EDataTypes::FLOAT )
         && (datatype_2 == EDataTypes::INT || datatype_2 == EDataTypes::FLOAT)
    )
        return true;
    else 
        return false;
}

bool VM::isDatatypeConsistentAssignment(EDataTypes datatype_1, EDataTypes datatype_2)
{
    if (datatype_1 == datatype_2) 
        return true;
    else if ( (datatype_1 == EDataTypes::FLOAT)
         && (datatype_2 == EDataTypes::INT || datatype_2 == EDataTypes::FLOAT)
    )
        return true;
    else 
        return false;
}

void VM::init()
{
    stack.clear();
}


void VM::execute(Bytecode& bytecode, EExecStatus& status)
{
    // Create variables
    bytecode.makeAddresses();

    // Execute the code
    unsigned int i = 0;
    while(interpret(bytecode, i, status));

    printf("Program execution finished: %s\n", exec_status_descriptions[static_cast<int>(status)]);
    bytecode.printVariables();

    // Dispose variables
    bytecode.disposeAddresses();
}

bool VM::moveValue(void* lvar, EDataTypes ldatatype, Element& rval, EDataTypes rdatatype, EDataTypes rfinal_datatype, EExecStatus& status) {
    if (!isDatatypeConsistentAssignment(ldatatype, rfinal_datatype)) {
        status = EExecStatus::EXEC_ERROR_MOVE_INCONSISTEND_DATATYPES;
        return false;
    }

    if (rdatatype == EDataTypes::ADDRESS) {
        void* r_val = rval.getValue().addrVal.addr;

        switch(rfinal_datatype) {
            case EDataTypes::INT:
                if (ldatatype == EDataTypes::INT) {
                    *(long long int*)lvar = *(long long int*)r_val;
                } else if (ldatatype == EDataTypes::FLOAT) { 
                    *(long double*)lvar = *(long long int*)r_val;
                } else {
                    status = EExecStatus::EXEC_ERROR_MOVE_INCONSISTEND_DATATYPES;
                    return false; // We shouldn't be here. Dataypes are inconsistent
                }
                break;
            case EDataTypes::FLOAT:
                if (ldatatype == EDataTypes::FLOAT) {
                    *(long double*)lvar = *(long double*)r_val;
                } else { // We shouldn't be here. Dataypes are inconsistent
                    status = EExecStatus::EXEC_ERROR_MOVE_INCONSISTEND_DATATYPES;
                    return false;
                }
                break;
            case EDataTypes::STRING:
                if (ldatatype == EDataTypes::STRING) {
                    *(std::string*)lvar = *(std::string*)r_val;
                } else { // We shouldn't be here. Dataypes are inconsistent
                    status = EExecStatus::EXEC_ERROR_MOVE_INCONSISTEND_DATATYPES;
                    return false;
                }
                break;
            case EDataTypes::BOOLEAN:
                if (ldatatype == EDataTypes::BOOLEAN) {
                    *(bool*)lvar = *(bool*)r_val;
                } else { // We shouldn't be here. Dataypes are inconsistent
                    status = EExecStatus::EXEC_ERROR_MOVE_INCONSISTEND_DATATYPES;
                    return false;
                }
                break;
            case EDataTypes::ARRAY:
                if (ldatatype == EDataTypes::ARRAY) {
                    if (!Array::areArraysCompatible( ((Array*)lvar)->getElementDatatype(), ((Array*)r_val)->getElementDatatype())) {
                        status = EExecStatus::EXEC_ERROR_MOVE_INCONSISTEND_ARRAY_DATATYPES;
                        return false;
                    }

                    *(Array*)lvar = *(Array*)r_val;
                } else { // We shouldn't be here. Dataypes are inconsistent
                    status = EExecStatus::EXEC_ERROR_MOVE_INCONSISTEND_DATATYPES;
                    return false;
                }
                break;
        } // ~switch
    } else { // it's a regular value
        switch(rfinal_datatype) {
            case EDataTypes::INT:
                if (ldatatype == EDataTypes::INT) {
                    *(long long int*)lvar = rval.getValue().intVal;
                } else if (ldatatype == EDataTypes::FLOAT) { 
                    *(long double*)lvar = rval.getValue().intVal;
                } else {
                    status = EExecStatus::EXEC_ERROR_MOVE_INCONSISTEND_DATATYPES;
                    return false; // We shouldn't be here. Dataypes are inconsistent
                }
                break;
            case EDataTypes::FLOAT:
                if (ldatatype == EDataTypes::FLOAT) {
                    *(long double*)lvar = rval.getValue().floatVal;
                } else { // We shouldn't be here. Dataypes are inconsistent
                    status = EExecStatus::EXEC_ERROR_MOVE_INCONSISTEND_DATATYPES;
                    return false;
                }
                break;
            case EDataTypes::STRING:
                if (ldatatype == EDataTypes::STRING) {
                    *(std::string*)lvar = rval.getValue().stringVal;
                } else { // We shouldn't be here. Dataypes are inconsistent
                    status = EExecStatus::EXEC_ERROR_MOVE_INCONSISTEND_DATATYPES;
                    return false;
                }
                break;
            case EDataTypes::BOOLEAN:
                if (ldatatype == EDataTypes::BOOLEAN) {
                    *(bool*)lvar = rval.getValue().booleanVal;
                } else { // We shouldn't be here. Dataypes are inconsistent
                    status = EExecStatus::EXEC_ERROR_MOVE_INCONSISTEND_DATATYPES;
                    return false;
                }
                break;
            case EDataTypes::ARRAY:
                if (ldatatype == EDataTypes::ARRAY) {
                    if (!Array::areArraysCompatible( ((Array*)lvar)->getElementDatatype(), rval.getValue().arrayVal.getElementDatatype())) {
                        status = EExecStatus::EXEC_ERROR_MOVE_INCONSISTEND_ARRAY_DATATYPES;
                        return false;
                    }

                    *(Array*)lvar = rval.getValue().arrayVal;
                } else { // We shouldn't be here. Dataypes are inconsistent
                    status = EExecStatus::EXEC_ERROR_MOVE_INCONSISTEND_DATATYPES;
                    return false;
                }
                break;
        } // ~switch
    } // ~if

    return true;
}

bool VM::interpret(Bytecode& bytecode, unsigned int& idx, EExecStatus& status)
{
    const std::vector<unsigned char>& code = bytecode.getCode();

    status = EExecStatus::OK_RUN;

    if (idx >= code.size()) {
        status = EExecStatus::OK_STOP;
        return false;
    }

    unsigned char c = code[idx++];

    switch(c) {
        case EInstrCodes::NOP: break;
        case EInstrCodes::DATA: {
            status = EExecStatus::EXEC_ERROR_INVALID_INSTRUCTION;
            return false; // It's handled separately
        }
        case EInstrCodes::DDATA: {
            status = EExecStatus::EXEC_ERROR_INVALID_INSTRUCTION;
            return false; // It's handled separately
        }
        case EInstrCodes::FUN: {
            status = EExecStatus::EXEC_ERROR_INVALID_INSTRUCTION;
            return false; // It's handled separately
        }
        case EInstrCodes::PUTADDR: {
            unsigned int var_idx;
            unsigned char* pvar_idx = (unsigned char*)&var_idx;
            *pvar_idx = code[idx++];
            *(pvar_idx+1) = code[idx++];
            *(pvar_idx+2) = code[idx++];
            *(pvar_idx+3) = code[idx++];

            Datatype& variable = bytecode.getVariables()[var_idx];
            void* addr = variable.getAddress(); // Get a physical address of a variable with index var_idx 
            EDataTypes stack_datatype = decodeDatatype(variable);

            stack.push_back(Element(stack_datatype, addr));
            break;
        }
        case EInstrCodes::PUTINDADDR: {
            // Get number of indexes
            unsigned char n_idx;
            n_idx = code[idx++];

            // Take indexes from the stack
            std::vector<Element> indexes;
            for(int i=0; i<n_idx; i++) {
                Element idx_val = stack.back();
                indexes.push_back(idx_val);
                stack.pop_back();
            }

            // Get Array variable
            Element array_variable = stack.back();
            stack.pop_back();

            EDataTypes array_var_datatype = array_variable.getDatatype();
            if (array_var_datatype != EDataTypes::ADDRESS) {
                status = EExecStatus::EXEC_ERROR_PUTINDADDR_EXPECTED_ADDRESS;
                return false;
            }
            EDataTypes array_var_final_datatype = array_var_datatype == EDataTypes::ADDRESS ? array_variable.getValue().addrVal.datatype : array_var_datatype;

            if (array_variable.getValue().addrVal.datatype != EDataTypes::ARRAY) {
                status = EExecStatus::EXEC_ERROR_PUTINDADDR_EXPECTED_ARRAY;
                return false;
            }
            Array* addr = (Array*)array_variable.getValue().addrVal.addr;

            // Check if index types are consistend with the arrays definition
            // and get index values
            std::vector<ValuePointer> index_values;

            for (int i=0; i<indexes.size(); i++) {
                if (!indexes[i].isDatatypeMatch(addr->getIndexDatatypes()[i]) ) {
                    status = EExecStatus::EXEC_ERROR_PUTINDADDR_INCONSISTENT_INDEX_DATATYPES;
                    return false;
                }

                index_values.push_back(ValuePointer(std::string() + indexes[i].getFinalDatatypeString(), indexes[i].getVariablePhysicalAddress()));
            }
            
            char arr_el_datatype = addr->getElementDatatype()[0];
            EDataTypes stack_datatype = (arr_el_datatype == 'i') ? EDataTypes::INT :
                                        (arr_el_datatype == 'f') ? EDataTypes::FLOAT :
                                        (arr_el_datatype == 's') ? EDataTypes::STRING :
                                        (arr_el_datatype == 'b') ? EDataTypes::BOOLEAN :
                                        (arr_el_datatype == 'a') ? EDataTypes::ARRAY :
                                        EDataTypes::UNKNOWN;

            stack.push_back(Element(stack_datatype, addr->getElement(index_values)->getValue().pvalue));
            break;
        }
        case EInstrCodes::PUTMEMBERADDR: break; // TODO
        case EInstrCodes::PUTINT: {
            long long int val;
            unsigned char* pval = (unsigned char*)&val;
            *pval = code[idx++];
            *(pval+1) = code[idx++];
            *(pval+2) = code[idx++];
            *(pval+3) = code[idx++];
            *(pval+4) = code[idx++];
            *(pval+5) = code[idx++];
            *(pval+6) = code[idx++];
            *(pval+7) = code[idx++];

            stack.push_back(Element(val));
            break;
        }
        case EInstrCodes::PUTFLOAT: {
            long double val;
            unsigned char* pval = (unsigned char*)&val;
            *pval = code[idx++];
            *(pval+1) = code[idx++];
            *(pval+2) = code[idx++];
            *(pval+3) = code[idx++];
            *(pval+4) = code[idx++];
            *(pval+5) = code[idx++];
            *(pval+6) = code[idx++];
            *(pval+7) = code[idx++];
            *(pval+8) = code[idx++];
            *(pval+9) = code[idx++];
            *(pval+10) = code[idx++];
            *(pval+11) = code[idx++];

            stack.push_back(Element(val));
            break;
        }
        case EInstrCodes::PUTSTRING: {
            unsigned short c;
            std::string str;

            do {
                c = code[idx++];

                if (c == 0) {
                    break;
                }
                str += c;
            } while(true);

            stack.push_back(Element(str));
            break;
        }
        case EInstrCodes::PUTBOOLEAN: {
            unsigned char val = code[idx++];

            stack.push_back(Element(val == 1));
            break;
        }
        case EInstrCodes::MOVE: { 
            Element e_val = stack.back();
            stack.pop_back();
            Element e_var = stack.back();
            stack.pop_back();

            EDataTypes e_var_datatype = e_var.getDatatype();
            EDataTypes e_var_final_datatype = e_var_datatype == EDataTypes::ADDRESS ? e_var.getValue().addrVal.datatype : e_var_datatype;
            EDataTypes e_val_datatype = e_val.getDatatype();
            EDataTypes e_val_final_datatype = e_val_datatype == EDataTypes::ADDRESS ? e_val.getValue().addrVal.datatype : e_val_datatype;

            if (e_var_datatype != EDataTypes::ADDRESS) {
                status = EExecStatus::EXEC_ERROR_MOVE_EXPECTED_ADDRESS;
                return false;
            }

            // Left variable physical address
            void* l_var = e_var.getValue().addrVal.addr;

            // Calculate Right side value
            if ( !moveValue(l_var, e_var_final_datatype, e_val, e_val_datatype, e_val_final_datatype, status) ) {
                return false;
            }

            break;
        }
        case EInstrCodes::MOVEADD: {
            Element e_val = stack.back();
            stack.pop_back();
            Element e_var = stack.back();
            stack.pop_back();

            EDataTypes e_var_datatype = e_var.getDatatype();
            EDataTypes e_var_final_datatype = e_var_datatype == EDataTypes::ADDRESS ? e_var.getValue().addrVal.datatype : e_var_datatype;
            EDataTypes e_val_datatype = e_val.getDatatype();
            EDataTypes e_val_final_datatype = e_val_datatype == EDataTypes::ADDRESS ? e_val.getValue().addrVal.datatype : e_val_datatype;

            if (e_var_datatype != EDataTypes::ADDRESS) {
                status = EExecStatus::EXEC_ERROR_MOVEADD_EXPECTED_ADDRESS;
                return false;
            }

            if (!isDatatypeConsistentAssignment(e_var_final_datatype, e_val_final_datatype)) {
                status = EExecStatus::EXEC_ERROR_MOVEADD_INCONSISTEND_DATATYPES;
                return false;
            }

            // Left variable physical address
            void* l_var = e_var.getValue().addrVal.addr;

            // Calculate Right side value
            if (e_val_datatype == EDataTypes::ADDRESS) {
                void* r_val = e_val.getValue().addrVal.addr;

                switch(e_val_final_datatype) {
                    case EDataTypes::INT:
                        if (e_var_final_datatype == EDataTypes::INT) {
                            *(long long int*)l_var += *(long long int*)r_val;
                        } else if (e_var_final_datatype == EDataTypes::FLOAT) { 
                            *(long double*)l_var += *(long long int*)r_val;
                        } else {
                            status = EExecStatus::EXEC_ERROR_MOVEADD_INCONSISTEND_DATATYPES;
                            return false; // We shouldn't be here. Dataypes are inconsistent
                        }
                        break;
                    case EDataTypes::FLOAT:
                        if (e_var_final_datatype == EDataTypes::FLOAT) {
                            *(long double*)l_var += *(long double*)r_val;
                        } else { // We shouldn't be here. Dataypes are inconsistent
                            status = EExecStatus::EXEC_ERROR_MOVEADD_INCONSISTEND_DATATYPES;
                            return false;
                        }
                        break;
                    case EDataTypes::STRING:
                        if (e_var_final_datatype == EDataTypes::STRING) {
                            *(std::string*)l_var += *(std::string*)r_val;
                        } else { // We shouldn't be here. Dataypes are inconsistent
                            status = EExecStatus::EXEC_ERROR_MOVEADD_INCONSISTEND_DATATYPES;
                            return false;
                        }
                        break;
                    case EDataTypes::BOOLEAN:
                        if (e_var_final_datatype == EDataTypes::BOOLEAN) {
                            *(bool*)l_var = *(bool*)l_var || *(bool*)r_val;
                        } else { // We shouldn't be here. Dataypes are inconsistent
                            status = EExecStatus::EXEC_ERROR_MOVEADD_INCONSISTEND_DATATYPES;
                            return false;
                        }
                        break;
                    case EDataTypes::ARRAY:
                        if (e_var_final_datatype == EDataTypes::ARRAY) {
                            *(Array*)l_var += *(Array*)r_val;
                        } else { // We shouldn't be here. Dataypes are inconsistent
                            status = EExecStatus::EXEC_ERROR_MOVEADD_INCONSISTEND_DATATYPES;
                            return false;
                        }
                        break;


//                        status = EExecStatus::EXEC_ERROR_MOVEADD_ARRAY_NOT_SUPPORTED;
//                        return false;
                } // ~switch
            } else { // it's a regular value
                switch(e_val_final_datatype) {
                    case EDataTypes::INT:
                        if (e_var_final_datatype == EDataTypes::INT) {
                            *(long long int*)l_var += e_val.getValue().intVal;
                        } else if (e_var_final_datatype == EDataTypes::FLOAT) { 
                            *(long double*)l_var += e_val.getValue().intVal;
                        } else {
                            status = EExecStatus::EXEC_ERROR_MOVEADD_INCONSISTEND_DATATYPES;
                            return false; // We shouldn't be here. Dataypes are inconsistent
                        }
                        break;
                    case EDataTypes::FLOAT:
                        if (e_var_final_datatype == EDataTypes::FLOAT) {
                            *(long double*)l_var += e_val.getValue().floatVal;
                        } else { // We shouldn't be here. Dataypes are inconsistent
                            status = EExecStatus::EXEC_ERROR_MOVEADD_INCONSISTEND_DATATYPES;
                            return false;
                        }
                        break;
                    case EDataTypes::STRING:
                        if (e_var_final_datatype == EDataTypes::STRING) {
                            *(std::string*)l_var += e_val.getValue().stringVal;
                        } else { // We shouldn't be here. Dataypes are inconsistent
                            status = EExecStatus::EXEC_ERROR_MOVEADD_INCONSISTEND_DATATYPES;
                            return false;
                        }
                        break;
                    case EDataTypes::BOOLEAN:
                        if (e_var_final_datatype == EDataTypes::BOOLEAN) {
                            *(bool*)l_var = *(bool*)l_var || e_val.getValue().booleanVal;
                        } else { // We shouldn't be here. Dataypes are inconsistent
                            status = EExecStatus::EXEC_ERROR_MOVEADD_INCONSISTEND_DATATYPES;
                            return false;
                        }
                        break;
                    case EDataTypes::ARRAY:
                        if (e_var_final_datatype == EDataTypes::ARRAY) {
                            *(Array*)l_var += e_val.getValue().arrayVal;
                        } else { // We shouldn't be here. Dataypes are inconsistent
                            status = EExecStatus::EXEC_ERROR_MOVEADD_INCONSISTEND_DATATYPES;
                            return false;
                        }
                        break;


//                        status = EExecStatus::EXEC_ERROR_MOVEADD_ARRAY_NOT_SUPPORTED;
//                        return false;
                } // ~switch
            } // ~if

            break;
        }
        case EInstrCodes::MOVESUBTR: {
            Element e_val = stack.back();
            stack.pop_back();
            Element e_var = stack.back();
            stack.pop_back();

            EDataTypes e_var_datatype = e_var.getDatatype();
            EDataTypes e_var_final_datatype = e_var_datatype == EDataTypes::ADDRESS ? e_var.getValue().addrVal.datatype : e_var_datatype;
            EDataTypes e_val_datatype = e_val.getDatatype();
            EDataTypes e_val_final_datatype = e_val_datatype == EDataTypes::ADDRESS ? e_val.getValue().addrVal.datatype : e_val_datatype;

            if (e_var_datatype != EDataTypes::ADDRESS) {
                status = EExecStatus::EXEC_ERROR_MOVESUBTR_EXPECTED_ADDRESS;
                return false;
            }

            if (!isDatatypeConsistentAssignment(e_var_final_datatype, e_val_final_datatype)) {
                status = EExecStatus::EXEC_ERROR_MOVESUBTR_INCONSISTEND_DATATYPES;
                return false;
            }

            // Left variable physical address
            void* l_var = e_var.getValue().addrVal.addr;

            // Calculate Right side value
            if (e_val_datatype == EDataTypes::ADDRESS) {
                void* r_val = e_val.getValue().addrVal.addr;

                switch(e_val_final_datatype) {
                    case EDataTypes::INT:
                        if (e_var_final_datatype == EDataTypes::INT) {
                            *(long long int*)l_var -= *(long long int*)r_val;
                        } else if (e_var_final_datatype == EDataTypes::FLOAT) { 
                            *(long double*)l_var -= *(long long int*)r_val;
                        } else {
                            status = EExecStatus::EXEC_ERROR_MOVESUBTR_INCONSISTEND_DATATYPES;
                            return false; // We shouldn't be here. Dataypes are inconsistent
                        }
                        break;
                    case EDataTypes::FLOAT:
                        if (e_var_final_datatype == EDataTypes::FLOAT) {
                            *(long double*)l_var -= *(long double*)r_val;
                        } else { // We shouldn't be here. Dataypes are inconsistent
                            status = EExecStatus::EXEC_ERROR_MOVESUBTR_INCONSISTEND_DATATYPES;
                            return false;
                        }
                        break;
                    case EDataTypes::STRING:
                        if (e_var_final_datatype == EDataTypes::STRING) {
                            boost::erase_all(*(std::string*)l_var, *(std::string*)r_val); // remove all occurences of r_val from l_var
                        } else { // We shouldn't be here. Dataypes are inconsistent
                            status = EExecStatus::EXEC_ERROR_MOVESUBTR_INCONSISTEND_DATATYPES;
                            return false;
                        }
                        break;
                    case EDataTypes::BOOLEAN:
                        if (e_var_final_datatype == EDataTypes::BOOLEAN) {
                            *(bool*)l_var = (*(bool*)l_var != *(bool*)r_val);   // XOR
                        } else { // We shouldn't be here. Dataypes are inconsistent
                            status = EExecStatus::EXEC_ERROR_MOVESUBTR_INCONSISTEND_DATATYPES;
                            return false;
                        }
                        break;
                    case EDataTypes::ARRAY:
                        status = EExecStatus::EXEC_ERROR_MOVESUBTR_ARRAY_NOT_SUPPORTED;
                        return false;
                } // ~switch
            } else { // it's a regular value
                switch(e_val_final_datatype) {
                    case EDataTypes::INT:
                        if (e_var_final_datatype == EDataTypes::INT) {
                            *(long long int*)l_var -= e_val.getValue().intVal;
                        } else if (e_var_final_datatype == EDataTypes::FLOAT) { 
                            *(long double*)l_var -= e_val.getValue().intVal;
                        } else {
                            status = EExecStatus::EXEC_ERROR_MOVESUBTR_INCONSISTEND_DATATYPES;
                            return false; // We shouldn't be here. Dataypes are inconsistent
                        }
                        break;
                    case EDataTypes::FLOAT:
                        if (e_var_final_datatype == EDataTypes::FLOAT) {
                            *(long double*)l_var -= e_val.getValue().floatVal; // remove all occurences of r_val from l_var
                        } else { // We shouldn't be here. Dataypes are inconsistent
                            status = EExecStatus::EXEC_ERROR_MOVESUBTR_INCONSISTEND_DATATYPES;
                            return false;
                        }
                        break;
                    case EDataTypes::STRING:
                        if (e_var_final_datatype == EDataTypes::STRING) {
                            boost::erase_all(*(std::string*)l_var, e_val.getValue().stringVal); // remove all occurences of r_val from l_var
                        } else { // We shouldn't be here. Dataypes are inconsistent
                            status = EExecStatus::EXEC_ERROR_MOVESUBTR_INCONSISTEND_DATATYPES;
                            return false;
                        }
                        break;
                    case EDataTypes::BOOLEAN:
                        status = EExecStatus::EXEC_ERROR_MOVESUBTR_BOOLEAN_NOT_SUPPORTED;
                        return false; // Subtraction is undefined for boolean
                    case EDataTypes::ARRAY:
                        status = EExecStatus::EXEC_ERROR_MOVESUBTR_ARRAY_NOT_SUPPORTED;
                        return false;
                } // ~switch
            } // ~if

            break;
        }
        case EInstrCodes::MOVEMUL: {
            Element e_val = stack.back();
            stack.pop_back();
            Element e_var = stack.back();
            stack.pop_back();

            EDataTypes e_var_datatype = e_var.getDatatype();
            EDataTypes e_var_final_datatype = e_var_datatype == EDataTypes::ADDRESS ? e_var.getValue().addrVal.datatype : e_var_datatype;
            EDataTypes e_val_datatype = e_val.getDatatype();
            EDataTypes e_val_final_datatype = e_val_datatype == EDataTypes::ADDRESS ? e_val.getValue().addrVal.datatype : e_val_datatype;

            if (e_var_datatype != EDataTypes::ADDRESS) {
                status = EExecStatus::EXEC_ERROR_MOVEMUL_EXPECTED_ADDRESS;
                return false;
            }

            if (!isDatatypeConsistentAssignment(e_var_final_datatype, e_val_final_datatype)) {
                status = EExecStatus::EXEC_ERROR_MOVEMUL_INCONSISTEND_DATATYPES;
                return false;
            }

            // Left variable physical address
            void* l_var = e_var.getValue().addrVal.addr;

            // Calculate Right side value
            if (e_val_datatype == EDataTypes::ADDRESS) {
                void* r_val = e_val.getValue().addrVal.addr;

                switch(e_val_final_datatype) {
                    case EDataTypes::INT:
                        if (e_var_final_datatype == EDataTypes::INT) {
                            *(long long int*)l_var *= *(long long int*)r_val;
                        } else if (e_var_final_datatype == EDataTypes::FLOAT) { 
                            *(long double*)l_var *= *(long long int*)r_val;
                        } else {
                            status = EExecStatus::EXEC_ERROR_MOVEMUL_INCONSISTEND_DATATYPES;
                            return false; // We shouldn't be here. Dataypes are inconsistent
                        }
                        break;
                    case EDataTypes::FLOAT:
                        if (e_var_final_datatype == EDataTypes::FLOAT) {
                            *(long double*)l_var *= *(long double*)r_val;
                        } else { // We shouldn't be here. Dataypes are inconsistent
                            status = EExecStatus::EXEC_ERROR_MOVEMUL_INCONSISTEND_DATATYPES;
                            return false;
                        }
                        break;
                    case EDataTypes::STRING:
                        status = EExecStatus::EXEC_ERROR_MOVEMUL_STRING_NOT_SUPPORTED;
                        return false; // String multiplication is undefined
                    case EDataTypes::BOOLEAN:
                        if (e_var_final_datatype == EDataTypes::BOOLEAN) {
                            *(bool*)l_var = *(bool*)l_var && *(bool*)r_val;
                        } else { // We shouldn't be here. Dataypes are inconsistent
                            status = EExecStatus::EXEC_ERROR_MOVEMUL_INCONSISTEND_DATATYPES;
                            return false;
                        }
                        break;
                    case EDataTypes::ARRAY:
                        status = EExecStatus::EXEC_ERROR_MOVEMUL_ARRAY_NOT_SUPPORTED;
                        return false;
                } // ~switch
            } else { // it's a regular value
                switch(e_val_final_datatype) {
                    case EDataTypes::INT:
                        if (e_var_final_datatype == EDataTypes::INT) {
                            *(long long int*)l_var *= e_val.getValue().intVal;
                        } else if (e_var_final_datatype == EDataTypes::FLOAT) { 
                            *(long double*)l_var *= e_val.getValue().intVal;
                        } else {
                            status = EExecStatus::EXEC_ERROR_MOVEMUL_INCONSISTEND_DATATYPES;
                            return false; // We shouldn't be here. Dataypes are inconsistent
                        }
                        break;
                    case EDataTypes::FLOAT:
                        if (e_var_final_datatype == EDataTypes::FLOAT) {
                            *(long double*)l_var *= e_val.getValue().floatVal;
                        } else { // We shouldn't be here. Dataypes are inconsistent
                            status = EExecStatus::EXEC_ERROR_MOVEMUL_INCONSISTEND_DATATYPES;
                            return false;
                        }
                        break;
                    case EDataTypes::STRING:
                        status = EExecStatus::EXEC_ERROR_MOVEMUL_STRING_NOT_SUPPORTED;
                        return false; // String multiplication is undefined
                    case EDataTypes::BOOLEAN:
                        if (e_var_final_datatype == EDataTypes::BOOLEAN) {
                            *(bool*)l_var = *(bool*)l_var && e_val.getValue().booleanVal;
                        } else { // We shouldn't be here. Dataypes are inconsistent
                            status = EExecStatus::EXEC_ERROR_MOVEMUL_INCONSISTEND_DATATYPES;
                            return false;
                        }
                        break;
                    case EDataTypes::ARRAY:
                        status = EExecStatus::EXEC_ERROR_MOVEMUL_ARRAY_NOT_SUPPORTED;
                        return false;
                } // ~switch
            } // ~if

            break;
        }
        case EInstrCodes::MOVEDIV: {
            Element e_val = stack.back();
            stack.pop_back();
            Element e_var = stack.back();
            stack.pop_back();

            EDataTypes e_var_datatype = e_var.getDatatype();
            EDataTypes e_var_final_datatype = e_var_datatype == EDataTypes::ADDRESS ? e_var.getValue().addrVal.datatype : e_var_datatype;
            EDataTypes e_val_datatype = e_val.getDatatype();
            EDataTypes e_val_final_datatype = e_val_datatype == EDataTypes::ADDRESS ? e_val.getValue().addrVal.datatype : e_val_datatype;

            if (e_var_datatype != EDataTypes::ADDRESS) {
                status = EExecStatus::EXEC_ERROR_MOVEDIV_EXPECTED_ADDRESS;
                return false;
            }

            if (!isDatatypeConsistentAssignment(e_var_final_datatype, e_val_final_datatype)) {
                status = EExecStatus::EXEC_ERROR_MOVEDIV_INCONSISTEND_DATATYPES;
                return false;
            }

            // Left variable physical address
            void* l_var = e_var.getValue().addrVal.addr;

            // Calculate Right side value
            if (e_val_datatype == EDataTypes::ADDRESS) {
                void* r_val = e_val.getValue().addrVal.addr;

                switch(e_val_final_datatype) {
                    case EDataTypes::INT:
                        if (e_var_final_datatype == EDataTypes::INT) {
                            status = EExecStatus::EXEC_ERROR_MOVEDIV_INCONSISTEND_DATATYPES;
                            return false; // Division cannot be assigned to INT variable
                        } else if (e_var_final_datatype == EDataTypes::FLOAT) { 
                            if (*(long long int*)r_val == 0) {
                                status = EExecStatus::EXEC_ERROR_MOVEDIV_DIV_BY_0;
                                return false; // Div by 0
                            } else {
                                *(long double*)l_var /= *(long long int*)r_val;
                            }
                        } else {
                            status = EExecStatus::EXEC_ERROR_MOVEDIV_INCONSISTEND_DATATYPES;
                            return false; // We shouldn't be here. Dataypes are inconsistent
                        }
                        break;
                    case EDataTypes::FLOAT:
                        if (e_var_final_datatype == EDataTypes::FLOAT) {
                            if (*(long double*)r_val == 0.0) {
                                status = EExecStatus::EXEC_ERROR_MOVEDIV_DIV_BY_0;
                                return false; // Div by 0
                            } else {
                                *(long double*)l_var /= *(long double*)r_val;
                            }
                        } else { // We shouldn't be here. Dataypes are inconsistent
                            status = EExecStatus::EXEC_ERROR_MOVEDIV_INCONSISTEND_DATATYPES;
                            return false;
                        }
                        break;
                    case EDataTypes::STRING:
                        status = EExecStatus::EXEC_ERROR_MOVEDIV_STRING_NOT_SUPPORTED;
                        return false; // String division is undefined
                    case EDataTypes::BOOLEAN:
                        status = EExecStatus::EXEC_ERROR_MOVEDIV_BOOLEAN_NOT_SUPPORTED;
                        return false; // Boolean division is undefined
                    case EDataTypes::ARRAY:
                        status = EExecStatus::EXEC_ERROR_MOVEDIV_ARRAY_NOT_SUPPORTED;
                        return false;
                } // ~switch
            } else { // it's a regular value
                switch(e_val_final_datatype) {
                    case EDataTypes::INT:
                        if (e_var_final_datatype == EDataTypes::INT) {
                            status = EExecStatus::EXEC_ERROR_MOVEDIV_INCONSISTEND_DATATYPES;
                            return false; // Division cannot be assigned to INT variable
                        } else if (e_var_final_datatype == EDataTypes::FLOAT) { 
                            if (e_val.getValue().intVal == 0) {
                                status = EExecStatus::EXEC_ERROR_MOVEDIV_DIV_BY_0;
                                return false; // Div by 0
                            } else {
                                *(long double*)l_var *= e_val.getValue().intVal;
                            }
                        } else {
                            status = EExecStatus::EXEC_ERROR_MOVEDIV_INCONSISTEND_DATATYPES;
                            return false; // We shouldn't be here. Dataypes are inconsistent
                        }
                        break;
                    case EDataTypes::FLOAT:
                        if (e_var_final_datatype == EDataTypes::FLOAT) {
                            if (e_val.getValue().floatVal == 0.0) {
                                status = EExecStatus::EXEC_ERROR_MOVEDIV_DIV_BY_0;
                                return false; // Div by 0
                            } else {
                                *(long double*)l_var *= e_val.getValue().floatVal;
                            }
                        } else { // We shouldn't be here. Dataypes are inconsistent
                            status = EExecStatus::EXEC_ERROR_MOVEDIV_INCONSISTEND_DATATYPES;
                            return false;
                        }
                        break;
                    case EDataTypes::STRING:
                        status = EExecStatus::EXEC_ERROR_MOVEDIV_STRING_NOT_SUPPORTED;
                        return false; // String division is undefined
                    case EDataTypes::BOOLEAN:
                        status = EExecStatus::EXEC_ERROR_MOVEDIV_BOOLEAN_NOT_SUPPORTED;
                        return false; // Boolean division is undefined
                    case EDataTypes::ARRAY:
                        status = EExecStatus::EXEC_ERROR_MOVEDIV_ARRAY_NOT_SUPPORTED;
                        return false;
                } // ~switch
            } // ~if

            break;
        }
        case EInstrCodes::EQUAL: {
            Element e_rval = stack.back();
            stack.pop_back();
            Element e_lval = stack.back();
            stack.pop_back();

            EDataTypes e_lval_datatype = e_lval.getDatatype();
            EDataTypes e_lval_final_datatype = e_lval_datatype == EDataTypes::ADDRESS ? e_lval.getValue().addrVal.datatype : e_lval_datatype;
            EDataTypes e_rval_datatype = e_rval.getDatatype();
            EDataTypes e_rval_final_datatype = e_rval_datatype == EDataTypes::ADDRESS ? e_rval.getValue().addrVal.datatype : e_rval_datatype;

            if (!isDatatypeConsistent(e_lval_final_datatype, e_rval_final_datatype)) {
                status = EExecStatus::EXEC_ERROR_EQUAL_INCONSISTEND_DATATYPES;
                return false;
            }

            // Left variable physical address
            const void* p_l_val = e_lval.getVariablePhysicalAddress();
            // Right variable physical address
            const void* p_r_val = e_rval.getVariablePhysicalAddress();

            if (e_lval_final_datatype == EDataTypes::INT && e_rval_final_datatype == EDataTypes::INT) {
                stack.push_back(Element(*(long long int*)p_l_val == *(long long int*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::FLOAT && e_rval_final_datatype == EDataTypes::FLOAT) {
                stack.push_back(Element(*(long double*)p_l_val == *(long double*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::INT && e_rval_final_datatype == EDataTypes::FLOAT) {
                stack.push_back(Element(*(long long int*)p_l_val == *(long double*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::FLOAT && e_rval_final_datatype == EDataTypes::INT) {
                stack.push_back(Element(*(long double*)p_l_val == *(long long int*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::STRING && e_rval_final_datatype == EDataTypes::STRING) {
                stack.push_back(Element(*(std::string*)p_l_val == *(std::string*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::BOOLEAN && e_rval_final_datatype == EDataTypes::BOOLEAN) {
                stack.push_back(Element(*(bool*)p_l_val == *(bool*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::ARRAY && e_rval_final_datatype == EDataTypes::ARRAY) {
                stack.push_back(Element(*(Array*)p_l_val == *(Array*)p_r_val));
            } else {
                status = EExecStatus::EXEC_ERROR_EQUAL_INCONSISTEND_DATATYPES;
                return false; // Inconsistent datatypes
            }

            break;
        }
        case EInstrCodes::NOTEQUAL: {
            Element e_rval = stack.back();
            stack.pop_back();
            Element e_lval = stack.back();
            stack.pop_back();

            EDataTypes e_lval_datatype = e_lval.getDatatype();
            EDataTypes e_lval_final_datatype = e_lval_datatype == EDataTypes::ADDRESS ? e_lval.getValue().addrVal.datatype : e_lval_datatype;
            EDataTypes e_rval_datatype = e_rval.getDatatype();
            EDataTypes e_rval_final_datatype = e_rval_datatype == EDataTypes::ADDRESS ? e_rval.getValue().addrVal.datatype : e_rval_datatype;

            if (!isDatatypeConsistent(e_lval_final_datatype, e_rval_final_datatype)) {
                status = EExecStatus::EXEC_ERROR_NOTEQUAL_INCONSISTEND_DATATYPES;
                return false;
            }

            // Left variable physical address
            const void* p_l_val = e_lval.getVariablePhysicalAddress();
            // Right variable physical address
            const void* p_r_val = e_rval.getVariablePhysicalAddress();

            if (e_lval_final_datatype == EDataTypes::INT && e_rval_final_datatype == EDataTypes::INT) {
                stack.push_back(Element(*(long long int*)p_l_val != *(long long int*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::FLOAT && e_rval_final_datatype == EDataTypes::FLOAT) {
                stack.push_back(Element(*(long double*)p_l_val != *(long double*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::INT && e_rval_final_datatype == EDataTypes::FLOAT) {
                stack.push_back(Element(*(long long int*)p_l_val != *(long double*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::FLOAT && e_rval_final_datatype == EDataTypes::INT) {
                stack.push_back(Element(*(long double*)p_l_val != *(long long int*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::STRING && e_rval_final_datatype == EDataTypes::STRING) {
                stack.push_back(Element(*(std::string*)p_l_val != *(std::string*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::BOOLEAN && e_rval_final_datatype == EDataTypes::BOOLEAN) {
                stack.push_back(Element(*(bool*)p_l_val != *(bool*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::ARRAY && e_rval_final_datatype == EDataTypes::ARRAY) {
                stack.push_back(Element(*(Array*)p_l_val != *(Array*)p_r_val));
            } else {
                status = EExecStatus::EXEC_ERROR_NOTEQUAL_INCONSISTEND_DATATYPES;
                return false; // Inconsistent datatypes
            }

            break;
        }
        case EInstrCodes::LESSEQUAL: {
            Element e_rval = stack.back();
            stack.pop_back();
            Element e_lval = stack.back();
            stack.pop_back();

            EDataTypes e_lval_datatype = e_lval.getDatatype();
            EDataTypes e_lval_final_datatype = e_lval_datatype == EDataTypes::ADDRESS ? e_lval.getValue().addrVal.datatype : e_lval_datatype;
            EDataTypes e_rval_datatype = e_rval.getDatatype();
            EDataTypes e_rval_final_datatype = e_rval_datatype == EDataTypes::ADDRESS ? e_rval.getValue().addrVal.datatype : e_rval_datatype;

            if (!isDatatypeConsistent(e_lval_final_datatype, e_rval_final_datatype)) {
                status = EExecStatus::EXEC_ERROR_LESSEQUAL_INCONSISTEND_DATATYPES;
                return false;
            }

            // Left variable physical address
            const void* p_l_val = e_lval.getVariablePhysicalAddress();
            // Right variable physical address
            const void* p_r_val = e_rval.getVariablePhysicalAddress();

            if (e_lval_final_datatype == EDataTypes::INT && e_rval_final_datatype == EDataTypes::INT) {
                stack.push_back(Element(*(long long int*)p_l_val <= *(long long int*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::FLOAT && e_rval_final_datatype == EDataTypes::FLOAT) {
                stack.push_back(Element(*(long double*)p_l_val <= *(long double*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::INT && e_rval_final_datatype == EDataTypes::FLOAT) {
                stack.push_back(Element(*(long long int*)p_l_val <= *(long double*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::FLOAT && e_rval_final_datatype == EDataTypes::INT) {
                stack.push_back(Element(*(long double*)p_l_val <= *(long long int*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::STRING && e_rval_final_datatype == EDataTypes::STRING) {
                stack.push_back(Element(*(std::string*)p_l_val <= *(std::string*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::BOOLEAN && e_rval_final_datatype == EDataTypes::BOOLEAN) {
                stack.push_back(Element(*(bool*)p_l_val <= *(bool*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::ARRAY && e_rval_final_datatype == EDataTypes::ARRAY) {
                status = EExecStatus::EXEC_ERROR_LESSEQUAL_ARRAY_NOT_SUPPORTED;
                return false; // Inconsistent datatypes
            } else {
                status = EExecStatus::EXEC_ERROR_LESSEQUAL_INCONSISTEND_DATATYPES;
                return false; // Inconsistent datatypes
            }

            break;
        }
        case EInstrCodes::GREATEREQUAL: {
            Element e_rval = stack.back();
            stack.pop_back();
            Element e_lval = stack.back();
            stack.pop_back();

            EDataTypes e_lval_datatype = e_lval.getDatatype();
            EDataTypes e_lval_final_datatype = e_lval_datatype == EDataTypes::ADDRESS ? e_lval.getValue().addrVal.datatype : e_lval_datatype;
            EDataTypes e_rval_datatype = e_rval.getDatatype();
            EDataTypes e_rval_final_datatype = e_rval_datatype == EDataTypes::ADDRESS ? e_rval.getValue().addrVal.datatype : e_rval_datatype;

            if (!isDatatypeConsistent(e_lval_final_datatype, e_rval_final_datatype)) {
                status = EExecStatus::EXEC_ERROR_GREATEREQUAL_INCONSISTEND_DATATYPES;
                return false;
            }

            // Left variable physical address
            const void* p_l_val = e_lval.getVariablePhysicalAddress();
            // Right variable physical address
            const void* p_r_val = e_rval.getVariablePhysicalAddress();

            if (e_lval_final_datatype == EDataTypes::INT && e_rval_final_datatype == EDataTypes::INT) {
                stack.push_back(Element(*(long long int*)p_l_val >= *(long long int*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::FLOAT && e_rval_final_datatype == EDataTypes::FLOAT) {
                stack.push_back(Element(*(long double*)p_l_val >= *(long double*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::INT && e_rval_final_datatype == EDataTypes::FLOAT) {
                stack.push_back(Element(*(long long int*)p_l_val >= *(long double*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::FLOAT && e_rval_final_datatype == EDataTypes::INT) {
                stack.push_back(Element(*(long double*)p_l_val >= *(long long int*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::STRING && e_rval_final_datatype == EDataTypes::STRING) {
                stack.push_back(Element(*(std::string*)p_l_val >= *(std::string*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::BOOLEAN && e_rval_final_datatype == EDataTypes::BOOLEAN) {
                stack.push_back(Element(*(bool*)p_l_val >= *(bool*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::ARRAY && e_rval_final_datatype == EDataTypes::ARRAY) {
                status = EExecStatus::EXEC_ERROR_GREATEREQUAL_ARRAY_NOT_SUPPORTED;
                return false; // Inconsistent datatypes
            } else {
                status = EExecStatus::EXEC_ERROR_GREATEREQUAL_INCONSISTEND_DATATYPES;
                return false; // Inconsistent datatypes
            }

            break;
        }
        case EInstrCodes::LESS: {
            Element e_rval = stack.back();
            stack.pop_back();
            Element e_lval = stack.back();
            stack.pop_back();

            EDataTypes e_lval_datatype = e_lval.getDatatype();
            EDataTypes e_lval_final_datatype = e_lval_datatype == EDataTypes::ADDRESS ? e_lval.getValue().addrVal.datatype : e_lval_datatype;
            EDataTypes e_rval_datatype = e_rval.getDatatype();
            EDataTypes e_rval_final_datatype = e_rval_datatype == EDataTypes::ADDRESS ? e_rval.getValue().addrVal.datatype : e_rval_datatype;

            if (!isDatatypeConsistent(e_lval_final_datatype, e_rval_final_datatype)) {
                status = EExecStatus::EXEC_ERROR_LESS_INCONSISTEND_DATATYPES;
                return false;
            }

            // Left variable physical address
            const void* p_l_val = e_lval.getVariablePhysicalAddress();
            // Right variable physical address
            const void* p_r_val = e_rval.getVariablePhysicalAddress();

            if (e_lval_final_datatype == EDataTypes::INT && e_rval_final_datatype == EDataTypes::INT) {
                stack.push_back(Element(*(long long int*)p_l_val < *(long long int*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::FLOAT && e_rval_final_datatype == EDataTypes::FLOAT) {
                stack.push_back(Element(*(long double*)p_l_val < *(long double*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::INT && e_rval_final_datatype == EDataTypes::FLOAT) {
                stack.push_back(Element(*(long long int*)p_l_val < *(long double*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::FLOAT && e_rval_final_datatype == EDataTypes::INT) {
                stack.push_back(Element(*(long double*)p_l_val < *(long long int*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::STRING && e_rval_final_datatype == EDataTypes::STRING) {
                stack.push_back(Element(*(std::string*)p_l_val < *(std::string*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::BOOLEAN && e_rval_final_datatype == EDataTypes::BOOLEAN) {
                stack.push_back(Element(*(bool*)p_l_val < *(bool*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::ARRAY && e_rval_final_datatype == EDataTypes::ARRAY) {
                status = EExecStatus::EXEC_ERROR_LESS_ARRAY_NOT_SUPPORTED;
                return false; // Inconsistent datatypes
            } else {
                status = EExecStatus::EXEC_ERROR_LESS_INCONSISTEND_DATATYPES;
                return false; // Inconsistent datatypes
            }

            break;
        }
        case EInstrCodes::GREATER: {
            Element e_rval = stack.back();
            stack.pop_back();
            Element e_lval = stack.back();
            stack.pop_back();

            EDataTypes e_lval_datatype = e_lval.getDatatype();
            EDataTypes e_lval_final_datatype = e_lval_datatype == EDataTypes::ADDRESS ? e_lval.getValue().addrVal.datatype : e_lval_datatype;
            EDataTypes e_rval_datatype = e_rval.getDatatype();
            EDataTypes e_rval_final_datatype = e_rval_datatype == EDataTypes::ADDRESS ? e_rval.getValue().addrVal.datatype : e_rval_datatype;

            if (!isDatatypeConsistent(e_lval_final_datatype, e_rval_final_datatype)) {
                status = EExecStatus::EXEC_ERROR_GREATER_INCONSISTEND_DATATYPES;
                return false;
            }

            // Left variable physical address
            const void* p_l_val = e_lval.getVariablePhysicalAddress();
            // Right variable physical address
            const void* p_r_val = e_rval.getVariablePhysicalAddress();

            if (e_lval_final_datatype == EDataTypes::INT && e_rval_final_datatype == EDataTypes::INT) {
                stack.push_back(Element(*(long long int*)p_l_val > *(long long int*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::FLOAT && e_rval_final_datatype == EDataTypes::FLOAT) {
                stack.push_back(Element(*(long double*)p_l_val > *(long double*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::INT && e_rval_final_datatype == EDataTypes::FLOAT) {
                stack.push_back(Element(*(long long int*)p_l_val > *(long double*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::FLOAT && e_rval_final_datatype == EDataTypes::INT) {
                stack.push_back(Element(*(long double*)p_l_val > *(long long int*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::STRING && e_rval_final_datatype == EDataTypes::STRING) {
                stack.push_back(Element(*(std::string*)p_l_val > *(std::string*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::BOOLEAN && e_rval_final_datatype == EDataTypes::BOOLEAN) {
                stack.push_back(Element(*(bool*)p_l_val > *(bool*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::ARRAY && e_rval_final_datatype == EDataTypes::ARRAY) {
                status = EExecStatus::EXEC_ERROR_GREATER_ARRAY_NOT_SUPPORTED;
                return false; // Inconsistent datatypes
            } else {
                status = EExecStatus::EXEC_ERROR_GREATER_INCONSISTEND_DATATYPES;
                return false; // Inconsistent datatypes
            }

            break;
        }
        case EInstrCodes::JUMPIFFALSE: {
            Element e_val = stack.back(); // Pop the boolean value from the stack
            stack.pop_back();

            EDataTypes e_val_datatype = e_val.getDatatype();
            EDataTypes e_val_final_datatype = e_val_datatype == EDataTypes::ADDRESS ? e_val.getValue().addrVal.datatype : e_val_datatype;

            if (e_val_final_datatype != EDataTypes::BOOLEAN) {
                status = EExecStatus::EXEC_ERROR_JUMPIFFALSE_CONDITION_EXPECTED_BOOLEAN;
                return false;   // The attribute should be of BOOLEAN datatype
            }

            const bool* value;

            if (e_val_datatype == EDataTypes::ADDRESS) {
                value = (const bool*)e_val.getValue().addrVal.addr;
            } else {
                value = &(e_val.getValue().booleanVal);
            }
            
            if (!*value) {
                unsigned int addr;
                unsigned char* paddr = (unsigned char*)&addr;
                *paddr = code[idx++];
                *(paddr+1) = code[idx++];
                *(paddr+2) = code[idx++];
                *(paddr+3) = code[idx++];

                idx = addr;
            } else {
                idx += 4; // Skip the jump address in the bytecode
            }

            break;
        }
        case EInstrCodes::JUMP: {
            unsigned int addr;
            unsigned char* paddr = (unsigned char*)&addr;
            *paddr = code[idx++];
            *(paddr+1) = code[idx++];
            *(paddr+2) = code[idx++];
            *(paddr+3) = code[idx++];

            idx = addr;

            break;
        }
        case EInstrCodes::MUL: {
            Element e_rval = stack.back();
            stack.pop_back();
            Element e_lval = stack.back();
            stack.pop_back();

            EDataTypes e_lval_datatype = e_lval.getDatatype();
            EDataTypes e_lval_final_datatype = e_lval_datatype == EDataTypes::ADDRESS ? e_lval.getValue().addrVal.datatype : e_lval_datatype;
            EDataTypes e_rval_datatype = e_rval.getDatatype();
            EDataTypes e_rval_final_datatype = e_rval_datatype == EDataTypes::ADDRESS ? e_rval.getValue().addrVal.datatype : e_rval_datatype;

            if (!isDatatypeConsistent(e_lval_final_datatype, e_rval_final_datatype)) {
                status = EExecStatus::EXEC_ERROR_MUL_INCONSISTEND_DATATYPES;
                return false;
            }

            // Left variable physical address
            const void* p_l_val = e_lval.getVariablePhysicalAddress();
            // Right variable physical address
            const void* p_r_val = e_rval.getVariablePhysicalAddress();

            if (e_lval_final_datatype == EDataTypes::INT && e_rval_final_datatype == EDataTypes::INT) {
                stack.push_back(Element(*(long long int*)p_l_val * *(long long int*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::FLOAT && e_rval_final_datatype == EDataTypes::FLOAT) {
                stack.push_back(Element(*(long double*)p_l_val * *(long double*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::INT && e_rval_final_datatype == EDataTypes::FLOAT) {
                stack.push_back(Element(*(long long int*)p_l_val * *(long double*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::FLOAT && e_rval_final_datatype == EDataTypes::INT) {
                stack.push_back(Element(*(long double*)p_l_val * *(long long int*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::STRING && e_rval_final_datatype == EDataTypes::STRING) {
                status = EExecStatus::EXEC_ERROR_MUL_STRING_NOT_SUPPORTED;
                return false; // Multiplication not defined for STRING
            } else if (e_lval_final_datatype == EDataTypes::BOOLEAN && e_rval_final_datatype == EDataTypes::BOOLEAN) {
                stack.push_back(Element(*(bool*)p_l_val && *(bool*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::ARRAY && e_rval_final_datatype == EDataTypes::ARRAY) {
                status = EExecStatus::EXEC_ERROR_MULL_ARRAY_NOT_SUPPORTED;
                return false; // Inconsistent datatypes
            } else {
                status = EExecStatus::EXEC_ERROR_MUL_INCONSISTEND_DATATYPES;
                return false; // Inconsistent datatypes
            }

            break;
        }
        case EInstrCodes::DIV: {
            Element e_rval = stack.back();
            stack.pop_back();
            Element e_lval = stack.back();
            stack.pop_back();

            EDataTypes e_lval_datatype = e_lval.getDatatype();
            EDataTypes e_lval_final_datatype = e_lval_datatype == EDataTypes::ADDRESS ? e_lval.getValue().addrVal.datatype : e_lval_datatype;
            EDataTypes e_rval_datatype = e_rval.getDatatype();
            EDataTypes e_rval_final_datatype = e_rval_datatype == EDataTypes::ADDRESS ? e_rval.getValue().addrVal.datatype : e_rval_datatype;

            if (!isDatatypeConsistent(e_lval_final_datatype, e_rval_final_datatype)) {
                status = EExecStatus::EXEC_ERROR_DIV_INCONSISTEND_DATATYPES;
                return false;
            }

            // Left variable physical address
            const void* p_l_val = e_lval.getVariablePhysicalAddress();
            // Right variable physical address
            const void* p_r_val = e_rval.getVariablePhysicalAddress();

            if (e_lval_final_datatype == EDataTypes::INT && e_rval_final_datatype == EDataTypes::INT) {
                if (*(long long int*)p_r_val == 0) {
                    status = EExecStatus::EXEC_ERROR_DIV_DIV_BY_0;
                    return false; // Division by 0
                } else {
                    stack.push_back(Element((long double)*(long long int*)p_l_val / (long double)*(long long int*)p_r_val));
                }
            } else if (e_lval_final_datatype == EDataTypes::FLOAT && e_rval_final_datatype == EDataTypes::FLOAT) {
                if (*(long double*)p_r_val == 0.0) {
                    status = EExecStatus::EXEC_ERROR_DIV_DIV_BY_0;
                    return false; // Division by 0
                } else {
                    stack.push_back(Element(*(long double*)p_l_val / *(long double*)p_r_val));
                }
            } else if (e_lval_final_datatype == EDataTypes::INT && e_rval_final_datatype == EDataTypes::FLOAT) {
                if (*(long double*)p_r_val == 0.0) {
                    status = EExecStatus::EXEC_ERROR_DIV_DIV_BY_0;
                    return false; // Division by 0
                } else {
                    stack.push_back(Element((long double)*(long long int*)p_l_val / *(long double*)p_r_val));
                }
            } else if (e_lval_final_datatype == EDataTypes::FLOAT && e_rval_final_datatype == EDataTypes::INT) {
                if (*(long long int*)p_r_val == 0) {
                    status = EExecStatus::EXEC_ERROR_DIV_DIV_BY_0;
                    return false; // Division by 0
                } else {
                    stack.push_back(Element(*(long double*)p_l_val / (long double)*(long long int*)p_r_val));
                }
            } else if (e_lval_final_datatype == EDataTypes::STRING && e_rval_final_datatype == EDataTypes::STRING) {
                status = EExecStatus::EXEC_ERROR_DIV_STRING_NOT_SUPPORTED;
                return false; // Division not defined for STRING
            } else if (e_lval_final_datatype == EDataTypes::BOOLEAN && e_rval_final_datatype == EDataTypes::BOOLEAN) {
                status = EExecStatus::EXEC_ERROR_DIV_BOOLEAN_NOT_SUPPORTED;
                return false; // division not defined for BOOLEAN
            } else if (e_lval_final_datatype == EDataTypes::ARRAY && e_rval_final_datatype == EDataTypes::ARRAY) {
                status = EExecStatus::EXEC_ERROR_DIV_ARRAY_NOT_SUPPORTED;
                return false; // Inconsistent datatypes
            } else {
                status = EExecStatus::EXEC_ERROR_DIV_INCONSISTEND_DATATYPES;
                return false; // Inconsistent datatypes
            }

            break;
        }
        case EInstrCodes::ADD: {
            Element e_rval = stack.back();
            stack.pop_back();
            Element e_lval = stack.back();
            stack.pop_back();

            EDataTypes e_lval_datatype = e_lval.getDatatype();
            EDataTypes e_lval_final_datatype = e_lval_datatype == EDataTypes::ADDRESS ? e_lval.getValue().addrVal.datatype : e_lval_datatype;
            EDataTypes e_rval_datatype = e_rval.getDatatype();
            EDataTypes e_rval_final_datatype = e_rval_datatype == EDataTypes::ADDRESS ? e_rval.getValue().addrVal.datatype : e_rval_datatype;

            if (!isDatatypeConsistent(e_lval_final_datatype, e_rval_final_datatype)) {
                status = EExecStatus::EXEC_ERROR_ADD_INCONSISTEND_DATATYPES;
                return false;
            }

            // Left variable physical address
            const void* p_l_val = e_lval.getVariablePhysicalAddress();
            // Right variable physical address
            const void* p_r_val = e_rval.getVariablePhysicalAddress();

            if (e_lval_final_datatype == EDataTypes::INT && e_rval_final_datatype == EDataTypes::INT) {
                stack.push_back(Element(*(long long int*)p_l_val + *(long long int*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::FLOAT && e_rval_final_datatype == EDataTypes::FLOAT) {
                stack.push_back(Element(*(long double*)p_l_val + *(long double*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::INT && e_rval_final_datatype == EDataTypes::FLOAT) {
                stack.push_back(Element(*(long long int*)p_l_val + *(long double*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::FLOAT && e_rval_final_datatype == EDataTypes::INT) {
                stack.push_back(Element(*(long double*)p_l_val + *(long long int*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::STRING && e_rval_final_datatype == EDataTypes::STRING) {
                stack.push_back(Element(*(std::string*)p_l_val + *(std::string*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::BOOLEAN && e_rval_final_datatype == EDataTypes::BOOLEAN) {
                stack.push_back(Element(*(bool*)p_l_val || *(bool*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::ARRAY && e_rval_final_datatype == EDataTypes::ARRAY) {
                stack.push_back(Element(*(Array*)p_l_val + *(Array*)p_r_val));
//                status = EExecStatus::EXEC_ERROR_ADD_ARRAY_NOT_SUPPORTED;
//                return false; // Inconsistent datatypes
            } else {
                status = EExecStatus::EXEC_ERROR_ADD_INCONSISTEND_DATATYPES;
                return false; // Inconsistent datatypes
            }

            break;
        }
        case EInstrCodes::SUB: {
            Element e_rval = stack.back();
            stack.pop_back();
            Element e_lval = stack.back();
            stack.pop_back();

            EDataTypes e_lval_datatype = e_lval.getDatatype();
            EDataTypes e_lval_final_datatype = e_lval_datatype == EDataTypes::ADDRESS ? e_lval.getValue().addrVal.datatype : e_lval_datatype;
            EDataTypes e_rval_datatype = e_rval.getDatatype();
            EDataTypes e_rval_final_datatype = e_rval_datatype == EDataTypes::ADDRESS ? e_rval.getValue().addrVal.datatype : e_rval_datatype;

            if (!isDatatypeConsistent(e_lval_final_datatype, e_rval_final_datatype)) {
                status = EExecStatus::EXEC_ERROR_SUB_INCONSISTEND_DATATYPES;
                return false;
            }

            // Left variable physical address
            const void* p_l_val = e_lval.getVariablePhysicalAddress();
            // Right variable physical address
            const void* p_r_val = e_rval.getVariablePhysicalAddress();

            if (e_lval_final_datatype == EDataTypes::INT && e_rval_final_datatype == EDataTypes::INT) {
                stack.push_back(Element(*(long long int*)p_l_val - *(long long int*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::FLOAT && e_rval_final_datatype == EDataTypes::FLOAT) {
                stack.push_back(Element(*(long double*)p_l_val - *(long double*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::INT && e_rval_final_datatype == EDataTypes::FLOAT) {
                stack.push_back(Element(*(long long int*)p_l_val - *(long double*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::FLOAT && e_rval_final_datatype == EDataTypes::INT) {
                stack.push_back(Element(*(long double*)p_l_val - *(long long int*)p_r_val));
            } else if (e_lval_final_datatype == EDataTypes::STRING && e_rval_final_datatype == EDataTypes::STRING) {
                boost::erase_all(*(std::string*)p_l_val, *(std::string*)p_r_val);
                stack.push_back(Element(std::string(*(std::string*)p_l_val))); // p_l_val contains a reduced string
            } else if (e_lval_final_datatype == EDataTypes::BOOLEAN && e_rval_final_datatype == EDataTypes::BOOLEAN) {
                stack.push_back(Element(*(bool*)p_l_val != *(bool*)p_r_val)); // XOR
            } else if (e_lval_final_datatype == EDataTypes::ARRAY && e_rval_final_datatype == EDataTypes::ARRAY) {
                status = EExecStatus::EXEC_ERROR_SUB_ARRAY_NOT_SUPPORTED;
                return false; // Inconsistent datatypes
            } else {
                status = EExecStatus::EXEC_ERROR_SUB_INCONSISTEND_DATATYPES;
                return false; // Inconsistent datatypes
            }

            break;
        }
        case EInstrCodes::NEG: {
            Element e_lval = stack.back();
            stack.pop_back();

            EDataTypes e_lval_datatype = e_lval.getDatatype();
            EDataTypes e_lval_final_datatype = e_lval_datatype == EDataTypes::ADDRESS ? e_lval.getValue().addrVal.datatype : e_lval_datatype;

            // Left variable physical address
            const void* p_l_val = e_lval.getVariablePhysicalAddress();

            if (e_lval_final_datatype == EDataTypes::INT) {
                stack.push_back(Element(- *(long long int*)p_l_val));
            } else if (e_lval_final_datatype == EDataTypes::FLOAT) {
                stack.push_back(Element(- *(long double*)p_l_val));
            } else if (e_lval_final_datatype == EDataTypes::STRING) {
                status = EExecStatus::EXEC_ERROR_NEG_STRING_NOT_SUPPORTED;
                return false; // Negation not defined for STRING
            } else if (e_lval_final_datatype == EDataTypes::BOOLEAN) {
                stack.push_back(Element(! *(bool*)p_l_val));
            } else if (e_lval_final_datatype == EDataTypes::ARRAY) {
                status = EExecStatus::EXEC_ERROR_NEG_ARRAY_NOT_SUPPORTED;
                return false; // Inconsistent datatypes
            } else {
                status = EExecStatus::EXEC_ERROR_NEG_INCONSISTEND_DATATYPES;
                return false; // Inconsistent datatypes
            }

            break;
        }
        case EInstrCodes::END: {
            status = EExecStatus::OK_STOP;
            return false;
        }
        case EInstrCodes::CALL: {   // Function index from the DATA section (1 unsigned int - 4 bytes) 
            unsigned int var_idx;
            unsigned char* pvar_idx = (unsigned char*)&var_idx;
            *pvar_idx = code[idx++];
            *(pvar_idx+1) = code[idx++];
            *(pvar_idx+2) = code[idx++];
            *(pvar_idx+3) = code[idx++];

            Datatype& variable = bytecode.getVariables()[var_idx];

            callstack.push_back(new CallStackEntry(idx));
            idx = variable.getFunRef();

            break;
        }
        case EInstrCodes::SYSCALL: { // The system sunction name: String literal (the array of ushort (2 bytes) wide character string (utf-16) + ending 0)
            // Parameter value
            //----------------
            unsigned int var_idx;
            unsigned char* pvar_idx = (unsigned char*)&var_idx;
            *pvar_idx = code[idx++];
            *(pvar_idx+1) = code[idx++];
            *(pvar_idx+2) = code[idx++];
            *(pvar_idx+3) = code[idx++];

            Datatype& variable = bytecode.getVariables()[var_idx];

            // Take the parameter variable idx on the callstack
            unsigned int callstack_pos = variable.getCallStackPos();

            // Take the function variable from the callstack
            CallStackEntry* callStackEntry = callstack.back();

            CallStackValuePointer* call_stack_var = callStackEntry->getVariable(callstack_pos); // Get a physical address of a variable with index var_idx 
            void* par_variable =  call_stack_var->value;

            // Get the value and put to the stack
            EDataTypes variable_datatype = decodeDatatype(variable);

            // Function name
            //--------------
            unsigned short c;
            std::string sys_fun_name;

            do {
                c = code[idx++];

                if (c == 0) {
                    break;
                }
                sys_fun_name += c;
            } while(true);

            if (sys_fun_name == "sin") {
                long double ret_val = 0.0;
                
                if (variable_datatype == EDataTypes::INT) {
                    ret_val = sin((long double)(*(long long int*)par_variable));
                } else if (variable_datatype == EDataTypes::FLOAT) {
                    ret_val = sin(*(long double*)par_variable);
                } else if (variable_datatype == EDataTypes::STRING) {
                    status = EExecStatus::EXEC_ERROR_SYSCALL_SIN_STRING_NOT_SUPPORTED;
                    return false; // sin not defined for STRING
                } else if (variable_datatype == EDataTypes::BOOLEAN) {
                    status = EExecStatus::EXEC_ERROR_SYSCALL_SIN_BOOLEAN_NOT_SUPPORTED;
                    return false; // sin not defined for BOOLEAN
                } else {
                    status = EExecStatus::EXEC_ERROR_SYSCALL_SIN_INCONSISTENT_DATATYPES;
                    return false; // Inconsistent datatypes
                }

                stack.push_back(Element(ret_val));
            } else if (sys_fun_name == "cos") {
                long double ret_val = 0.0;
                
                if (variable_datatype == EDataTypes::INT) {
                    ret_val = cos((long double)(*(long long int*)par_variable));
                } else if (variable_datatype == EDataTypes::FLOAT) {
                    ret_val = cos(*(long double*)par_variable);
                } else if (variable_datatype == EDataTypes::STRING) {
                    status = EExecStatus::EXEC_ERROR_SYSCALL_COS_STRING_NOT_SUPPORTED;
                    return false; // sin not defined for STRING
                } else if (variable_datatype == EDataTypes::BOOLEAN) {
                    status = EExecStatus::EXEC_ERROR_SYSCALL_COS_BOOLEAN_NOT_SUPPORTED;
                    return false; // sin not defined for BOOLEAN
                } else {
                    status = EExecStatus::EXEC_ERROR_SYSCALL_COS_INCONSISTENT_DATATYPES;
                    return false; // Inconsistent datatypes
                }

                stack.push_back(Element(ret_val));
            } else {
                status = EExecStatus::EXEC_ERROR_SYSCALL_UNKNOWN_FUNCTION;
                return false;   // Unknown system function
            }

            break;
        }
        case EInstrCodes::RETURN: {// No attributes
            if (callstack.size() > 0) {
                // Take the function variable from the DATA section
                unsigned int var_idx;
                unsigned char* pvar_idx = (unsigned char*)&var_idx;
                *pvar_idx = code[idx++];
                *(pvar_idx+1) = code[idx++];
                *(pvar_idx+2) = code[idx++];
                *(pvar_idx+3) = code[idx++];

                Datatype& variable = bytecode.getVariables()[var_idx];

                // Take the function variable idx on the callstack
                unsigned int callstack_pos = variable.getCallStackPos();

                // Take the function variable from the callstack
                CallStackEntry* callStackEntry = callstack.back();
                callstack.pop_back();

                CallStackValuePointer* call_stack_var = callStackEntry->getVariable(callstack_pos); // Get a physical address of a variable with index var_idx 
                void* fun_variable =  call_stack_var->value;

                // Get the value and put to the stack
                EDataTypes variable_datatype = decodeDatatype(variable);

                switch (variable_datatype) {
                    case EDataTypes::INT: {
                        stack.push_back(Element(*(long long int*)fun_variable));
                        break;
                    }
                    case EDataTypes::FLOAT: {
                        stack.push_back(Element(*(long double*)fun_variable));
                        break;
                    }
                    case EDataTypes::STRING: {
                        stack.push_back(Element(*(std::string*)fun_variable));
                        break;
                    }
                    case EDataTypes::BOOLEAN: {
                        stack.push_back(Element(*(bool*)fun_variable));
                        break;
                    }
                    case EDataTypes::ARRAY: {
                        stack.push_back(Element(*(Array*)fun_variable));
                        break;
                    }
                } // ~switch

                // Deallocate the callctack
                idx = callStackEntry->getReturnIdx();
                delete callStackEntry;
            } else {
                status = EExecStatus::EXEC_ERROR_RETURN_NO_RETURN_POINT;
                return false; // no return point for RETURN
            }

            break;
        }
        case EInstrCodes::INITVAR: {
            unsigned int var_idx;
            unsigned char* pvar_idx = (unsigned char*)&var_idx;
            *pvar_idx = code[idx++];
            *(pvar_idx+1) = code[idx++];
            *(pvar_idx+2) = code[idx++];
            *(pvar_idx+3) = code[idx++];

            Datatype& variable = bytecode.getVariables()[var_idx];
            variable.setToDefault();

            break;
        }

        case EInstrCodes::ALLOCVAR: {
            unsigned int var_idx;
            unsigned char* pvar_idx = (unsigned char*)&var_idx;

            *pvar_idx = code[idx++];
            *(pvar_idx+1) = code[idx++];
            *(pvar_idx+2) = code[idx++];
            *(pvar_idx+3) = code[idx++];

            Datatype& variable = bytecode.getVariables()[var_idx];

            void* p = variable.makeDynamicAddress();

            CallStackEntry* callStackEntry = callstack.back();
            unsigned int var_pos_on_callstack = callStackEntry->addVariable(decodeDatatype(variable), p);

            variable.setCallStackPos(var_pos_on_callstack); // register the dynamic variable position in the DATA section reference variable

            break;
        }

        case EInstrCodes::ALLOCVARS: {
            unsigned int var_idx;
            unsigned char* pvar_idx = (unsigned char*)&var_idx;

            *pvar_idx = code[idx++];
            *(pvar_idx+1) = code[idx++];
            *(pvar_idx+2) = code[idx++];
            *(pvar_idx+3) = code[idx++];

            Datatype& variable = bytecode.getVariables()[var_idx];

            void* l_var = variable.makeDynamicAddress();

            CallStackEntry* callStackEntry = callstack.back();
            unsigned int var_pos_on_callstack = callStackEntry->addVariable(decodeDatatype(variable), l_var);

            variable.setCallStackPos(var_pos_on_callstack); // register the dynamic variable position in the DATA section reference variable

            // take the value from the stack and put to the variable
            Element e_val = stack.back();
            stack.pop_back();

            EDataTypes e_val_datatype = e_val.getDatatype();
            EDataTypes e_val_final_datatype = e_val_datatype == EDataTypes::ADDRESS ? e_val.getValue().addrVal.datatype : e_val_datatype;

            EDataTypes e_var_final_datatype = decodeDatatype(variable);

           if ( !moveValue(l_var, e_var_final_datatype, e_val, e_val_datatype, e_val_final_datatype, status) ) {
               return false;
           }

            break;
        }

        case EInstrCodes::PUTDADDR: {
            unsigned int var_idx;
            unsigned char* pvar_idx = (unsigned char*)&var_idx;
            *pvar_idx = code[idx++];
            *(pvar_idx+1) = code[idx++];
            *(pvar_idx+2) = code[idx++];
            *(pvar_idx+3) = code[idx++];

            Datatype& variable = bytecode.getVariables()[var_idx];
            unsigned int callstack_pos = variable.getCallStackPos();

            CallStackEntry* callStackEntry = callstack.back();
            
            CallStackValuePointer* call_stack_var = callStackEntry->getVariable(callstack_pos); // Get a physical address of a variable with index var_idx 
            void* addr = call_stack_var->value;

            EDataTypes stack_datatype = call_stack_var->datatype;

            stack.push_back(Element(stack_datatype, addr));
            break;
        }

    } // ~switch

    return true;
}
