#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>

#include "bytecode.h"

enum class EParseStatus : int
{
    PARSE_OK                            = 0,
    PARSE_ERROR                         = 1,
    PARSE_ERROR_COMPILATION_UNIT        = 2,
    PARSE_ERROR_ASSIGNMENT_EXPRESSION   = 3,
    PARSE_ERROR_ASSIGNMENT_OPERATOR     = 4,
    PARSE_ERROR_CONDITIONAL_EXPRESSION  = 5,
    PARSE_ERROR_LOGICAL_OR_EXPRESSION   = 6,
    PARSE_ERROR_COND_EXP_MISSING_COLON  = 7,
    PARSE_ERROR_LOGICAL_AND_EXPRESSION  = 8, 
    PARSE_ERROR_EQUALITY_OPERATOR       = 9,    // TODO: remove it
    PARSE_ERROR_EQUALITY_EXPRESSION     = 10,   // TODO: remove it
    PARSE_ERROR_RELATIONAL_EXPRESSION   = 11,
    PARSE_ERROR_RELATION_OPERATOR       = 12,
    PARSE_ERROR_ADDITIVE_EXPRESSION     = 13,
    PARSE_ERROR_ADDITIVE_OPERATOR       = 14,
    PARSE_ERROR_MULT_EXPRESSION         = 15,
    PARSE_ERROR_MULT_OPERATOR           = 16,
    PARSE_ERROR_UNARY_EXPRESSION        = 17,
    PARSE_ERROR_UNARY_OPERATOR          = 18,
    PARSE_ERROR_POSTFIX_EXPRESSION      = 19,
    PARSE_ERROR_ARRAY_INDEX             = 20,
    PARSE_ERROR_ARGUMENT_LIST           = 21,
    PARSE_ERROR_ARGUMENTLIST_EXPRESSION = 22,
    PARSE_ERROR_IDENTIFIER              = 23,
    PARSE_ERROR_PRIMARY_EXPRESSION      = 24,
    PARSE_ERROR_STRING_EXPRESSION       = 25,
    PARSE_ERROR_UNSIGNED_INTEGER        = 26,
    PARSE_ERROR_UNSIGNED_FLOAT          = 27,
    PARSE_ERROR_VARIABLE_DECLARATION            = 28,
    PARSE_ERROR_DUPLICATE_VARIABLE_DECLARATION  = 29,
    PARSE_ERROR_TYPE_DECLARATION                = 30,
    PARSE_ERROR_INDEX_TYPE_DECLARATION          = 31,
    PARSE_ERROR_IDENTIFIER_RESERVED_WORD        = 32,
    PARSE_ERROR_ASSIGNMENT_DATATYPE             = 33,
    PARSE_ERROR_UNKNOWN_IDENTIFIER              = 34,
    PARSE_ERROR_ARRAY_DATATYPE                  = 35,
    PARSE_ERROR_DATATYPE_MISMATCH               = 36,
    PARSE_ERROR_CONDITIONAL_EXPRESSION_STATEMENTS_DATATYPES = 37,
    PARSE_ERROR_CONDITIONAL_EXPRESSION_CONDITION_DATATYPE   = 38,
    PARSE_ERROR_EXPECTED_BOOLEAN_DATATYPE                   = 39,
    PARSE_ERROR_CONDITIONAL_EXPRESSION_FALSE                = 40,
    PARSE_ERROR_CONDITIONAL_EXPRESSION_TRUE                 = 41,
    PARSE_ERROR_BOOLEAN_VALUE                               = 42,
    PARSE_ERROR_LITERALS                                    = 43,
    PARSE_ERROR_UNRECOGNIZED_IDENTIFIER_TYPE                = 44,
    PARSE_ERROR_MISSING_FUNCTION_ARGUMENT_LIST              = 45,
    PARSE_ERROR_INCOMPATIBLE_FUNCTION_ARGUMENTS             = 46,
    PARSE_ERROR_BLOCK_STATEMENT                             = 47,
    PARSE_ERROR_ARGUMENT_LIST_DECLARATION                   = 48,
    PARSE_ERROR_FUNCTION_TYPE_DECLARATION                   = 49,
    PARSE_ERROR_FUNCTION_NAME_IN_USE                        = 50,
    PARSE_ERROR_FUNCTION_DEFINITION                         = 51,
    PARSE_ERROR_FUNCTION_PARAMETERS_DECLARATION             = 52,
    PARSE_ERROR_TYPE_DECLARATION_MISSING_OF                 = 53,
    PARSE_ERROR_TYPE_DECLARATION_MISSING_CLOSING_BRACKET    = 54,
    PARSE_ERROR_TYPE_DECLARATION_MISSING_OPENING_BRACKET    = 55,
    PARSE_ERROR_PRIMARY_EXPRESSION_MISSING_CLOSING_BRACKET  = 56
};


enum class EAction {
    CONTINUE, STOP
};
extern const char* parseStatusDescription[];

enum EAssignmentPosition { LEFT, RIGHT };
enum EIDMode { DECLARATION, EVALUATION }; // used to evaluate the identifier datatype -> DECLARATION - taken from the datatype declaration, EVALUATION - taken from the 'variables' array.

enum class RetVal {
    OK, FAIL_CONTINUE, FAIL_STOP
};

/*
int x
array [int,int] of int y

a.b[c+1].d = 10.1+a.b[c+2].d

VAR x int
VAR y array int int - int

PUTADDR a           ; stack: [addr_a]                       ; a={b[]=[{d=1},{d=2},{d=3},{d=4}]}, c=2
PUTADDR S, b        ; stack: [addr_a.b]               
PUTADDR c           ; stack: [addr_a.b, addr_c]
PUT 1               ; stack: [addr_a.b, addr_c, 1]
ADD ADDR, VAL       ; stack: [addr_a.b, 3]
PUTADDR S, IND, S   ; stack: [addr_a.b[3] ]
PUTADDR S, d        ; stack: [addr_a.b[3].d]
PUT 10.1            ; stack: [addr_a.b[3].d, 10 ]

PUTADDR a           ; stack: [addr_a.b[3].d, 10.1, addr_a ]
PUTADDR S, b        ; stack: [addr_a.b[3].d, 10.1, addr_a.b ]
PUTADDR c           ; stack: [addr_a.b[3].d, 10.1, addr_a.b, addr_c ]
PUT 2               ; stack: [addr_a.b[3].d, 10.1, addr_a.b, addr_c, 2 ]
ADD ADDR, VAL       ; stack: [addr_a.b[3].d, 10.1, addr_a.b, 4 ]
PUTADDR S, IND, S   ; stack: [addr_a.b[3].d, 10.1, addr_a.b[4] ]
PUTADDR S, d        ; stack: [addr_a.b[3].d, 10.1, addr_a.b[4].d ]
ADD VAL, ADDR       ; stack: [addr_a.b[3].d, 14.1 ]

MOV S, ADDR         ; stack: []                             ; a.b[3].d = 14.1
*/

//enum ETokenizeStatus
//{
//    TOKENIZE_OK,
//    TOKENIZE_ERROR
//};

//enum EToken
//{

//};

//class Token
//{
//    EToken token;
//};

class CodeLocation {
public:
    CodeLocation() {}
    CodeLocation(int pos, int row, int col) : pos(pos), row(row), col(col) {}
    int pos{ -1 };
    int row{ -1 };
    int col{ -1 };

    void set(int pos, int row, int col) {
        this->pos = pos;
        this->row = row;
        this->col = col;
    }

    CodeLocation operator+(int inc) {
        return CodeLocation(pos + inc, col + inc, row);
    }
};

class ParseTrace
{
public:
    ParseTrace() : status(EParseStatus::PARSE_OK) {}
    ParseTrace(const CodeLocation& pos, EParseStatus status) : pos(pos), status(status) {
    }
    ~ParseTrace() {
        // qDebug() << "~ParseTrace:" << pos << status;
    }

    void clear() {
        pos.pos = -1;
        pos.col = -1;
        pos.row = -1;

        confidence = -1;

        status = EParseStatus::PARSE_OK;
        suberrors.clear();
    }

    std::string _getString(int level) const {
        std::string ret = std::string(level, ' ') 
            + "[" + std::to_string(pos.pos) 
            + "; (" + std::to_string(pos.row + 1) 
            + ", " + std::to_string(pos.col + 1) 
            + "), " + std::to_string(confidence)
            + "] " + parseStatusDescription[static_cast<int>(status)] + "\n";

        for(const ParseTrace &obj: suberrors) {
            ret.append(obj._getString(level+1));
        }

        return ret;
    }

    std::string getString() const {
        return _getString(0);
    }

public:
    CodeLocation pos;
    EParseStatus status;
    int confidence{ -1 };
    std::vector<ParseTrace> suberrors;
};

class Scope {
private:
    std::vector<std::string> scope;

public:
    Scope() {}
    Scope(const std::string& s) {
        scope.push_back(s);
    }

    Scope(const std::vector<std::string>& scope) : scope(scope) {
    }

    Scope(const Scope& other) : scope(other.scope) {
    }

    Scope(const Scope& parent_scope, const std::string& suffix) {
        scope = parent_scope.scope;
        scope.push_back(suffix);
    }

    unsigned int size() const {
        return scope.size();
    }

    void add(const std::string& s) {
        scope.push_back(s);
    }

    bool operator==(const Scope& other) const {
        return scope == other.scope;
    }

    Scope& operator=(const Scope& other) {
        scope = other.scope;
        return *this;
    }

    void reduce() {
        if (!scope.empty()) {
            scope.pop_back();
        }
    }

    bool startsWith(const Scope& other) const {
        if (scope.size() < other.scope.size()) {
            return false;
        }

        for(int i=0; i<other.size(); i++) {
            if (scope[i] != other.scope[i]) {
                return false;
            }
        }

        return true;
    }

    const std::string toString() const {
        std::string ret;
        bool is_first = true;

        for(const std::string& s: scope) {
            if (is_first) {
                ret = s;
                is_first = false;
            } else {
                ret += "." + s;
            }
        }

        return ret;
    }

    bool isRoot() const {
        return (scope.size() == 1 && scope.at(0) == "0");
    }
};

class Variable {
public:
    enum class EVariableTypes { UNDEFINED, BUILTIN_VARIABLE, VARIABLE, DYNAMIC_VARIABLE, BUILTIN_FUNCTION, FUNCTION };

private:
    enum EVariableTypes entityType;
    Scope scope;
    std::string name;
    std::string type;
    std::string type_fun_params;
    CodeLocation code_position;
    int variable_idx;
    int function_ref;

public:
    Variable() : entityType(EVariableTypes::UNDEFINED), name(""), type(""), type_fun_params(""), variable_idx(-1), function_ref(-1) {}
    Variable(const Variable& other) :  entityType(other.entityType),
                                        scope(other.scope),
                                        name(other.name),
                                        type(other.type),
                                        type_fun_params(other.type_fun_params),
                                        code_position(other.code_position),
                                        variable_idx(other.variable_idx),
                                        function_ref(other.function_ref)
    {
    }
    Variable(EVariableTypes entityType, const Scope& scope, const std::string& name, const std::string& type, const std::string& type_fun_params, CodeLocation code_position) :
        entityType(entityType), scope(scope), name(name), type(type), type_fun_params(type_fun_params), code_position(code_position), variable_idx(-1), function_ref(-1) {
    }

    Variable& operator=(const Variable& other) {
        entityType = other.entityType;
        scope = other.scope;
        name = other.name;
        type = other.type;
        type_fun_params = other.type_fun_params;
        code_position = other.code_position;
        variable_idx = other.variable_idx;
        function_ref = other.function_ref;

        return *this;
    }
    void setEntityType(enum EVariableTypes entityType) { this->entityType = entityType; }
    enum EVariableTypes getEntityType() const { return entityType; }
    const Scope& getScope() const { return scope; }
    const std::string& getName() const { return name; }
    const std::string& getType() const { 
        return type;
    }
    const std::string& getType_fun_params() const { 
        return type_fun_params;
    }
    const CodeLocation& getPosition() const { return code_position; }
    int getIdx() const { return variable_idx; }
    void setIdx(int idx) { variable_idx = idx; }
    void setFunctionRef(int function_ref) { this->function_ref = function_ref; }
    int getFunctionRef() const { return function_ref; }

    void reallocateFunctionRef(unsigned int new_pos) {
        function_ref += new_pos;
    }

    bool operator==(const Variable& other) const {
        return (scope == other.scope) && (name == other.name);
    }
};

class Variables {
private:
    std::vector<Variable> variables;
    std::string UNKNOWN_DATATYPE = "-1";

public:
    enum class EScopeRange {EXACT, INHERITANCE};

    void clear() {
        variables.clear();
    }

    const std::vector<Variable>& getVariables() {
        return variables;
    }
    
    bool add(const Variable& v, unsigned int& pos) {
        std::vector<Variable>::iterator iter = std::find(variables.begin(), variables.end(), v);

        if (iter == variables.end()) {
            variables.push_back(v);
            pos = variables.size()-1;
            variables[pos].setIdx(pos);
            return true;
        } else {
            pos = iter->getIdx();
            return false;
        }
    }

    const std::string& getDatatype(const std::string& v) {
        for (int i=0; i<variables.size(); i++) {
            if (variables.at(i).getName() == v) {
                return variables.at(i).getType();
            }
        }

        return UNKNOWN_DATATYPE;
    }

    // Looks for a variable 'name' visible from the 'scope'
    // scopeRange == EScopeRange::EXACT
    //      looks for exact match in the given scope. Usefull to check if varaible declaration hasn't been doubled
    // scopeRange == EScopeRange::INHERITANCE
    //      looks for the variable in the highest visible scope
    // Eg. Looking for "x" while in the "0.1.2" scope.
    //     There are 0.x, 0.1.x
    //     0.1.x will be returned
    bool getVariable(const Scope& scope, EScopeRange scopeRange, const std::string& name, Variable& var) {
        Scope DUMMY_SCOPE;
        const Scope* max_scope = &DUMMY_SCOPE;
        int found_idx = -1;

        // // Scope reduction
        // // scope = 0123.. => 01

        // std::string reduced_scope;
        // int first_dot_pos = scope.indexOf('.');

        // if (first_dot_pos != -1) {
        //     int no_dots = scope.size() - first_dot_pos;
        //     int reduced_scope_len = first_dot_pos - no_dots;

        //     if (reduced_scope_len < 1) {
        //         var = Variable(Variable::EVariableTypes::UNDEFINED, "", name, "", "", -1);
        //         return false;
        //     }

        //     reduced_scope = scope.left(reduced_scope_len);
        // } else {
        //     reduced_scope = scope;
        // }

        if (scopeRange == EScopeRange::EXACT) {
            for (int i=0; i<variables.size(); i++) {
                if (variables.at(i).getName() == name && variables.at(i).getScope() == scope) {
                    var = variables[i];
                    return true; 
                }
            }
        } else {    // EScopeRange::INHERITANCE
            for (int i=0; i<variables.size(); i++) {
                if (variables.at(i).getName() == name) {
                    const Scope* cur_scope = &(variables[i].getScope());

                    if (scope.startsWith(*cur_scope) ) {
                        if (cur_scope->size() > max_scope->size()) {
                            max_scope = cur_scope;
                            found_idx = i;
                        }
                    }
                }
            }
        }

        if (found_idx != -1) {
            var = variables[found_idx];
            return true;
        } else {
            var = Variable(Variable::EVariableTypes::UNDEFINED, DUMMY_SCOPE, name, "", "", CodeLocation());
            return false;
        }
    }

    // idx - the function refernce index in Variables table
    // function_ref - function's position (address) in the code
    void setFunctionRef(int idx, int function_ref) {
        if (idx < variables.size()) {
            variables[idx].setFunctionRef(function_ref);
        }
    }

    // void reallocateFunctionRef(unsigned int new_pos) {
    //     for (int i=0; i<variables.size(); i++) {
    //         if (variables[i].getEntityType() == Variable::EVariableTypes::FUNCTION
    //         || variables[i].getEntityType() == Variable::EVariableTypes::BUILTIN_FUNCTION
    //         ) {
    //             variables[i].reallocateFunctionRef(new_pos);
    //         }
    //     }
    // }

    void updateFunctionRefs(const Bytecode& bytecode) {
        for(FunctionRef f: bytecode.getFunctionRefs()) {
            variables[f.variable_index].setFunctionRef(f.function_pos);
        }
    }

};

class Parser
{
public:
    enum EArithOperators {ADD, SUB, MUL, DIV};
    enum class EExtendedAssignOperation { NONE, ADD, SUB, MUL, DIV };
    enum class EDataMode {STATIC, DYNAMIC};

    Parser();

private:
    std::string FLOAT_DATATYPE = "f";
    std::string UNKNOWN_DATATYPE = "-1";

    Variables variables;
    std::vector<unsigned int> function_refs;    // Ids of function variables. Initially they contain references to functions in the 'function_bytecode'. They need to be udjasted after merging 'function_bytecode to 'bytecode'

    const std::string& upcast_datatype(const std::string& datatype);
    const std::string& maxDatatype(const std::string& datatype_1, const std::string& datatype_2);
    bool isDatatypeNumeric(const std::string& datatype);
    bool isReserved(std::string const& s);
    bool check_symbol(std::string const& s, const CodeLocation& pos, char symbol);
    bool check_string(std::string const& s, CodeLocation& pos, std::string const& str);
    void whitespace(std::string const& s, CodeLocation& pos);
    bool isTerminal(std::string const& s, const CodeLocation& pos);
    bool process_array_datatype(std::string& datatype, const std::string& index_datatype);
//    ETokenizeStatus tokenize(std::string const& s);
    bool identifier_first_char(std::string const& s, const CodeLocation& pos);
    bool identifier_char(std::string const& s, const CodeLocation& pos);
    bool string_char(std::string const& s, CodeLocation& pos);
    bool digit(std::string const& s, CodeLocation& pos);
    RetVal unsigned_float(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, std::string& value);
    RetVal unsigned_integer(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, std::string& value);
    RetVal string_expression(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, std::string& value);
    RetVal boolean_value(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, std::string& value);
    RetVal literals(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, EAssignmentPosition assign_pos, Bytecode& bytecode, std::string& datatype);
    RetVal primary_expression(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, EAssignmentPosition assign_pos, Bytecode& bytecode, std::string& datatype, const Scope& scope);
    RetVal identifier(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, Variable& variable, const Scope& scope, Variables::EScopeRange scopeRange);
    RetVal argumentlist_expression(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, EAssignmentPosition assign_pos, Bytecode& bytecode, std::string& datatype, int& param_count, const Scope& scope);
    RetVal postfix_expression(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, EAssignmentPosition assign_pos, Bytecode& bytecode, std::string& datatype, const Scope& scope);
    RetVal unary_operator(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, std::string& datatype);
    RetVal unary_expression(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, EAssignmentPosition assign_pos, Bytecode& bytecode, std::string& datatype, const Scope& scope);
    RetVal mult_operator(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, EArithOperators& arith_operator);
    RetVal mult_expression(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, EAssignmentPosition assign_pos, Bytecode& bytecode, std::string& datatype, const Scope& scope);
    RetVal additive_operator(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, EArithOperators& arith_operator);
    RetVal additive_expression(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, EAssignmentPosition assign_pos, Bytecode& bytecode, std::string& datatype, const Scope& scope);
    RetVal relation_operator(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, Bytecode& bytecode);
    RetVal relational_expression(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, EAssignmentPosition assign_pos, Bytecode& bytecode, std::string& datatype, const Scope& scope);
    RetVal logical_and_expression(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, EAssignmentPosition assign_pos, Bytecode& bytecode, std::string& datatype, const Scope& scope);
    RetVal logical_or_expression(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, EAssignmentPosition assign_pos, Bytecode& bytecode, std::string& datatype, const Scope& scope);
    RetVal conditional_expression(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, EAssignmentPosition assign_pos, Bytecode& bytecode, std::string& datatype, const Scope& scope);
    RetVal assignment_operator(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, Bytecode& bytecode, EExtendedAssignOperation& extendedOperation);
    RetVal assignment_expression(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, Bytecode& bytecode, const Scope& scope);
    RetVal index_type_declaration(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, std::string& datatype);
    RetVal type_declaration(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, std::string& datatype);
    RetVal variable_declaration(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, const Scope& scope, EDataMode data_mode, std::string& variable_datatype, unsigned int& variable_idx, std::vector<unsigned int>& function_variables);
    RetVal function_parameters_declaration(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, std::vector<unsigned int>& parameter_idxs, const Scope& scope, std::string& parameter_types);
    RetVal function_definition(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, Bytecode& bytecode, Bytecode& function_bytecode, const Scope& parent_scope);
    RetVal block_statement(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, Bytecode& bytecode, Bytecode& function_bytecode, const Scope& scope, EDataMode data_mode, std::vector<unsigned int>& function_variables);
    RetVal translation_unit(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, Bytecode& bytecode, Bytecode& function_bytecode, const Scope& scope, EDataMode data_mode, std::vector<unsigned int>& function_variables);
    void initVars(Bytecode& bytecode, const Scope& scope);

public:
    static bool isDatatypeConsistent(const std::string& datatype_1, const std::string& datatype_2);
    static bool isDatatypeConsistentAssignment(const std::string& datatype_1, const std::string& datatype_2);
    static bool isDatatypeConsistentFunctionArguments(const std::string& datatype_1, const std::string& datatype_2);
    EParseStatus parse(std::string const& s, ParseTrace& parse_trace, Bytecode& bytecode);
    void clear();
};

#endif // PARSER_H
