#include "bytecode.h"


/*******************************************
 * class Datatype
 *******************************************/
Datatype::Datatype() : variableType(EVariableTypes::UNDEFINED), name(NULL), scope(NULL), datatype(NULL), funParamDatatype(NULL), address(NULL), function_ref(-1), dynamic_idx(-1) {}
Datatype::Datatype(const char* name, const char* scope, const char* datatype, EVariableTypes variableType) : variableType(variableType), name(NULL), scope(NULL), datatype(NULL), funParamDatatype(NULL), address(NULL), function_ref(-1), dynamic_idx(-1) {
    setName(name);
    setScope(scope);
    setDatatype(datatype);
}
Datatype::Datatype(const char* name, const char* scope, const char* datatype, unsigned int dynamic_idx, EVariableTypes variableType) : 
                                    variableType(variableType), name(NULL), scope(NULL), datatype(NULL), funParamDatatype(NULL), address(NULL), function_ref(-1), dynamic_idx(dynamic_idx) {
    setName(name);
    setScope(scope);
    setDatatype(datatype);
}
Datatype::Datatype(const char* name, const char* scope, const char* datatype, const char* funParamDatatype, EVariableTypes variableType, int function_ref) : 
                                    variableType(variableType), name(NULL), scope(NULL), datatype(NULL), funParamDatatype(NULL), address(NULL), function_ref(function_ref), dynamic_idx(-1) {
    setName(name);
    setScope(scope);
    setDatatype(datatype);
    setFunParamDatatype(funParamDatatype);
}
Datatype::Datatype(const Datatype& other) : name(NULL), scope(NULL), datatype(NULL), funParamDatatype(NULL), address(NULL) {
    variableType = other.variableType;
    function_ref = other.function_ref;
    setName(other.getName());
    setScope(other.getScope());
    setDatatype(other.getDatatype());
    setFunParamDatatype(other.getFunParamDatatype());
    dynamic_idx = other.dynamic_idx;
    // TODO: handle copying 'address'
}

Datatype::~Datatype() {
    free(name);
    free(scope);
    free(datatype);
    free(funParamDatatype);
    free(address);
}

bool Datatype::operator==(const Datatype& other) const {
// TODO: don't we have to consider name and variableType
    return datatype == other.getDatatype();
}

void Datatype::setName(const char* name) {
    if (name != NULL) {
        size_t new_size = strlen(name)+1;
        this->name = (char*)realloc((void*)this->name, new_size);
        strcpy_s(this->name, new_size, name);
    }
}

void Datatype::setScope(const char* scope) {
    if (scope != NULL) {
        size_t new_size = strlen(scope)+1;
        this->scope = (char*)realloc((void*)this->scope, new_size);
        strcpy_s(this->scope, new_size, scope);
    }
}

void Datatype::setDatatype(const char* datatype) {
    if (datatype != NULL) {
        size_t new_size = strlen(datatype)+1;
        this->datatype = (char*)realloc((void*)this->datatype, new_size);
        strcpy_s(this->datatype, new_size, datatype);
    }
}

void Datatype::setFunParamDatatype(const char* funParamDatatype) {
    if (funParamDatatype != NULL) {
        size_t new_size = strlen(funParamDatatype)+1;
        this->funParamDatatype = (char*)realloc((void*)this->funParamDatatype, new_size);
        strcpy_s(this->funParamDatatype, new_size, funParamDatatype);
    }
}

const char* Datatype::getName() const {
    return name;
}

const char* Datatype::getScope() const {
    return scope;
}

const char* Datatype::getDatatype() const {
    return datatype;
}

const char* Datatype::getFunParamDatatype() const {
    return funParamDatatype;
}

enum Datatype::EVariableTypes Datatype::getVariableType() const {
    return variableType;
}

void* Datatype::makeDynamicAddress() {
    void* ret = NULL;

    if (datatype[0] == 'i') {
        ret = new long long int;
    } else if (datatype[0] == 'f') {
        ret = new long double;
    } else if (datatype[0] == 's') {
        ret = new std::string;
    } else if (datatype[0] == 'b') {
        ret = new unsigned char;
    } else if (datatype[0] == 'a') {
        std::vector<char> index_datatypes;
        int i;
        // Get index datatypes
        for(i=3; i<strlen(datatype); i++) {
            unsigned char c = datatype[i];

            if (c == ']') {
                i+=2; // Skip ']' and following ' '
                break;
            }
            if (c == ',') continue;
            index_datatypes.push_back(c);
        }
        // Get element datatype
        std::string element_datatype = std::string(datatype+i);
        ret = new Array(index_datatypes, element_datatype);
    }

    return ret;
}

void* Datatype::makeAddress() {
    address = makeDynamicAddress();
    setToDefault();

    return address;
}

void Datatype::setToDefault() {
    if (address != NULL) {
        if (datatype[0] == 'i') {
            *(long long int*)address = 0;
        } else if (datatype[0] == 'f') {
            *(long double*)address = 0.0;
        } else if (datatype[0] == 's') {
            (*(std::string*)address) = "";
        } else if (datatype[0] == 'b') {
            *(unsigned char*)address = 1; // true
        } else if (datatype[0] == 'a') {
            (*(Array*)address).clearElements();
        }
    }
}

void* Datatype::getAddress() {
    return address;
}

void Datatype::disposeAddress() {
    if (strcmp(datatype, "i") == 0) {
        delete static_cast<long long int*>(address);
    } else if (strcmp(datatype, "f") == 0) {
        delete static_cast<long double*>(address);
    } else if (strcmp(datatype, "s") == 0) {
        delete static_cast<std::string*>(address);
    } else if (strcmp(datatype, "b") == 0) {
        delete static_cast<unsigned char*>(address);
    } else if (strcmp(datatype, "a") == 0) {
        delete static_cast<Array*>(address);
    }

    address = NULL;
}

void Datatype::printVariable()
{
    if (variableType == EVariableTypes::VARIABLE) {
        if (datatype[0] == 'i') {
            printf("[%s]: %lld\n", datatype, *static_cast<long long int*>(address));
        } else if (datatype[0] == 'f') {
            printf("[%s]: %LF\n", datatype, *static_cast<long double*>(address));
        } else if (datatype[0] == 's') {
            printf("[%s]: %s\n", datatype, (*static_cast<std::string*>(address)).data());
        } else if (datatype[0] == 'b') {
            printf("[%s]: %u\n", datatype, (unsigned int)*static_cast<unsigned char*>(address));
        } else if (datatype[0] == 'a') {
            printf("[%s]: %s\n", datatype, (*static_cast<Array*>(address)).toString().data());
        }
    }
}

unsigned int Datatype::getFunRef() const {
    return function_ref;
}

unsigned int Datatype::getDynamicIdx() const {
    return dynamic_idx;
}

void Datatype::setCallStackPos(unsigned int callstack_pos) {
    this->callstack_pos = callstack_pos;
}

unsigned int Datatype::getCallStackPos() {
    return callstack_pos;
}


Bytecode::Bytecode()
{
}
