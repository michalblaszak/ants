#include "parser.h"
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>

const char* parseStatusDescription[] = {
    "PARSE_OK",
    "PARSE_ERROR",
    "PARSE_ERROR_COMPILATION_UNIT",
    "PARSE_ERROR_ASSIGNMENT_EXPRESSION",
    "PARSE_ERROR_ASSIGNMENT_OPERATOR",
    "PARSE_ERROR_CONDITIONAL_EXPRESSION",
    "PARSE_ERROR_LOGICAL_OR_EXPRESSION",
    "PARSE_ERROR_COND_EXP_MISSING_COLON",
    "PARSE_ERROR_LOGICAL_AND_EXPRESSION",
    "PARSE_ERROR_EQUALITY_OPERATOR",        // TODO: remove it
    "PARSE_ERROR_EQUALITY_EXPRESSION",      // TODO: remove it
    "PARSE_ERROR_RELATIONAL_EXPRESSION",
    "PARSE_ERROR_RELATION_OPERATOR",
    "PARSE_ERROR_ADDITIVE_EXPRESSION",
    "PARSE_ERROR_ADDITIVE_OPERATOR",
    "PARSE_ERROR_MULT_EXPRESSION",
    "PARSE_ERROR_MULT_OPERATOR",
    "PARSE_ERROR_UNARY_EXPRESSION",
    "PARSE_ERROR_UNARY_OPERATOR",
    "PARSE_ERROR_POSTFIX_EXPRESSION",
    "PARSE_ERROR_ARRAY_INDEX",
    "PARSE_ERROR_ARGUMENT_LIST",
    "PARSE_ERROR_ARGUMENTLIST_EXPRESSION",
    "PARSE_ERROR_IDENTIFIER",
    "PARSE_ERROR_PRIMARY_EXPRESSION",
    "PARSE_ERROR_STRING_EXPRESSION",
    "PARSE_ERROR_UNSIGNED_INTEGER",
    "PARSE_ERROR_UNSIGNED_FLOAT",
    "PARSE_ERROR_VARIABLE_DECLARATION",
    "PARSE_ERROR_DUPLICATE_VARIABLE_DECLARATION",
    "PARSE_ERROR_TYPE_DECLARATION",
    "PARSE_ERROR_INDEX_TYPE_DECLARATION",
    "PARSE_ERROR_IDENTIFIER_RESERVED_WORD",
    "PARSE_ERROR_ASSIGNMENT_DATATYPE",
    "PARSE_ERROR_UNKNOWN_IDENTIFIER",
    "PARSE_ERROR_ARRAY_DATATYPE",
    "PARSE_ERROR_DATATYPE_MISMATCH",
    "PARSE_ERROR_CONDITIONAL_EXPRESSION_STATEMENTS_DATATYPES",
    "PARSE_ERROR_CONDITIONAL_EXPRESSION_CONDITION_DATATYPE",
    "PARSE_ERROR_EXPECTED_BOOLEAN_DATATYPE",
    "PARSE_ERROR_CONDITIONAL_EXPRESSION_FALSE",
    "PARSE_ERROR_CONDITIONAL_EXPRESSION_TRUE",
    "PARSE_ERROR_BOOLEAN_VALUE",
    "PARSE_ERROR_LITERALS",
    "PARSE_ERROR_UNRECOGNIZED_IDENTIFIER_TYPE",
    "PARSE_ERROR_MISSING_FUNCTION_ARGUMENT_LIST",
    "PARSE_ERROR_INCOMPATIBLE_FUNCTION_ARGUMENTS",
    "PARSE_ERROR_BLOCK_STATEMENT",
    "PARSE_ERROR_ARGUMENT_LIST_DECLARATION",
    "PARSE_ERROR_FUNCTION_TYPE_DECLARATION",
    "PARSE_ERROR_FUNCTION_NAME_IN_USE",
    "PARSE_ERROR_FUNCTION_DEFINITION", 
    "PARSE_ERROR_FUNCTION_PARAMETERS_DECLARATION",
    "PARSE_ERROR_TYPE_DECLARATION_MISSING_OF",
    "PARSE_ERROR_TYPE_DECLARATION_MISSING_CLOSING_BRACKET",
    "PARSE_ERROR_TYPE_DECLARATION_MISSING_OPENING_BRACKET",
    "PARSE_ERROR_PRIMARY_EXPRESSION_MISSING_CLOSING_BRACKET"
};

const char terminal_symbols[] = ",+-*/()[]{}.<>!=?: \t\n";
const int terminal_symbols_len = 21;

const char* reservedWords[] = {
    "and", "or", "not", "int", "float", "string", "boolean", "array", "of", "function"
};

Parser::Parser()
{

}

const std::string& Parser::upcast_datatype(const std::string& datatype)
{
    if (datatype == "i") {
        return FLOAT_DATATYPE;
    } else {
        return datatype;
    }
}

bool Parser::isDatatypeNumeric(const std::string& datatype)
{
    return (datatype == "i" || datatype == "f" || datatype == "n");
}

bool Parser::isDatatypeConsistent(const std::string& datatype_1, const std::string& datatype_2)
{
    if (datatype_1 == datatype_2) 
        return true;
    else if ( (datatype_1 == "i" || datatype_1 == "f" || datatype_1 == "n")
         && (datatype_2 == "i" || datatype_2 == "f" || datatype_2 == "n")
    )
        return true;
    else 
        return false;
}

bool Parser::isDatatypeConsistentAssignment(const std::string& datatype_1, const std::string& datatype_2)
{
    if (datatype_1 == datatype_2) 
        return true;
    else if ( (datatype_1 == "f" || datatype_1 == "n" )
         && (datatype_2 == "i" || datatype_2 == "f" || datatype_2 == "n")
    )
        return true;
    else 
        return false;
}

bool Parser::isDatatypeConsistentFunctionArguments(const std::string& fun_def_param_types, const std::string& param_datatypes) {
    std::vector<std::string> l_fun_def_param_types;
    std::vector<std::string> l_param_datatypes;

    boost::split(l_fun_def_param_types, fun_def_param_types, boost::is_any_of(","));
    boost::split(l_param_datatypes, param_datatypes, boost::is_any_of(","));

    if (l_fun_def_param_types.size() != l_param_datatypes.size()) {
        return false;
    }

    for(int i = 0; i < l_fun_def_param_types.size(); i++) {
        if (!isDatatypeConsistentAssignment(l_fun_def_param_types[i], l_param_datatypes[i])) {
            return false;
        }
    }

    return true;
}

const std::string& Parser::maxDatatype(const std::string& datatype_1, const std::string& datatype_2)
{
    if (datatype_1 == datatype_2) return datatype_1;
    if (datatype_1 == "-1") return datatype_2;
    if (datatype_2 == "-1") return datatype_1;
    if (datatype_1 == "i" && datatype_2 == "f") return datatype_2;
    if (datatype_1 == "i" && datatype_2 == "n") return datatype_1;
    if (datatype_1 == "f" && datatype_2 == "i") return datatype_1;
    if (datatype_1 == "f" && datatype_2 == "n") return datatype_1;
    if (datatype_1 == "n" && datatype_2 == "i") return datatype_2;
    if (datatype_1 == "n" && datatype_2 == "f") return datatype_2;

    return UNKNOWN_DATATYPE;
}


bool Parser::isReserved(std::string const& s)
{
    for(int i=0; i<sizeof(reservedWords)/sizeof(char*); i++) {
        if (boost::iequals(s, reservedWords[i]))
            return true;
    }

    // s not found in the list of reserved words
    return false;
}

bool Parser::check_symbol(std::string const& s, const CodeLocation& pos, char symbol)
{
    return (s.length() > pos.pos) && (s[pos.pos] == symbol);
}

bool Parser::check_string(std::string const& s, CodeLocation& pos, std::string const& str)
{
    int str_len = str.length();

    if (s.length() < pos.pos+str_len)
        return false;

    // TODO: handle UTF-8 comparision
    for (int i=0; i<str_len; i++) {
        
        if (std::toupper(s[pos.pos+i]) != std::toupper(str[i]))
            return false;
    }

    pos.pos += str_len;
    pos.col += str_len;

    return true;
}

bool Parser::isTerminal(std::string const& s, const CodeLocation& pos)
{
    if (s.length() <= pos.pos) return true; // EOF is also terminal 

    for(int i = 0; i<terminal_symbols_len; i++) {
        if (s[pos.pos] == terminal_symbols[i])
            return true;
    }

    return false;
}

// Consume whitespaces
void Parser::whitespace(std::string const& s, CodeLocation& pos)
{
    int s_len = s.length();

    while (s_len > pos.pos && (s[pos.pos] == ' ' || s[pos.pos] == '\t' || s[pos.pos] == '\n')) {
        if (s[pos.pos] == '\n') {
            pos.col = 0;
            pos.row++;
        }
        else {
            pos.col++;
        }

        pos.pos++;
    }
}

bool Parser::identifier_first_char(std::string const& s, const CodeLocation& pos)
{
    return (s.length() > pos.pos) 
    && (   (s[pos.pos] >= 'A' && s[pos.pos] <='Z') 
        || (s[pos.pos] >= 'a' && s[pos.pos] <='z') 
        || s[pos.pos] == '_'
       );
}

bool Parser::identifier_char(std::string const& s, const CodeLocation& pos)
{
    return (s.length() > pos.pos) 
    && (   (s[pos.pos] >= 'A' && s[pos.pos] <='Z') 
        || (s[pos.pos] >= 'a' && s[pos.pos] <='z') 
        || (s[pos.pos] >= '0' && s[pos.pos] <='9') 
        || s[pos.pos] == '_'
       );
}

bool Parser::digit(std::string const& s, CodeLocation& pos)
{
    return (s.length() > pos.pos) 
    &&     (s[pos.pos] >= '0' && s[pos.pos] <='9');
}

RetVal Parser::identifier(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, Variable& variable, const Scope& scope, Variables::EScopeRange scopeRange)
{
    CodeLocation initial_pos = pos;
    CodeLocation nonspace_initial_pos;
    ParseTrace child_parse_trace;
    EParseStatus ret = EParseStatus::PARSE_ERROR_IDENTIFIER;

    whitespace(s, pos);

    nonspace_initial_pos = pos;

    if (identifier_first_char(s, pos)) {
        pos.pos++;
        pos.col++;

        while(identifier_char(s, pos)) {
            pos.pos++;
            pos.col++;
        }

        if (isTerminal(s, pos)) {
            std::string id_str = s.substr(nonspace_initial_pos.pos, pos.pos-nonspace_initial_pos.pos);

            if (isReserved(id_str)) {
                ret = EParseStatus::PARSE_ERROR_IDENTIFIER_RESERVED_WORD;
            } else {
                variables.getVariable(scope, scopeRange, id_str, variable);
                return RetVal::OK;
            }
        }
    }

    // We have an error
    child_parse_trace.pos = initial_pos;
    child_parse_trace.status = ret;
    parse_trace.suberrors.push_back(child_parse_trace);

    pos = initial_pos;
    return RetVal::FAIL_CONTINUE;
}

bool Parser::string_char(std::string const& s, CodeLocation& pos)
{
    if (s.length() > pos.pos && s[pos.pos] == '"') { // The end of the string
        return false;
    }

    if ((s.length() > pos.pos + 1)
        && s[pos.pos] == '\\' && (s[pos.pos + 1] == '"' || s[pos.pos + 1] == '\\')
        ) {
        pos.pos += 2;
        pos.col += 2;

        return true;
    }

    if (s.length() > pos.pos) { // Take everything between " and "
        pos.pos++;
        pos.col++;

        return true;
    }

    return false;
}

RetVal Parser::string_expression(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, std::string& value)
{
    CodeLocation initial_pos = pos;
    ParseTrace child_parse_trace;
    RetVal ret = RetVal::FAIL_CONTINUE;

    whitespace(s, pos);

    if (check_symbol(s, pos, '"')) {
        ret = RetVal::FAIL_STOP;

        pos.pos++;
        pos.col++;

        while(string_char(s, pos)); // 'string_char' advances 'pos'

        if (check_symbol(s, pos, '"')) {
            pos.pos++;
            pos.col++;

            if (isTerminal(s, pos)) {
                value = s.substr(initial_pos.pos+1, pos.pos-initial_pos.pos-2); // Quotation marks included so we need to remove them
                return RetVal::OK;
            }
        }
    }

    // We have an error
    child_parse_trace.pos = initial_pos;
    child_parse_trace.status = EParseStatus::PARSE_ERROR_STRING_EXPRESSION;
    parse_trace.suberrors.push_back(child_parse_trace);

    pos = initial_pos;
    return ret;
}

RetVal Parser::unsigned_integer(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, std::string& value)
{
    CodeLocation initial_pos = pos;
    ParseTrace child_parse_trace;

    whitespace(s, pos);

    if (digit(s, pos)) {
        pos.pos++;
        pos.col++;

        while (digit(s, pos)) {
            pos.pos++;
            pos.col++;
        }

        if (isTerminal(s, pos)) {
            value = s.substr(initial_pos.pos, pos.pos-initial_pos.pos);
            return RetVal::OK;
        }
    }

    // We have an error
    child_parse_trace.pos = initial_pos;
    child_parse_trace.status = EParseStatus::PARSE_ERROR_UNSIGNED_INTEGER;
    parse_trace.suberrors.push_back(child_parse_trace);

    pos = initial_pos;
    return RetVal::FAIL_CONTINUE;
}

RetVal Parser::unsigned_float(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, std::string& value)
{
    CodeLocation initial_pos = pos;
    ParseTrace child_parse_trace;
    RetVal ret = RetVal::FAIL_CONTINUE;

    whitespace(s, pos);

    if ((ret = unsigned_integer(s, pos, child_parse_trace, value)) == RetVal::OK) {
        ret = RetVal::FAIL_CONTINUE;

        if (check_symbol(s, pos, '.')) {
            ret = RetVal::FAIL_STOP;

            pos.pos++;
            pos.col++;

            if (unsigned_integer(s, pos, child_parse_trace, value) == RetVal::OK) {
                if (isTerminal(s, pos)) {
                    value = s.substr(initial_pos.pos, pos.pos-initial_pos.pos);
                    return RetVal::OK;
                }
            }
        }
    }

    // We have an error
    child_parse_trace.pos = initial_pos;
    child_parse_trace.status = EParseStatus::PARSE_ERROR_UNSIGNED_FLOAT;
    parse_trace.suberrors.push_back(child_parse_trace);

    pos = initial_pos;
    return ret;
}

RetVal Parser::boolean_value(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, std::string& value)
{
    CodeLocation initial_pos = pos;
    ParseTrace child_parse_trace;

    whitespace(s, pos);

    if (check_string(s, pos, "true")) {
        if (isTerminal(s, pos)) {
            value = "true";
            return RetVal::OK;
        }
    } else if (check_string(s, pos, "false")) {
        if (isTerminal(s, pos)) {
            value = "false";
            return RetVal::OK;
        }
    }

    // We have an error
    child_parse_trace.pos = initial_pos;
    child_parse_trace.status = EParseStatus::PARSE_ERROR_BOOLEAN_VALUE;
    parse_trace.suberrors.push_back(child_parse_trace);

    pos = initial_pos;
    return RetVal::FAIL_CONTINUE;
}

RetVal Parser::literals(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, EAssignmentPosition assign_pos, Bytecode& bytecode, std::string& datatype)
{
    CodeLocation initial_pos = pos;
    ParseTrace child_parse_trace;
    std::string bytecode_primary;
    Variable variable;
    RetVal ret;

    whitespace(s, pos);

    if (assign_pos == LEFT) {
        ret = RetVal::FAIL_STOP;
    }
    else { // RIGHT
        if ((ret = string_expression(s, pos, child_parse_trace, bytecode_primary)) == RetVal::OK) {
            datatype = "s";
            bytecode.PUTSTRING(bytecode_primary.data());
            return RetVal::OK;
        }

        if (ret == RetVal::FAIL_CONTINUE
            && (ret = unsigned_float(s, pos, child_parse_trace, bytecode_primary)) == RetVal::OK)
        {
            datatype = "f";
            bytecode.PUTFLOAT(std::stold(bytecode_primary));
            return RetVal::OK;
        }

        if (ret == RetVal::FAIL_CONTINUE
            && (ret = unsigned_integer(s, pos, child_parse_trace, bytecode_primary)) == RetVal::OK)
        {
            datatype = "i";
            bytecode.PUTINT(std::stoll(bytecode_primary));
            return RetVal::OK;
        }

        if (ret == RetVal::FAIL_CONTINUE
            && (ret = boolean_value(s, pos, child_parse_trace, bytecode_primary)) == RetVal::OK)
        {
            datatype = "b";
            bytecode.PUTBOOLEAN(boost::iequals(bytecode_primary, "true"));
            return RetVal::OK;
        }
    }
   // We have an error
    child_parse_trace.pos = initial_pos;
    child_parse_trace.status = EParseStatus::PARSE_ERROR_LITERALS;
    parse_trace.suberrors.push_back(child_parse_trace);

    pos = initial_pos;

    return ret;
}

RetVal Parser::primary_expression(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, EAssignmentPosition assign_pos, Bytecode& bytecode, std::string& datatype, const Scope& scope)
{
    CodeLocation initial_pos = pos;
    ParseTrace child_parse_trace;
    std::string bytecode_primary;
    Variable variable;
    RetVal ret = RetVal::FAIL_CONTINUE;

    whitespace(s, pos);

    if (identifier(s, pos, child_parse_trace, variable, scope, Variables::EScopeRange::INHERITANCE) == RetVal::OK) {
        if (variable.getIdx() == -1) {
            child_parse_trace.pos = initial_pos;
            child_parse_trace.status = EParseStatus::PARSE_ERROR_UNKNOWN_IDENTIFIER;
            parse_trace.suberrors.push_back(child_parse_trace);

            pos = initial_pos;

            return RetVal::FAIL_STOP;
        } else { // It's a known id. Let's check if it is a variable or a function
            if (variable.getEntityType() == Variable::EVariableTypes::BUILTIN_VARIABLE 
            || variable.getEntityType() == Variable::EVariableTypes::VARIABLE
            || variable.getEntityType() == Variable::EVariableTypes::DYNAMIC_VARIABLE) {
                datatype = variable.getType();

                if (variable.getEntityType() == Variable::EVariableTypes::BUILTIN_VARIABLE 
                || variable.getEntityType() == Variable::EVariableTypes::VARIABLE) {
                    bytecode.PUTADDR(variable.getIdx());
                } else { // variable.getEntityType() == Variable::EVariableTypes::DYNAMIC_VARIABLE)
                    bytecode.PUTDADDR(variable.getIdx());
                }

                do {
                    whitespace(s, pos);

                    if (check_symbol(s, pos, '[') ) {
                        pos.pos++;
                        pos.col++;
                    
                        whitespace(s, pos);

                        std::string datatype_args;
                        int arg_count = 0;

                        ParseTrace array_index_trace(pos, EParseStatus::PARSE_ERROR_ARRAY_INDEX);

                        if (argumentlist_expression(s, pos, array_index_trace, RIGHT, bytecode, datatype_args, arg_count, scope) == RetVal::OK) {
                            whitespace(s, pos);
            
                            if (check_symbol(s, pos, ']')) {
                                if (process_array_datatype(datatype, datatype_args) ) {
                                    pos.pos++;
                                    pos.col++;

                                    bytecode.PUTINDADDR(arg_count); // Takes array address from the stack and index value from the stack and puts the address of the resulting variable to the stack
                                    continue; // return PARSE_OK;
                                } else {
                                    // We got an error
                                    ParseTrace XXarray_index_trace(pos, EParseStatus::PARSE_ERROR_ARRAY_DATATYPE);
                                    array_index_trace.suberrors.push_back(XXarray_index_trace);
                                    child_parse_trace.suberrors.push_back(array_index_trace);

                                    child_parse_trace.pos = initial_pos;
                                    child_parse_trace.status = EParseStatus::PARSE_ERROR_PRIMARY_EXPRESSION;
                                    parse_trace.suberrors.push_back(child_parse_trace);

                                    pos = initial_pos;
                                    return RetVal::FAIL_STOP;
                                }
                            }
                            else {
                                child_parse_trace.pos = initial_pos;
                                child_parse_trace.status = EParseStatus::PARSE_ERROR_PRIMARY_EXPRESSION_MISSING_CLOSING_BRACKET;
                                child_parse_trace.suberrors.push_back(array_index_trace);
                                parse_trace.suberrors.push_back(child_parse_trace);

                                pos = initial_pos;
                                return RetVal::FAIL_STOP;
                            }
                        }
                        else {
                            
                            child_parse_trace.pos = initial_pos;
                            child_parse_trace.status = EParseStatus::PARSE_ERROR_PRIMARY_EXPRESSION;
                            child_parse_trace.suberrors.push_back(array_index_trace);
                            parse_trace.suberrors.push_back(child_parse_trace);

                            pos = initial_pos;
                            return RetVal::FAIL_STOP;
                        }

                        // We shouldn't be here
                        break;
                        
                    } else {
                        return RetVal::OK;
                    }
                } while(true);
            } else if (variable.getEntityType() == Variable::EVariableTypes::BUILTIN_FUNCTION || variable.getEntityType() == Variable::EVariableTypes::FUNCTION) {
                whitespace(s, pos);

                if (assign_pos == RIGHT && check_symbol(s, pos, '(') ) {
                    pos.pos++;
                    pos.col++;

                    whitespace(s, pos);

                    std::string datatype_args;
                    int arg_count;

                    if (argumentlist_expression(s, pos, child_parse_trace, RIGHT, bytecode, datatype_args, arg_count, scope) == RetVal::OK) {
                        whitespace(s, pos);

                        if (check_symbol(s, pos, ')')) {
                            pos.pos++;
                            pos.col++;

                            if (isDatatypeConsistentFunctionArguments(variable.getType_fun_params(), datatype_args)) {
                                datatype = variable.getType();
                                bytecode.CALL(variable.getIdx());
                                return RetVal::OK;
                            } else {
                                ParseTrace argument_list_trace(pos, EParseStatus::PARSE_ERROR_INCOMPATIBLE_FUNCTION_ARGUMENTS);
                                child_parse_trace.suberrors.push_back(argument_list_trace);

                                child_parse_trace.pos = initial_pos;
                                child_parse_trace.status = EParseStatus::PARSE_ERROR_PRIMARY_EXPRESSION;
                                parse_trace.suberrors.push_back(child_parse_trace);

                                pos = initial_pos;
                                return RetVal::FAIL_STOP;
                            }
                        }
                    }

                    ParseTrace argument_list_trace(pos, EParseStatus::PARSE_ERROR_ARGUMENT_LIST);
                    child_parse_trace.suberrors.push_back(argument_list_trace);

                    child_parse_trace.pos = initial_pos;
                    child_parse_trace.status = EParseStatus::PARSE_ERROR_PRIMARY_EXPRESSION;
                    parse_trace.suberrors.push_back(child_parse_trace);

                    pos = initial_pos;
                    return RetVal::FAIL_STOP;
                } else if (assign_pos == LEFT) {
                    datatype = variable.getType();
                    bytecode.PUTDADDR(variable.getIdx());
                    return RetVal::OK;
                } else {
                    child_parse_trace.pos = initial_pos;
                    child_parse_trace.status = EParseStatus::PARSE_ERROR_MISSING_FUNCTION_ARGUMENT_LIST;
                    parse_trace.suberrors.push_back(child_parse_trace);

                    pos = initial_pos;
                    return RetVal::FAIL_STOP;
                }
            } else {
                child_parse_trace.pos = initial_pos;
                child_parse_trace.status = EParseStatus::PARSE_ERROR_UNRECOGNIZED_IDENTIFIER_TYPE;
                parse_trace.suberrors.push_back(child_parse_trace);

                pos = initial_pos;
                return RetVal::FAIL_STOP;
            }
        }
    }

    // We have an error
    child_parse_trace.pos = initial_pos;
    child_parse_trace.status = EParseStatus::PARSE_ERROR_PRIMARY_EXPRESSION;
    parse_trace.suberrors.push_back(child_parse_trace);

    pos = initial_pos;
    return ret;
}

RetVal Parser::argumentlist_expression(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, EAssignmentPosition assign_pos, Bytecode& bytecode, std::string& datatype, int& param_count, const Scope& scope)
{
    CodeLocation initial_pos = pos;
    ParseTrace child_parse_trace;
    std::string datatype_args;
    std::string ret_datatype;
    int count = 0;

    do {
        whitespace(s, pos);

        if (conditional_expression(s, pos, child_parse_trace, assign_pos, bytecode, datatype_args, scope) != RetVal::OK) {
            child_parse_trace.pos = initial_pos;
            child_parse_trace.status = EParseStatus::PARSE_ERROR_ARGUMENTLIST_EXPRESSION;
            parse_trace.suberrors.push_back(child_parse_trace);

            pos = initial_pos;
            return RetVal::FAIL_STOP;
        }

        whitespace(s, pos);

        ret_datatype += datatype_args;
        count++;

        if (check_symbol(s, pos, ',')) {
            ret_datatype += ",";
            pos.pos++;
            pos.col++;
        } else {
            break;
        }
    } while(true);

    datatype = ret_datatype;
    param_count = count;
    return RetVal::OK;
}

bool Parser::process_array_datatype(std::string& datatype, const std::string& index_datatype)
{
    if (boost::algorithm::istarts_with(datatype, std::string("a [") + index_datatype + "]")) {
        datatype.erase(0, index_datatype.length()+5); // +4 to handle brackets and spaces
        return true;
    }

    return false;
}

RetVal Parser::postfix_expression(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, EAssignmentPosition assign_pos, Bytecode& bytecode, std::string& datatype, const Scope& scope)
{
    CodeLocation initial_pos = pos;
    ParseTrace child_parse_trace;
    RetVal ret;
    EAction ret_action = EAction::CONTINUE;

    whitespace(s, pos);

    if (assign_pos == RIGHT) {
        ret = literals(s, pos, child_parse_trace, assign_pos, bytecode, datatype);

        switch (ret) {
        case RetVal::OK:
            return RetVal::OK;
        case RetVal::FAIL_STOP:
            child_parse_trace.pos = initial_pos;
            child_parse_trace.status = EParseStatus::PARSE_ERROR_POSTFIX_EXPRESSION;
            parse_trace.suberrors.push_back(child_parse_trace);

            pos = initial_pos;
            return RetVal::FAIL_STOP;
        case RetVal::FAIL_CONTINUE:
            if (check_symbol(s, pos, '('))
            {
                pos.pos++;
                pos.col++;

                if ((ret = conditional_expression(s, pos, child_parse_trace, assign_pos, bytecode, datatype, scope)) == RetVal::OK) {
                    whitespace(s, pos);

                    if (check_symbol(s, pos, ')')) {
                        pos.pos++;
                        pos.col++;

                        return RetVal::OK;
                    }
                }

                // We have an error
                child_parse_trace.pos = initial_pos;
                child_parse_trace.status = EParseStatus::PARSE_ERROR_POSTFIX_EXPRESSION;
                parse_trace.suberrors.push_back(child_parse_trace);

                pos = initial_pos;
                return RetVal::FAIL_STOP;
            }
            break;
        }
    }

    // Scope modificator
    // std::string scope_modificator;
    Scope primary_expr_scope(scope);

    while (check_symbol(s, pos, '.')) {
        pos.pos++;
        pos.col++;

        ret_action = EAction::STOP;

        // scope_modificator += '.';
        primary_expr_scope.reduce(); // takes the last element off
    }

    bool is_first = true;

    do {
        if ((ret = primary_expression(s, pos, child_parse_trace, assign_pos, bytecode, datatype, is_first ? primary_expr_scope : scope)) == RetVal::OK) {
            is_first = false;

            if (check_symbol(s, pos, '.') ) {
                ret_action = EAction::STOP;

                continue;
            } else {
                return RetVal::OK;
            }
        } else {
            break;
        }
    } while(true);

    // We have an error
    child_parse_trace.pos = initial_pos;
    child_parse_trace.status = EParseStatus::PARSE_ERROR_POSTFIX_EXPRESSION;
    parse_trace.suberrors.push_back(child_parse_trace);

    pos = initial_pos;
    return ret_action == EAction::STOP ? RetVal::FAIL_STOP : ret;
}

RetVal Parser::unary_operator(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, std::string& datatype)
{
    CodeLocation initial_pos = pos;
    ParseTrace child_parse_trace;

    whitespace(s, pos);

    if ( check_symbol(s, pos, '-') ) {
        datatype = "n";
        pos.pos++;
        pos.col++;

        return RetVal::OK;
    } else if ( check_string(s, pos, "not") ) {
        datatype = "b";
        return RetVal::OK;
    }
    // This is not an assignment operator

    child_parse_trace.pos = initial_pos;
    child_parse_trace.status = EParseStatus::PARSE_ERROR_UNARY_OPERATOR;
    parse_trace.suberrors.push_back(child_parse_trace);

    pos = initial_pos;
    return RetVal::FAIL_CONTINUE;
}

RetVal Parser::unary_expression(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, EAssignmentPosition assign_pos, Bytecode& bytecode, std::string& datatype, const Scope& scope)
{
    CodeLocation initial_pos = pos;
    ParseTrace child_parse_trace;
    std::string operator_datatype;
    EParseStatus ret = EParseStatus::PARSE_ERROR_UNARY_EXPRESSION;
    RetVal child_ret;
    EAction ret_action = EAction::CONTINUE;

    whitespace(s, pos);
    if (assign_pos == LEFT) {
        if ((child_ret = postfix_expression(s, pos, child_parse_trace, assign_pos, bytecode, datatype, scope)) == RetVal::OK)
        {
            return RetVal::OK;
        }
        else {
            child_parse_trace.pos = initial_pos;
            child_parse_trace.status = EParseStatus::PARSE_ERROR_UNARY_EXPRESSION;;
            parse_trace.suberrors.push_back(child_parse_trace);

            pos = initial_pos;
            return child_ret;
        }
    }
    else { // RIGHT
        if ((child_ret = unary_operator(s, pos, child_parse_trace, operator_datatype)) == RetVal::OK) {
            ret_action = EAction::STOP;

            if ((child_ret = unary_expression(s, pos, child_parse_trace, assign_pos, bytecode, datatype, scope)) == RetVal::OK) {
                if (isDatatypeConsistent(operator_datatype, datatype)) {
                    bytecode.NEG();
                    return RetVal::OK;
                }
                else {
                    child_parse_trace.pos = initial_pos;
                    child_parse_trace.status = EParseStatus::PARSE_ERROR_DATATYPE_MISMATCH;
                    parse_trace.suberrors.push_back(child_parse_trace);

                    pos = initial_pos;
                    return RetVal::FAIL_STOP;
                }
            }
            else {
                child_parse_trace.pos = initial_pos;
                child_parse_trace.status = EParseStatus::PARSE_ERROR_UNARY_EXPRESSION;;
                parse_trace.suberrors.push_back(child_parse_trace);

                pos = initial_pos;
                return RetVal::FAIL_STOP;
            }
        }
        else {
            if (child_ret == RetVal::FAIL_CONTINUE
                && (child_ret = postfix_expression(s, pos, child_parse_trace, assign_pos, bytecode, datatype, scope)) == RetVal::OK)
            {
                return RetVal::OK;
            }
            else {
                child_parse_trace.pos = initial_pos;
                child_parse_trace.status = EParseStatus::PARSE_ERROR_UNARY_EXPRESSION;;
                parse_trace.suberrors.push_back(child_parse_trace);

                pos = initial_pos;
                return child_ret;
            }
        }
    }
    // Actually we shouldn't be here
    child_parse_trace.pos = initial_pos;
    child_parse_trace.status = EParseStatus::PARSE_ERROR_UNARY_EXPRESSION;;
    parse_trace.suberrors.push_back(child_parse_trace);

    pos = initial_pos;
    return RetVal::FAIL_CONTINUE;
}

RetVal Parser::mult_operator(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, EArithOperators& arith_operator)
{
    CodeLocation initial_pos = pos;
    ParseTrace child_parse_trace;

    whitespace(s, pos);

    if ( check_symbol(s, pos, '*') ) {
        pos.pos++;
        pos.col++;

        arith_operator = MUL;

        return RetVal::OK;
    }

    if ( check_symbol(s, pos, '/') ) {
        pos.pos++;
        pos.col++;

        arith_operator = DIV;

        return RetVal::OK;
    }

    // This is not an assignment operator

    child_parse_trace.pos = initial_pos;
    child_parse_trace.status = EParseStatus::PARSE_ERROR_MULT_OPERATOR;
    parse_trace.suberrors.push_back(child_parse_trace);

    pos = initial_pos;
    return RetVal::FAIL_CONTINUE;
}

RetVal Parser::mult_expression(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, EAssignmentPosition assign_pos, Bytecode& bytecode, std::string& datatype, const Scope& scope)
{
    CodeLocation initial_pos = pos;
    ParseTrace child_parse_trace;
    bool put_mul = false;
    std::string datatype_pre = "-1";
    std::string datatype_cur;
    EArithOperators arith_operator;
    EAction ret_action = EAction::CONTINUE;
    RetVal child_ret;

    do {
        whitespace(s, pos);

        if ((child_ret = unary_expression(s, pos, child_parse_trace, assign_pos, bytecode, datatype_cur, scope)) != RetVal::OK) {
            child_parse_trace.pos = initial_pos;
            child_parse_trace.status = EParseStatus::PARSE_ERROR_MULT_EXPRESSION;
            parse_trace.suberrors.push_back(child_parse_trace);

            pos = initial_pos;
            return ret_action == EAction::STOP ? RetVal::FAIL_STOP : child_ret;
        }

        ret_action = EAction::STOP;

        if (put_mul) {
            if (arith_operator == MUL) {
                bytecode.MUL();
            } else if (arith_operator == DIV) {
                bytecode.DIV();
            }

            if ( !(isDatatypeNumeric(datatype_pre) && isDatatypeNumeric(datatype_cur)) ) {
                child_parse_trace.pos = initial_pos;
                child_parse_trace.status = EParseStatus::PARSE_ERROR_DATATYPE_MISMATCH;
                parse_trace.suberrors.push_back(child_parse_trace);

                pos = initial_pos;
                return RetVal::FAIL_STOP;
            }
        }

        if((child_ret = mult_operator(s, pos, child_parse_trace, arith_operator)) != RetVal::OK) {
            break;
        } else {
            put_mul = true;
            datatype_pre = maxDatatype(datatype_pre, datatype_cur);
        }

    } while(true);

    datatype = maxDatatype(datatype_pre, datatype_cur);
    return RetVal::OK;
}

RetVal Parser::additive_operator(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, EArithOperators& arith_operator)
{
    CodeLocation initial_pos = pos;
    ParseTrace child_parse_trace;

    whitespace(s, pos);

    if ( check_symbol(s, pos, '+') ) {
        pos.pos++;
        pos.col++;

        arith_operator = ADD;

        return RetVal::OK;
    }

    if ( check_symbol(s, pos, '-') ) {
        pos.pos++;
        pos.col++;

        arith_operator = SUB;

        return RetVal::OK;
    }

    // This is not an assignment operator

    child_parse_trace.pos = initial_pos;
    child_parse_trace.status = EParseStatus::PARSE_ERROR_ADDITIVE_OPERATOR;
    parse_trace.suberrors.push_back(child_parse_trace);

    pos = initial_pos;
    return RetVal::FAIL_CONTINUE;
}

RetVal Parser::additive_expression(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, EAssignmentPosition assign_pos, Bytecode& bytecode, std::string& datatype, const Scope& scope)
{
    CodeLocation initial_pos = pos;
    ParseTrace child_parse_trace;
    bool put_add = false;
    std::string datatype_pre = "-1";
    std::string datatype_cur;
    EArithOperators arith_operator;
    EAction ret_action = EAction::CONTINUE;
    RetVal child_ret;

    do {
        whitespace(s, pos);

        if ((child_ret = mult_expression(s, pos, child_parse_trace, assign_pos, bytecode, datatype_cur, scope)) != RetVal::OK) {
            child_parse_trace.pos = initial_pos;
            child_parse_trace.status = EParseStatus::PARSE_ERROR_ADDITIVE_EXPRESSION;
            parse_trace.suberrors.push_back(child_parse_trace);

            pos = initial_pos;
            return ret_action == EAction::STOP ? RetVal::FAIL_STOP : child_ret;
        }

        if (put_add) {
            if (arith_operator == ADD) {
                bytecode.ADD();
            } else {
                bytecode.SUB();
            }

            if ( !(
                (isDatatypeNumeric(datatype_pre) && isDatatypeNumeric(datatype_cur))
                || (datatype_pre == "s" && datatype_cur == "s")
                ) ) {
                child_parse_trace.pos = initial_pos;
                child_parse_trace.status = EParseStatus::PARSE_ERROR_DATATYPE_MISMATCH;
                parse_trace.suberrors.push_back(child_parse_trace);

                pos = initial_pos;
                return RetVal::FAIL_STOP;
            }
        }

        if((child_ret = additive_operator(s, pos, child_parse_trace, arith_operator)) != RetVal::OK) {
            break;
        } else {
            ret_action = EAction::STOP;
            put_add = true;
            datatype_pre = maxDatatype(datatype_pre, datatype_cur);
        }
    } while(true);

    datatype = maxDatatype(datatype_pre, datatype_cur);
    return RetVal::OK;
}

RetVal Parser::relation_operator(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, Bytecode& bytecode)
{
    CodeLocation initial_pos = pos;
    ParseTrace child_parse_trace;

    whitespace(s, pos);

    if ( check_symbol(s, pos, '=') && check_symbol(s, pos+1, '=')) {
        pos.pos += 2;
        pos.col += 2;
        bytecode.EQUAL();
        return RetVal::OK;
    }

    if (check_symbol(s, pos, '!') && check_symbol(s, pos+1, '=')) {
        pos.pos += 2;
        pos.col += 2;
        bytecode.NOTEQUAL();
        return RetVal::OK;
    }

    if (check_symbol(s, pos, '<') && check_symbol(s, pos+1, '=')) {
        pos.pos += 2;
        pos.col += 2;
        bytecode.LESSEQUAL();
        return RetVal::OK;
    }

    if (check_symbol(s, pos, '>') && check_symbol(s, pos+1, '=')) {
        pos.pos += 2;
        pos.col += 2;
        bytecode.GREATEREQUAL();
        return RetVal::OK;
    }

    if (check_symbol(s, pos, '<') ) {
        pos.pos++;
        pos.col++;
        bytecode.LESS();
        return RetVal::OK;
    }

    if (check_symbol(s, pos, '>') ) {
        pos.pos++;
        pos.col++;
        bytecode.GREATER();
        return RetVal::OK;
    }

    // This is not an assignment operator

    child_parse_trace.pos = initial_pos;
    child_parse_trace.status = EParseStatus::PARSE_ERROR_RELATION_OPERATOR;
    parse_trace.suberrors.push_back(child_parse_trace);

    pos = initial_pos;
    return RetVal::FAIL_CONTINUE;
}

RetVal Parser::relational_expression(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, EAssignmentPosition assign_pos, Bytecode& bytecode, std::string& datatype, const Scope& scope)
{
    CodeLocation initial_pos = pos;
    ParseTrace child_parse_trace;
    std::string datatype_left;
    std::string datatype_right;
    Bytecode bytecode_relation;
    RetVal child_ret;
    EAction ret_action = EAction::CONTINUE;

    whitespace(s, pos);

    if ((child_ret = additive_expression(s, pos, child_parse_trace, assign_pos, bytecode, datatype_left, scope)) != RetVal::OK) {
        child_parse_trace.pos = initial_pos;
        child_parse_trace.status = EParseStatus::PARSE_ERROR_RELATIONAL_EXPRESSION;
        parse_trace.suberrors.push_back(child_parse_trace);

        pos = initial_pos;
        return child_ret;
    }

    if(relation_operator(s, pos, child_parse_trace, bytecode_relation) == RetVal::OK) {
        if (additive_expression(s, pos, child_parse_trace, assign_pos, bytecode, datatype_right, scope) != RetVal::OK) {
            child_parse_trace.pos = initial_pos;
            child_parse_trace.status = EParseStatus::PARSE_ERROR_RELATIONAL_EXPRESSION;
            parse_trace.suberrors.push_back(child_parse_trace);

            pos = initial_pos;
            return RetVal::FAIL_STOP;
        }

        if (!isDatatypeConsistent(datatype_left, datatype_right)) {
            child_parse_trace.pos = initial_pos;
            child_parse_trace.status = EParseStatus::PARSE_ERROR_DATATYPE_MISMATCH;
            parse_trace.suberrors.push_back(child_parse_trace);

            pos = initial_pos;
            return RetVal::FAIL_STOP;
        }

        datatype = "b";
        bytecode += bytecode_relation;
    } else {
        datatype = datatype_left;
    }

    return RetVal::OK;
}

RetVal Parser::logical_and_expression(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, EAssignmentPosition assign_pos, Bytecode& bytecode, std::string& datatype, const Scope& scope)
{
    CodeLocation initial_pos = pos;
    ParseTrace child_parse_trace;
    bool put_mul = false;
    std::string datatype_pre = "-1";
    std::string datatype_cur;
    EAction ret_action = EAction::CONTINUE;
    RetVal child_ret;

    do {
        whitespace(s, pos);

        if ((child_ret = relational_expression(s, pos, child_parse_trace, assign_pos, bytecode, datatype_cur, scope)) != RetVal::OK) {
            child_parse_trace.pos = initial_pos;
            child_parse_trace.status = EParseStatus::PARSE_ERROR_LOGICAL_AND_EXPRESSION;
            parse_trace.suberrors.push_back(child_parse_trace);

            pos = initial_pos;
            return ret_action == EAction::STOP ? RetVal::FAIL_STOP : child_ret;

        }

        if (put_mul) {
            bytecode.MUL();

            if (datatype_pre != "b" || datatype_cur != "b") {
                child_parse_trace.pos = initial_pos;
                child_parse_trace.status = EParseStatus::PARSE_ERROR_EXPECTED_BOOLEAN_DATATYPE;
                parse_trace.suberrors.push_back(child_parse_trace);

                pos = initial_pos;
                return RetVal::FAIL_STOP;
            }

        }

        whitespace(s, pos);

        if (check_string(s, pos, "and")) {
            ret_action = EAction::STOP;
            put_mul = true;
            datatype_pre = datatype_cur;
        } else {
            break;
        }
    } while(true);

    datatype = datatype_cur;
    return RetVal::OK;
}

RetVal Parser::logical_or_expression(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, EAssignmentPosition assign_pos, Bytecode& bytecode, std::string& datatype, const Scope& scope)
{
    CodeLocation initial_pos = pos;
    ParseTrace child_parse_trace;
    bool put_add = false;
    std::string datatype_pre = "-1";
    std::string datatype_cur;
    EAction ret_action = EAction::CONTINUE;
    RetVal child_ret;

    do {
        whitespace(s, pos);

        if ((child_ret = logical_and_expression(s, pos, child_parse_trace, assign_pos, bytecode, datatype_cur, scope)) != RetVal::OK) {
            child_parse_trace.pos = initial_pos;
            child_parse_trace.status = EParseStatus::PARSE_ERROR_LOGICAL_OR_EXPRESSION;
            parse_trace.suberrors.push_back(child_parse_trace);

            pos = initial_pos;
            return ret_action == EAction::STOP ? RetVal::FAIL_STOP : child_ret;
        }

        if (put_add) {
            bytecode.ADD();

            if (datatype_pre != "b" || datatype_cur != "b") {
                child_parse_trace.pos = initial_pos;
                child_parse_trace.status = EParseStatus::PARSE_ERROR_EXPECTED_BOOLEAN_DATATYPE;
                parse_trace.suberrors.push_back(child_parse_trace);

                pos = initial_pos;
                return RetVal::FAIL_STOP;
            }
        }

        whitespace(s, pos);

        if (check_string(s, pos, "or") ) {
            ret_action = EAction::STOP;
            put_add = true;
            datatype_pre = datatype_cur;
        } else {
            break;
        }
    } while(true);

    datatype = datatype_cur;
    return RetVal::OK;
}

RetVal Parser::conditional_expression(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, EAssignmentPosition assign_pos, Bytecode& bytecode, std::string& datatype, const Scope& scope)
{
    CodeLocation initial_pos = pos;
    ParseTrace child_parse_trace;
    std::string datatype_condition;
    std::string datatype_true;
    std::string datatype_false;
    RetVal child_ret;

    whitespace(s, pos);

    if ((child_ret = logical_or_expression(s, pos, child_parse_trace, assign_pos, bytecode, datatype_condition, scope)) == RetVal::OK ) {
            whitespace(s, pos);

        if (check_symbol(s, pos, '?')) {
            if (datatype_condition == "b") {
                pos.pos++;
                pos.col++;

                unsigned int j_l2 = bytecode.JUMPIFFALSE(0);
                bytecode.addJump(j_l2+1);

                if (conditional_expression(s, pos, child_parse_trace, assign_pos, bytecode, datatype_true, scope) == RetVal::OK) {
                    whitespace(s, pos);
                    unsigned int j_l3 = bytecode.JUMP(0);
                    bytecode.addJump(j_l3+1);
                    if (check_symbol(s, pos, ':')) {
                        unsigned int l2 = bytecode.getCode().size();
                        bytecode.setAddress(j_l2+1, l2);

                        pos.pos++;
                        pos.col++;

                        if (conditional_expression(s, pos, child_parse_trace, assign_pos, bytecode, datatype_false, scope) == RetVal::OK) {
                            if ( isDatatypeConsistent(datatype_true, datatype_false) ) {
                                datatype = maxDatatype(datatype_true, datatype_false);
                                unsigned int l3 = bytecode.getCode().size();
                                bytecode.setAddress(j_l3+1, l3);
                                return RetVal::OK;
                            } else {
                                child_parse_trace.pos = initial_pos;
                                child_parse_trace.status = EParseStatus::PARSE_ERROR_CONDITIONAL_EXPRESSION_STATEMENTS_DATATYPES;
                                parse_trace.suberrors.push_back(child_parse_trace);

                                pos = initial_pos;
                                return RetVal::FAIL_STOP;
                            }
                        } else {
                            child_parse_trace.pos = initial_pos;
                            child_parse_trace.status = EParseStatus::PARSE_ERROR_CONDITIONAL_EXPRESSION_FALSE;
                            parse_trace.suberrors.push_back(child_parse_trace);

                            pos = initial_pos;
                            return RetVal::FAIL_STOP;
                        }
                    } else {
                        ParseTrace colon_parse_trace(pos, EParseStatus::PARSE_ERROR_COND_EXP_MISSING_COLON);
                        child_parse_trace.suberrors.push_back(colon_parse_trace);

                        child_parse_trace.pos = initial_pos;
                        child_parse_trace.status = EParseStatus::PARSE_ERROR_CONDITIONAL_EXPRESSION;
                        parse_trace.suberrors.push_back(child_parse_trace);

                        pos = initial_pos;
                        return RetVal::FAIL_STOP;
                    }
                } else {
                    child_parse_trace.pos = initial_pos;
                    child_parse_trace.status = EParseStatus::PARSE_ERROR_CONDITIONAL_EXPRESSION_TRUE;
                    parse_trace.suberrors.push_back(child_parse_trace);

                    pos = initial_pos;
                    return RetVal::FAIL_STOP;
                }
            } else {
                child_parse_trace.pos = initial_pos;
                child_parse_trace.status = EParseStatus::PARSE_ERROR_CONDITIONAL_EXPRESSION_CONDITION_DATATYPE;
                parse_trace.suberrors.push_back(child_parse_trace);

                pos = initial_pos;
                return RetVal::FAIL_STOP;
            }
        } else {
            datatype = datatype_condition;
            return RetVal::OK;
        }
    }
    else {
        child_parse_trace.pos = initial_pos;
        child_parse_trace.status = EParseStatus::PARSE_ERROR_CONDITIONAL_EXPRESSION;
        parse_trace.suberrors.push_back(child_parse_trace);

        pos = initial_pos;
        return child_ret;
    }


    // We shouldn't be here
    child_parse_trace.pos = initial_pos;
    child_parse_trace.status = EParseStatus::PARSE_ERROR_CONDITIONAL_EXPRESSION;
    parse_trace.suberrors.push_back(child_parse_trace);

    pos = initial_pos;
    return RetVal::FAIL_CONTINUE;
}

RetVal Parser::assignment_operator(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, Bytecode& bytecode, EExtendedAssignOperation& extendedOperation)
{
    CodeLocation initial_pos = pos;
    ParseTrace child_parse_trace;

    extendedOperation = EExtendedAssignOperation::NONE;

    whitespace(s, pos);

    if (check_symbol(s, pos, '=')) {
        pos.pos++;
        pos.col++;
        bytecode.MOVE();
        extendedOperation = EExtendedAssignOperation::NONE;
        return RetVal::OK;
    }

    if (check_symbol(s, pos, '+') && check_symbol(s, pos+1, '=')) {
        pos.pos += 2;
        pos.col += 2;
        bytecode.MOVEADD();
        extendedOperation = EExtendedAssignOperation::ADD;
        return RetVal::OK;
    }

    if (check_symbol(s, pos, '-') && check_symbol(s, pos+1, '=')) {
        pos.pos += 2;
        pos.col += 2;
        bytecode.MOVESUBTR();
        extendedOperation = EExtendedAssignOperation::SUB;
        return RetVal::OK;
    }

    if (check_symbol(s, pos, '*') && check_symbol(s, pos+1, '=')) {
        pos.pos += 2;
        pos.col += 2;
        bytecode.MOVEMUL();
        extendedOperation = EExtendedAssignOperation::MUL;
        return RetVal::OK;
    }

    if (check_symbol(s, pos, '/') && check_symbol(s, pos+1, '=')) {
        pos.pos += 2;
        pos.col += 2;
        bytecode.MOVEDIV();
        extendedOperation = EExtendedAssignOperation::DIV;
        return RetVal::OK;
    }

    // This is not an assignment operator

    child_parse_trace.pos = initial_pos;
    child_parse_trace.status = EParseStatus::PARSE_ERROR_ASSIGNMENT_OPERATOR;
    parse_trace.suberrors.push_back(child_parse_trace);

    pos = initial_pos;
    return RetVal::FAIL_CONTINUE;
}

RetVal Parser::assignment_expression(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, Bytecode& bytecode, const Scope& scope)
{
    CodeLocation initial_pos = pos;
    ParseTrace child_parse_trace;
    Bytecode operator_bytecode;
    std::string datatype_left;
    std::string datatype_right;
    EParseStatus ret = EParseStatus::PARSE_ERROR_ASSIGNMENT_EXPRESSION;
    EExtendedAssignOperation extendedOperation;
    RetVal child_ret;

    whitespace(s, pos);

    if ((child_ret = postfix_expression(s, pos, child_parse_trace, LEFT, bytecode, datatype_left, scope)) != RetVal::OK) {
        child_parse_trace.pos = initial_pos;
        child_parse_trace.status = EParseStatus::PARSE_ERROR_ASSIGNMENT_EXPRESSION;
        parse_trace.suberrors.push_back(child_parse_trace);

        pos = initial_pos;
        return child_ret;
    }

    if (assignment_operator(s, pos, child_parse_trace, operator_bytecode, extendedOperation) != RetVal::OK) {
        child_parse_trace.pos = initial_pos;
        child_parse_trace.status = EParseStatus::PARSE_ERROR_ASSIGNMENT_EXPRESSION;
        parse_trace.suberrors.push_back(child_parse_trace);

        pos = initial_pos;
        return RetVal::FAIL_STOP;
    }

    if (conditional_expression(s, pos, child_parse_trace, RIGHT, bytecode, datatype_right, scope) != RetVal::OK) {
        child_parse_trace.pos = initial_pos;
        child_parse_trace.status = EParseStatus::PARSE_ERROR_ASSIGNMENT_EXPRESSION;
        parse_trace.suberrors.push_back(child_parse_trace);

        pos = initial_pos;
        return RetVal::FAIL_STOP;
    }

    if ( isDatatypeConsistentAssignment(datatype_left, datatype_right) ) {
        switch (extendedOperation) {
            case EExtendedAssignOperation::NONE:
                break;
            case EExtendedAssignOperation::ADD:
                if ( !(
                    (isDatatypeNumeric(datatype_left) && isDatatypeNumeric(datatype_right))
                    || (datatype_left[0] == 's' && datatype_right[0] == 's')
                    || (datatype_left[0] == 'a' && datatype_right[0] == 'a')
                    ) )
                {
                    child_parse_trace.pos = initial_pos;
                    child_parse_trace.status = EParseStatus::PARSE_ERROR_ASSIGNMENT_DATATYPE;
                    parse_trace.suberrors.push_back(child_parse_trace);

                    pos = initial_pos;
                    return RetVal::FAIL_STOP;
                }
                break;
            case EExtendedAssignOperation::SUB:
                if ( !(
                    (isDatatypeNumeric(datatype_left) && isDatatypeNumeric(datatype_right))
                    || (datatype_left[0] == 's' && datatype_right[0] == 's')
                    || (datatype_left[0] == 'a' && datatype_right[0] == 'a')
                    ) )
                {
                    child_parse_trace.pos = initial_pos;
                    child_parse_trace.status = EParseStatus::PARSE_ERROR_ASSIGNMENT_DATATYPE;
                    parse_trace.suberrors.push_back(child_parse_trace);

                    pos = initial_pos;
                    return RetVal::FAIL_STOP;
                }
                break;
            case EExtendedAssignOperation::MUL:
            case EExtendedAssignOperation::DIV:
                if ( !(isDatatypeNumeric(datatype_left) && isDatatypeNumeric(datatype_right)) ) {
                    child_parse_trace.pos = initial_pos;
                    child_parse_trace.status = EParseStatus::PARSE_ERROR_ASSIGNMENT_DATATYPE;
                    parse_trace.suberrors.push_back(child_parse_trace);

                    pos = initial_pos;
                    return RetVal::FAIL_STOP;
                }
                break;
        } // ~switch

        bytecode += operator_bytecode;
        return RetVal::OK;
    } else {
        child_parse_trace.pos = initial_pos;
        child_parse_trace.status = EParseStatus::PARSE_ERROR_ASSIGNMENT_DATATYPE;
        parse_trace.suberrors.push_back(child_parse_trace);

        pos = initial_pos;
        return RetVal::FAIL_STOP;
    }
    

    // We shouldn't be here
    child_parse_trace.pos = initial_pos;
    child_parse_trace.status = EParseStatus::PARSE_ERROR_ASSIGNMENT_EXPRESSION;
    parse_trace.suberrors.push_back(child_parse_trace);

    pos = initial_pos;
    return RetVal::FAIL_CONTINUE;
}

RetVal Parser::index_type_declaration(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, std::string& datatype)
{
    CodeLocation initial_pos = pos;
    ParseTrace child_parse_trace;
    std::string encoded_datatype;

    do {
        whitespace(s, pos);

        if (check_string(s, pos, "int")) encoded_datatype += "i";
        else if (check_string(s, pos, "float")) encoded_datatype += "f";
        else if (check_string(s, pos, "string")) encoded_datatype += "s";
        else if (check_string(s, pos, "boolean")) encoded_datatype += "b";
        else {
            child_parse_trace.pos = initial_pos;
            child_parse_trace.status = EParseStatus::PARSE_ERROR_INDEX_TYPE_DECLARATION;
            parse_trace.suberrors.push_back(child_parse_trace);

            pos = initial_pos;
            return RetVal::FAIL_STOP;
        }

        whitespace(s, pos);

        if (check_symbol(s, pos, ',')) {
            encoded_datatype += ",";
            pos.pos++;
            pos.col++;
        } else {
            break;
        }
    } while(true);

    datatype = encoded_datatype;
    return RetVal::OK;
}

RetVal Parser::type_declaration(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, std::string& datatype)
{
    CodeLocation initial_pos = pos;
    ParseTrace child_parse_trace;

    whitespace(s, pos);

    if (check_string(s, pos, "int") && isTerminal(s, pos)) {
        datatype = "i";
        return RetVal::OK;
    }

    if (check_string(s, pos, "float") && isTerminal(s, pos)) {
        datatype = "f";
        return RetVal::OK;
    }

    if(check_string(s, pos, "string") && isTerminal(s, pos)) {
        datatype = "s";
        return RetVal::OK;
    }

    if(check_string(s, pos, "boolean") && isTerminal(s, pos)) {
        datatype = "b";
        return RetVal::OK;
    }

    if(check_string(s, pos, "array") && isTerminal(s, pos)) {
        whitespace(s, pos);
        if (check_symbol(s, pos, '[')) {
            std::string index_datatype;

            pos.pos++;
            pos.col++;

            if (index_type_declaration(s, pos, child_parse_trace, index_datatype) == RetVal::OK) {
                whitespace(s, pos);

                if (check_symbol(s, pos, ']')) {
                    pos.pos++;
                    pos.col++;

                    whitespace(s, pos);

                    if (check_string(s, pos, "of")) {
                        std::string sub_bytecode;

                        if (type_declaration(s, pos, child_parse_trace, sub_bytecode) == RetVal::OK) {
                            datatype = "a [" + index_datatype + "] " + sub_bytecode;
                            return RetVal::OK;
                        }
                    }
                    else {
                        ParseTrace sub_trace(pos, EParseStatus::PARSE_ERROR_TYPE_DECLARATION_MISSING_OF);
                        child_parse_trace.suberrors.push_back(sub_trace);
                    }
                }
                else {
                    ParseTrace sub_trace(pos, EParseStatus::PARSE_ERROR_TYPE_DECLARATION_MISSING_CLOSING_BRACKET);
                    child_parse_trace.suberrors.push_back(sub_trace);
                }
            }
        }
        else {
            ParseTrace sub_trace(pos, EParseStatus::PARSE_ERROR_TYPE_DECLARATION_MISSING_OPENING_BRACKET);
            child_parse_trace.suberrors.push_back(sub_trace);
        }

        child_parse_trace.pos = initial_pos;
        child_parse_trace.status = EParseStatus::PARSE_ERROR_TYPE_DECLARATION;
        parse_trace.suberrors.push_back(child_parse_trace);

        pos = initial_pos;
        return RetVal::FAIL_STOP;
    }

    // We have an error
    child_parse_trace.pos = initial_pos;
    child_parse_trace.status = EParseStatus::PARSE_ERROR_TYPE_DECLARATION;
    parse_trace.suberrors.push_back(child_parse_trace);

    pos = initial_pos;
    return RetVal::FAIL_CONTINUE;
}

RetVal Parser::variable_declaration(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, const Scope& scope, EDataMode data_mode, std::string& variable_datatype, unsigned int& variable_idx, std::vector<unsigned int>& function_variables)
{
    CodeLocation initial_pos = pos;
    ParseTrace child_parse_trace;
    std::string bytecode_id;
    Variable variable;
    RetVal child_ret;

    whitespace(s, pos);

    if ((child_ret = type_declaration(s, pos, child_parse_trace, variable_datatype)) != RetVal::OK) {
        child_parse_trace.pos = initial_pos;
        child_parse_trace.status = EParseStatus::PARSE_ERROR_VARIABLE_DECLARATION;
        parse_trace.suberrors.push_back(child_parse_trace);

        pos = initial_pos;
        return child_ret;
    }

    if (identifier(s, pos, child_parse_trace, variable, scope, Variables::EScopeRange::EXACT) != RetVal::OK) {
        child_parse_trace.pos = initial_pos;
        child_parse_trace.status = EParseStatus::PARSE_ERROR_VARIABLE_DECLARATION;
        parse_trace.suberrors.push_back(child_parse_trace);

        pos = initial_pos;
        return RetVal::FAIL_STOP;
    }

    if (variable.getIdx() == -1) { // Variable doesn't exist. Create it
        bool var_add = false;
        if (data_mode == EDataMode::STATIC) {
            var_add = variables.add(Variable(Variable::EVariableTypes::VARIABLE, scope, variable.getName(), variable_datatype, "", initial_pos), variable_idx);
        } else { // DYNAMIC
            var_add = variables.add(Variable(Variable::EVariableTypes::DYNAMIC_VARIABLE, scope, variable.getName(), variable_datatype, "", initial_pos), variable_idx);
        }
        function_variables.push_back(variable_idx);

        if (var_add) {
            return RetVal::OK;
        } else {
            child_parse_trace.pos = initial_pos;
            child_parse_trace.status = EParseStatus::PARSE_ERROR_DUPLICATE_VARIABLE_DECLARATION;
            parse_trace.suberrors.push_back(child_parse_trace);

            pos = initial_pos;
            return RetVal::FAIL_STOP;
        }
    } else {
        child_parse_trace.pos = initial_pos;
        child_parse_trace.status = EParseStatus::PARSE_ERROR_DUPLICATE_VARIABLE_DECLARATION;
        parse_trace.suberrors.push_back(child_parse_trace);

        pos = initial_pos;
        return RetVal::FAIL_STOP;
    }
    
    // We shouldn't be here
    child_parse_trace.pos = initial_pos;
    child_parse_trace.status = EParseStatus::PARSE_ERROR_VARIABLE_DECLARATION;
    parse_trace.suberrors.push_back(child_parse_trace);

    pos = initial_pos;
    return RetVal::FAIL_STOP;
}

void Parser::initVars(Bytecode& bytecode, const Scope& scope) {
    const std::vector<Variable>& v = variables.getVariables();

    for(int i=0; i<v.size(); i++) {
        if (v.at(i).getEntityType() == Variable::EVariableTypes::VARIABLE
        || v.at(i).getEntityType() == Variable::EVariableTypes::DYNAMIC_VARIABLE) {
            if (v.at(i).getScope() == scope) {
                bytecode.INITVAR(v.at(i).getIdx());
            }
        }
    }
}

RetVal Parser::function_parameters_declaration(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, std::vector<unsigned int>& parameter_idxs, const Scope& scope, std::string& parameter_types)
{
    CodeLocation initial_pos = pos;
    ParseTrace child_parse_trace;
    std::string encoded_datatype;
    std::string variable_datatype;
    unsigned int variable_idx;
    RetVal child_ret;
    EAction ret_action = EAction::CONTINUE;

    do {
        whitespace(s, pos);

        if ((child_ret = variable_declaration(s, pos, child_parse_trace, scope, EDataMode::DYNAMIC, variable_datatype, variable_idx, parameter_idxs)) == RetVal::OK) {
            encoded_datatype += variable_datatype;
        } else {
            child_parse_trace.pos = initial_pos;
            child_parse_trace.status = EParseStatus::PARSE_ERROR_FUNCTION_PARAMETERS_DECLARATION;
            parse_trace.suberrors.push_back(child_parse_trace);

            pos = initial_pos;
            return ret_action == EAction::STOP ? RetVal::FAIL_STOP : child_ret;
        }

        whitespace(s, pos);

        if (check_symbol(s, pos, ',')) {
            ret_action = EAction::STOP;

            encoded_datatype += ",";
            pos.pos++;
            pos.col++;
        } else {
            break;
        }
    } while(true);

    parameter_types = encoded_datatype;
    return RetVal::OK;
}

RetVal Parser::function_definition(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, Bytecode& bytecode, Bytecode& function_bytecode, const Scope& parent_scope)
{
    CodeLocation initial_pos = pos;
    ParseTrace child_parse_trace;
    std::string function_datatype;
    // std::string bytecode_id;
    std::vector<unsigned int> parameters;
    Variable function_id;
    bool is_first_function_statement = true;
    unsigned int code_pos=-1;
    std::string s_parameter_types;
    Scope function_scope(parent_scope);

    whitespace(s, pos);

    if (!(check_string(s, pos, "function")
        && isTerminal(s, pos)) )
    {
        child_parse_trace.pos = initial_pos;
        child_parse_trace.status = EParseStatus::PARSE_ERROR_FUNCTION_DEFINITION;
        parse_trace.suberrors.push_back(child_parse_trace);

        pos = initial_pos;
        return RetVal::FAIL_CONTINUE;
    }

    if (identifier(s, pos, child_parse_trace, function_id, parent_scope, Variables::EScopeRange::EXACT) != RetVal::OK) {
        child_parse_trace.pos = initial_pos;
        child_parse_trace.status = EParseStatus::PARSE_ERROR_FUNCTION_DEFINITION;
        parse_trace.suberrors.push_back(child_parse_trace);

        pos = initial_pos;
        return RetVal::FAIL_STOP;
    }
        
    if (function_id.getIdx() != -1) { // Function already declared
        child_parse_trace.pos = initial_pos;
        child_parse_trace.status = EParseStatus::PARSE_ERROR_FUNCTION_NAME_IN_USE;
        parse_trace.suberrors.push_back(child_parse_trace);

        pos = initial_pos;
        return RetVal::FAIL_STOP;
    }

    if (check_symbol(s, pos, '(')) {
        pos.pos++;
        pos.col++;
        function_scope.add(function_id.getName());
        if (function_parameters_declaration(s, pos, child_parse_trace, parameters, function_scope, s_parameter_types) == RetVal::OK) {
            whitespace(s, pos);

            if (check_symbol(s, pos, ')')) {
                pos.pos++;
                pos.col++;
            } else { // Missing ')'
                child_parse_trace.pos = initial_pos;
                child_parse_trace.status = EParseStatus::PARSE_ERROR_ARGUMENT_LIST_DECLARATION;
                parse_trace.suberrors.push_back(child_parse_trace);

                pos = initial_pos;
                return RetVal::FAIL_STOP;
            }
        } else {
            child_parse_trace.pos = initial_pos;
            child_parse_trace.status = EParseStatus::PARSE_ERROR_ARGUMENT_LIST_DECLARATION;
            parse_trace.suberrors.push_back(child_parse_trace);

            pos = initial_pos;
            return RetVal::FAIL_STOP;
        }
    }

    whitespace(s, pos);

    unsigned int fun_var_pos;
    if (check_string(s, pos, "of") && isTerminal(s, pos)) {
        if (type_declaration(s, pos, child_parse_trace, function_datatype) == RetVal::OK) {
            if (variables.add(Variable(Variable::EVariableTypes::FUNCTION, parent_scope, function_id.getName(), function_datatype, s_parameter_types, initial_pos), fun_var_pos)) {
                // Initialize function variable (used to pass a return value)
                code_pos = function_bytecode.ALLOCVAR(fun_var_pos);

                // variables.setFunctionRef(fun_var_pos, code_pos == -1 ? pos : code_pos);
                function_bytecode.addFunction(fun_var_pos, code_pos);

                // Initialize function parameters
                for (int i=parameters.size()-1; i>=0; i--) {
                    if (is_first_function_statement) {
                        code_pos = function_bytecode.ALLOCVARS(parameters[i]);
                        is_first_function_statement = false;
                    } else {
                        function_bytecode.ALLOCVARS(parameters[i]);
                    }
                } // ~for
            } else {
                child_parse_trace.pos = initial_pos;
                child_parse_trace.status = EParseStatus::PARSE_ERROR_FUNCTION_NAME_IN_USE;
                parse_trace.suberrors.push_back(child_parse_trace);

                pos = initial_pos;
                return RetVal::FAIL_STOP;
            }
        } else {
            child_parse_trace.pos = initial_pos;
            child_parse_trace.status = EParseStatus::PARSE_ERROR_FUNCTION_TYPE_DECLARATION;
            parse_trace.suberrors.push_back(child_parse_trace);

            pos = initial_pos;
            return RetVal::FAIL_STOP;
        }
    }

    Bytecode statement_bytecode;
    Bytecode statement_function_bytecode;
    std::vector<unsigned int> function_variables;   // Collects the variables defined by the function and underneeth block_statements to allocate them
    if (block_statement(s, pos, child_parse_trace, statement_bytecode, statement_function_bytecode, function_scope, Parser::EDataMode::DYNAMIC, function_variables) == RetVal::OK) {
        for(unsigned int i: function_variables) {
            function_bytecode.ALLOCVAR(i);
        }
        function_bytecode += statement_bytecode;
        function_bytecode.RETURN(fun_var_pos);
        function_bytecode += statement_function_bytecode;
        return RetVal::OK;
    }
    else {
        child_parse_trace.pos = initial_pos;
        child_parse_trace.status = EParseStatus::PARSE_ERROR_FUNCTION_DEFINITION;
        parse_trace.suberrors.push_back(child_parse_trace);

        pos = initial_pos;
        return RetVal::FAIL_STOP;
    }

    // We shouldn't be here
    child_parse_trace.pos = initial_pos;
    child_parse_trace.status = EParseStatus::PARSE_ERROR_FUNCTION_DEFINITION;
    parse_trace.suberrors.push_back(child_parse_trace);

    pos = initial_pos;
    return RetVal::FAIL_CONTINUE;
}

RetVal Parser::block_statement(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, Bytecode& bytecode, Bytecode& function_bytecode, const Scope& scope, EDataMode data_mode, std::vector<unsigned int>& function_variables)
{
    CodeLocation initial_pos = pos;
    ParseTrace child_parse_trace;

    whitespace(s, pos);

    if (check_symbol(s, pos, '{')) {
        pos.pos++;
        pos.col++;

        if (translation_unit(s, pos, child_parse_trace, bytecode, function_bytecode, scope, data_mode, function_variables) == RetVal::OK) {
            if (check_symbol(s, pos, '}')) {
                pos.pos++;
                pos.col++;

                return RetVal::OK;
            } else {
                child_parse_trace.pos = initial_pos;
                child_parse_trace.status = EParseStatus::PARSE_ERROR_BLOCK_STATEMENT;
                parse_trace.suberrors.push_back(child_parse_trace);

                pos = initial_pos;
                return RetVal::FAIL_STOP;
            }
        } else {
            child_parse_trace.pos = initial_pos;
            child_parse_trace.status = EParseStatus::PARSE_ERROR_BLOCK_STATEMENT;
            parse_trace.suberrors.push_back(child_parse_trace);

            pos = initial_pos;
            return RetVal::FAIL_STOP;
        }
    }
    
    child_parse_trace.pos = initial_pos;
    child_parse_trace.status = EParseStatus::PARSE_ERROR_BLOCK_STATEMENT;
    parse_trace.suberrors.push_back(child_parse_trace);

    pos = initial_pos;
    return RetVal::FAIL_CONTINUE;
}

RetVal Parser::translation_unit(std::string const& s, CodeLocation& pos, ParseTrace& parse_trace, Bytecode& bytecode, Bytecode& function_bytecode, const Scope& scope, EDataMode data_mode, std::vector<unsigned int>& function_variables)
{
    CodeLocation initial_pos = pos;
    Bytecode statement_bytecode;
    Bytecode statement_function_bytecode;
    int subscope_count = 0;
    std::string variable_datatype;
    unsigned int variable_idx;
    RetVal child_ret;

    whitespace(s, pos);

    do {
        ParseTrace child_parse_trace;
        statement_bytecode.clear();
        statement_function_bytecode.clear();

        if ((child_ret = assignment_expression(s, pos, child_parse_trace, statement_bytecode, scope)) == RetVal::OK) {
            bytecode += statement_bytecode;
        } else if (child_ret == RetVal::FAIL_CONTINUE
            && (child_ret = variable_declaration(s, pos, child_parse_trace, scope, data_mode, variable_datatype, variable_idx, function_variables)) == RetVal::OK)
        {
            bytecode += statement_bytecode;
        } else if (child_ret == RetVal::FAIL_CONTINUE
            && (child_ret = block_statement(s, pos, child_parse_trace, statement_bytecode, statement_function_bytecode, Scope(scope, std::to_string(subscope_count)), data_mode, function_variables)) == RetVal::OK)
        {
            initVars(bytecode, Scope(scope, std::to_string(subscope_count)));
            bytecode += statement_bytecode;
            function_bytecode += statement_function_bytecode;
            subscope_count++;
        } else if (child_ret == RetVal::FAIL_CONTINUE
            && (child_ret = function_definition(s, pos, child_parse_trace, statement_bytecode, statement_function_bytecode, scope)) == RetVal::OK)
        {
            function_bytecode += statement_bytecode;
            function_bytecode += statement_function_bytecode;
            subscope_count++;
        } else if (child_ret == RetVal::FAIL_CONTINUE
            && !scope.isRoot()
            && check_symbol(s, pos, '}'))
        { // For nested scope terminate on '}'
            return RetVal::OK;
        } else {
            child_parse_trace.pos = initial_pos;
            child_parse_trace.status = EParseStatus::PARSE_ERROR_COMPILATION_UNIT;
            parse_trace.suberrors.push_back(child_parse_trace);

            pos = initial_pos;
            return child_ret;
        }

        whitespace(s, pos);

        if (s.length() <= pos.pos) { // EOF
            break;
        }
    } while(true);

    bytecode.END();

    return RetVal::OK;
}

EParseStatus Parser::parse(std::string const& s, ParseTrace& parse_trace, Bytecode& bytecode)
{
    Bytecode function_bytecode;
    CodeLocation pos(0, 0, 0);
    Scope scope("0");
    Bytecode translation_bytecode;

    bytecode.clear();

    printf("Text to parse: %s\n", s.c_str());

//    ETokenizeStatus tokenize_ret = tokenize(s);

    // Declare predefined constants
    unsigned int var_true_pos, var_false_pos; // if OK - new variable index in the variable array; if error - code position of the previous declaration
    variables.add(Variable(Variable::EVariableTypes::BUILTIN_VARIABLE, scope, "true", "b", "", CodeLocation()), var_true_pos);
    variables.add(Variable(Variable::EVariableTypes::BUILTIN_VARIABLE, scope, "false", "b", "", CodeLocation()), var_false_pos);

    // Declare predefined functions
    unsigned int fun_idx;
    unsigned int fun_pos; // function address to add to the reference table

    // SIN(x)
    {
        Scope sin_scope(scope, "sin");
        unsigned int sin_par_idx;
        variables.add(Variable(Variable::EVariableTypes::BUILTIN_FUNCTION, scope, "sin", "f", "f", CodeLocation()), fun_idx);
        variables.add(Variable(Variable::EVariableTypes::DYNAMIC_VARIABLE, sin_scope, "x", "f", "", CodeLocation()), sin_par_idx);

        fun_pos = function_bytecode.ALLOCVAR(fun_idx);
        function_bytecode.ALLOCVARS(sin_par_idx);
        function_bytecode.PUTDADDR(fun_idx);
        function_bytecode.SYSCALL(sin_par_idx, std::string("sin").data());  // SYSCALL puts the result to the stack
        function_bytecode.MOVE();
        function_bytecode.RETURN(fun_idx);
        function_bytecode.addFunction(fun_idx, fun_pos);
        // variables.setFunctionRef(fun_idx, fun_pos);
    }
    // COS(x)
    {
        Scope cos_scope(scope, "cos");
        unsigned int cos_par_idx;
        variables.add(Variable(Variable::EVariableTypes::BUILTIN_FUNCTION, scope, "cos", "f", "f", CodeLocation()), fun_idx);
        variables.add(Variable(Variable::EVariableTypes::DYNAMIC_VARIABLE, cos_scope, "x", "f", "", CodeLocation()), cos_par_idx);

        fun_pos = function_bytecode.ALLOCVAR(fun_idx);
        function_bytecode.ALLOCVARS(cos_par_idx);
        function_bytecode.PUTDADDR(fun_idx);
        function_bytecode.SYSCALL(cos_par_idx, std::string("cos").data());  // SYSCALL puts the result to the stack
        function_bytecode.MOVE();
        function_bytecode.RETURN(fun_idx);
        function_bytecode.addFunction(fun_idx, fun_pos);
        // variables.setFunctionRef(fun_idx, fun_pos);
    }

    std::vector<unsigned int> function_variables; // Basically unused on the root level. This is used for allocating funtion-level variables.
    RetVal ret = translation_unit(s, pos, parse_trace, translation_bytecode, function_bytecode, scope, EDataMode::STATIC, function_variables);

    if (ret == RetVal::OK) {
        initVars(bytecode, scope);

        // Init predefined variables
        bytecode.PUTADDR(var_true_pos);
        bytecode.PUTBOOLEAN(true);
        bytecode.MOVE();

        bytecode.PUTADDR(var_false_pos);
        bytecode.PUTBOOLEAN(false);
        bytecode.MOVE();

        bytecode += translation_bytecode;

        // Adding variables and function table to the bytecode
        unsigned int function_section_pos = bytecode.getCode().size();
        bytecode += function_bytecode;

        // variables.reallocateFunctionRef(function_section_pos);
        variables.updateFunctionRefs(bytecode);


        // They are added to the Variables not to the Code
        for(const Variable& v: variables.getVariables()) {
            if (v.getEntityType() == Variable::EVariableTypes::BUILTIN_VARIABLE || v.getEntityType() == Variable::EVariableTypes::VARIABLE) {
                bytecode.DATA(v.getName().data(), v.getScope().toString().data(), v.getType().data());
            } else if (v.getEntityType() == Variable::EVariableTypes::DYNAMIC_VARIABLE) {
                bytecode.DDATA(v.getName().data(), v.getScope().toString().data(), v.getIdx(), v.getType().data());
            } else if (v.getEntityType() == Variable::EVariableTypes::BUILTIN_FUNCTION || v.getEntityType() == Variable::EVariableTypes::FUNCTION) {
                bytecode.FUN(v.getName().data(), v.getScope().toString().data(), v.getType().data(), v.getType_fun_params().data(), v.getFunctionRef());
            }
        }
    }

    bytecode.print("compile.bant");

    if (ret != RetVal::OK) {
        parse_trace.pos.set(0, 0, 0);
        parse_trace.status = EParseStatus::PARSE_ERROR;
        return EParseStatus::PARSE_ERROR;
    }

    return EParseStatus::PARSE_OK;
}

void Parser::clear()
{
    variables.clear();
}
