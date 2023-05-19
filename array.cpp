#include "array.h"
#include <iostream>
#include <locale>
#include <codecvt>

/********************************************************
 * ValuePointer
 ********************************************************/

ValuePointer::ValuePointer() : pvalue(NULL) {}
ValuePointer::ValuePointer(const std::string& datatype, void* pvalue) : datatype(datatype), pvalue(pvalue) {
}
ValuePointer::ValuePointer(const ValuePointer& other) {
    datatype = other.datatype;
    pvalue = other.pvalue;
}

std::string ValuePointer::toString() const {
    std::string ret;

    ret = datatype + std::string(" : ");
    switch(datatype[0]) {
        case 'i':
            ret += std::to_string( *(long long int*)pvalue );
            break;
        case 'f':
            ret += std::to_string( (double) *(long double*)pvalue );
            break;
        case 'b':
            ret += *(bool*)pvalue ? "true" : "false";
            break;
        case 's':
            ret += *(std::string*)pvalue;
            break;
        case 'a':
            ret += (*(Array*)pvalue).toString();
            break;
    }

    return ret;
}

bool ValuePointer::operator==(const ValuePointer& other) {
    char this_datatype_1 = datatype[0];
    char other_datatype_1 = other.datatype[0];

    if (this_datatype_1 == 'i' && other_datatype_1 == 'i') {
        return (*(long long int*)pvalue == *(long long int*)other.pvalue);
    } else if (this_datatype_1 == 'f' && other_datatype_1 == 'f') {
        return (*(long double*)pvalue == *(long double*)other.pvalue);
    } else if (this_datatype_1 == 'b' && other_datatype_1 == 'b') {
        return (*(bool*)pvalue == *(bool*)other.pvalue);
    } else if (this_datatype_1 == 's' && other_datatype_1 == 's') {
        return (*(std::string*)pvalue == *(std::string*)other.pvalue);
    } else if (this_datatype_1 == 'a' && other_datatype_1 == 'a') {
        return (*(Array*)pvalue == *(Array*)other.pvalue);
    }

    return false;
}

bool ValuePointer::operator!=(const ValuePointer& other) {
    char this_datatype_1 = datatype[0];
    char other_datatype_1 = other.datatype[0];

    if (this_datatype_1 == 'i' && other_datatype_1 == 'i') {
        return (*(long long int*)pvalue != *(long long int*)other.pvalue);
    } else if (this_datatype_1 == 'f' && other_datatype_1 == 'f') {
        return (*(long double*)pvalue != *(long double*)other.pvalue);
    } else if (this_datatype_1 == 'b' && other_datatype_1 == 'b') {
        return (*(bool*)pvalue != *(bool*)other.pvalue);
    } else if (this_datatype_1 == 's' && other_datatype_1 == 's') {
        return (*(std::string*)pvalue != *(std::string*)other.pvalue);
    } else if (this_datatype_1 == 'a' && other_datatype_1 == 'a') {
        return (*(Array*)pvalue != *(Array*)other.pvalue);
    }

    return true;
}

ValuePointer& ValuePointer::operator+=(const ValuePointer& other) {
    char this_datatype_1 = datatype[0];
    char other_datatype_1 = other.datatype[0];

    if (this_datatype_1 == 'i' && other_datatype_1 == 'i') {
        (*(long long int*)pvalue += *(long long int*)other.pvalue);
    } else if (this_datatype_1 == 'f' && other_datatype_1 == 'f') {
        (*(long double*)pvalue += *(long double*)other.pvalue);
    } else if (this_datatype_1 == 'b' && other_datatype_1 == 'b') {
        (*(bool*)pvalue += *(bool*)other.pvalue);
    } else if (this_datatype_1 == 's' && other_datatype_1 == 's') {
        (*(std::string*)pvalue += *(std::string*)other.pvalue);
    } else if (this_datatype_1 == 'a' && other_datatype_1 == 'a') {
        (*(Array*)pvalue += *(Array*)other.pvalue);
    }

    return *this;
}

/********************************************************
 * ArrayElement
 ********************************************************/
ArrayElement::ArrayElement() {
}

// ArrayElement::ArrayElement(const ValuePointer& pval): datatype(pval.datatype), value(pval.pvalue) {
// }

ArrayElement::ArrayElement(const ArrayElement& other) {
    // index_datatypes = other.index_datatypes; 'index-datatypes' will be filled in below in 'addIndex'
    value.datatype = other.value.datatype;

    // Copy indexes
    for (int i=0; i<other.indexes.size(); i++) {
        switch(other.indexes[i].datatype[0]) {
            case 'i':
                addIndex(*static_cast<long long int*>(other.indexes[i].pvalue));
                break;
            case 'f':
                addIndex(*static_cast<long double*>(other.indexes[i].pvalue));
                break;
            case 'b':
                addIndex(*static_cast<bool*>(other.indexes[i].pvalue));
                break;
            case 's':
                addIndex(*static_cast<std::string*>(other.indexes[i].pvalue));
                break;
        }
    }

    // Copy value
    switch(other.value.datatype[0]) {
        case 'i':
            setValue(*static_cast<long long int*>(other.value.pvalue));
            break;
        case 'f':
            setValue(*static_cast<long double*>(other.value.pvalue));
            break;
        case 'b':
            setValue(*static_cast<bool*>(other.value.pvalue));
            break;
        case 's':
            setValue(*static_cast<std::string*>(other.value.pvalue));
            break;
        case 'a':
            setValue(other.value.datatype, *static_cast<Array*>(other.value.pvalue));
            break;
    }
}

ArrayElement::~ArrayElement() {
    // Delete the element's value
    std::cout << "Delete ArrayElement: " << value.datatype.c_str() << std::endl;

    clear();
}

bool ArrayElement::operator==(const ArrayElement& other) {
    return value == other.value;
}

bool ArrayElement::operator!=(const ArrayElement& other) {
    return value != other.value;
}

void ArrayElement::clear_value() {
    // Delete the element's value

    if (!value.datatype.empty()) {
        if (value.pvalue != NULL) {
            switch(value.datatype[0]) {
                case 'i':
                    delete static_cast<long long int*>(value.pvalue);
                    break;
                case 'f':
                    delete static_cast<long double*>(value.pvalue);
                    break;
                case 'b':
                    delete static_cast<bool*>(value.pvalue);
                    break;
                case 's':
                    delete static_cast<std::string*>(value.pvalue);
                    break;
                case 'a':
                    delete static_cast<Array*>(value.pvalue);
                    break;
            }
        }
    }

    value.pvalue = NULL;
    value.datatype = "";
}

void ArrayElement::clear() {
    clear_value();

    // Release indexes
    for (int i=0; i<indexes.size(); i++) {
        switch(indexes[i].datatype[0]) {
            case 'i':
                delete static_cast<long long int*>(indexes[i].pvalue);
                break;
            case 'f':
                delete static_cast<long double*>(indexes[i].pvalue);
                break;
            case 'b':
                delete static_cast<bool*>(indexes[i].pvalue);
                break;
            case 's':
                delete static_cast<std::string*>(indexes[i].pvalue);
                break;
        }

        indexes[i].pvalue = NULL;
    }
}

void ArrayElement::operator= (const ArrayElement& other) {
    // index_datatypes = other.index_datatypes; 'index-datatypes' will be filled in below in 'addIndex'
    value.datatype = other.value.datatype;

    clear();

    // Copy indexes
    for (int i=0; i<other.indexes.size(); i++) {
        switch(other.indexes[i].datatype[0]) {
            case 'i':
                addIndex(*static_cast<long long int*>(other.indexes[i].pvalue));
                break;
            case 'f':
                addIndex(*static_cast<long double*>(other.indexes[i].pvalue));
                break;
            case 'b':
                addIndex(*static_cast<bool*>(other.indexes[i].pvalue));
                break;
            case 's':
                addIndex(*static_cast<std::string*>(other.indexes[i].pvalue));
                break;
        }
    }

    // Copy value
    switch(other.value.datatype[0]) {
        case 'i':
            setValue(*static_cast<long long int*>(other.value.pvalue));
            break;
        case 'f':
            setValue(*static_cast<long double*>(other.value.pvalue));
            break;
        case 'b':
            setValue(*static_cast<bool*>(other.value.pvalue));
            break;
        case 's':
            setValue(*static_cast<std::string*>(other.value.pvalue));
            break;
        case 'a':
            setValue(other.value.datatype, *static_cast<Array*>(other.value.pvalue));
            break;
    }
}

ArrayElement& ArrayElement::operator+=(const ArrayElement& other) {
    value += other.value;

    return *this;
}

void ArrayElement::addIndex(long long int idx) {
    long long int* p = new long long int;
    *p = idx;
    indexes.push_back(ValuePointer("i", p));
}
void ArrayElement::addIndex(long double idx) {
    long double* p = new long double;
    *p = idx;
    indexes.push_back(ValuePointer("f", p));
}
void ArrayElement::addIndex(bool idx) {
    bool* p = new bool;
    *p = idx;
    indexes.push_back(ValuePointer("b", p));
}
void ArrayElement::addIndex(const std::string& idx) {
    std::string* p = new std::string;
    *p = idx;
    indexes.push_back(ValuePointer("s", p));
}

void* ArrayElement::setValue(long long int v) {
    clear_value();
    value.datatype = "i";
    value.pvalue = new long long int;
    *(long long int*)value.pvalue = v;
    return value.pvalue;
}
void* ArrayElement::setValue(long double v) {
    clear_value();
    value.datatype = "f";
    value.pvalue = new long double;
    *(long double*)value.pvalue = v;
    return value.pvalue;
}
void* ArrayElement::setValue(bool v) {
    clear_value();
    value.datatype = "b";
    value.pvalue = new bool;
    *(bool*)value.pvalue = v;
    return value.pvalue;
}
void* ArrayElement::setValue(const std::string& v) {
    clear_value();
    value.datatype = "s";
    value.pvalue = new std::string;
    *(std::string*)value.pvalue = v;
    return value.pvalue;
}
void* ArrayElement::setValue(const std::string& datatype, const Array& v) {
    clear_value();
    value.datatype = datatype;
    value.pvalue = new Array;
    *(Array*)value.pvalue = v;
    return value.pvalue;
}

ValuePointer& ArrayElement::getValue() {
    return value;
}

bool ArrayElement::checkIndexes(const std::vector<ValuePointer>& search_indexes) {
    for (int i=0; i<indexes.size(); i++) {
        char datatype_code = indexes[i].datatype[0];
        char search_datatype_code = search_indexes[i].datatype[0];

        if (datatype_code == 'i' && search_datatype_code == 'i') {
            if (*(long long int*)(indexes[i].pvalue) != *(long long int*)(search_indexes[i].pvalue)) {
                return false;
            }
        } else if (datatype_code == 'f' && search_datatype_code == 'i') {
            if (*(long double*)(indexes[i].pvalue) != *(long long int*)(search_indexes[i].pvalue)) {
                return false;
            }
        } else if (datatype_code == 'f' && search_datatype_code == 'f') {
            if (*(long double*)(indexes[i].pvalue) != *(long double*)(search_indexes[i].pvalue)) {
                return false;
            }
        } else if (datatype_code == 'b' && search_datatype_code == 'b') {
            if (*(bool*)(indexes[i].pvalue) != *(bool*)(search_indexes[i].pvalue)) {
                return false;
            }
        } else if (datatype_code == 's' && search_datatype_code == 's') {
            if (*(std::string*)(indexes[i].pvalue) != *(std::string*)(search_indexes[i].pvalue)) {
                return false;
            }
        } else {
            return false; // Inconsistent index datatypes
        }
    }

    return true;
}

const std::vector<ValuePointer>& ArrayElement::getIndexes() {
    return indexes;
}

std::string ArrayElement::indexesToString() const {
    std::string ret;
    bool is_first = true;

    ret = "[";
    for (int i = 0; i < indexes.size(); i++) {
        ret += (is_first ? "" : ",") + indexes[i].toString();
        is_first = false;
    }
    ret += "] ";

    return ret;
}

std::string ArrayElement::valueToString() const {
    std::string ret;

    switch (value.datatype[0]) {
    case 'i':
        ret = std::to_string(*(long long int*)value.pvalue);
        break;
    case 'f':
        ret = std::to_string((double)*(long double*)value.pvalue);
        break;
    case 'b':
        ret = *(bool*)value.pvalue ? "true" : "false";
        break;
    case 's':
        ret = *(std::string*)value.pvalue;
        break;
    case 'a':
        ret = (*(Array*)value.pvalue).toString();
        break;
    }

    return ret;
}

std::string ArrayElement::toString() const {
    std::string ret;
    
    ret = indexesToString();
    ret += "[" + value.datatype + "] ";
    ret += valueToString();

    return ret;
}

/*************************************************
 * Array
 *************************************************/

Array::Array() {}
Array::Array(const std::vector<char>& index_datatypes, const std::string& element_datatype) {
    setIndexDatatypes(index_datatypes);
    setElementDatatype(element_datatype);
}
Array::Array(const Array& other) {
    index_datatypes = other.index_datatypes;
    element_datatype = other.element_datatype;

    for (int i=0; i<other.elements.size(); i++) {
        ArrayElement* el = new ArrayElement(*other.elements[i]);
        addElement(el);
    }
}
Array::~Array() {
    std::cout << "Delete Array: " << element_datatype << std::endl;

    clear();
}
void Array::addElement(ArrayElement* element) {
    elements.push_back(element);
}

void Array::clear() {
    index_datatypes.clear();
    element_datatype = "";

    clearElements();
}

void Array::clearElements() {
    for (int i=0; i<elements.size(); i++) {
        delete elements[i];
        elements[i] = NULL;
    }

    elements.clear();
}

void Array::operator=(const Array& other) {
    clear();

    index_datatypes = other.index_datatypes;
    element_datatype = other.element_datatype;

    for (int i=0; i<other.elements.size(); i++) {
        ArrayElement* el = new ArrayElement(*other.elements[i]);
        addElement(el);
    }
}

bool Array::operator==(Array& other) {
    if (elements.size() != other.elements.size()) {
        return false;
    }

    for (int i=0; i<elements.size(); i++) {
        ArrayElement* el = other.getElement(elements[i]->getIndexes(), false);

        if (el == NULL) {
            return false;
        }

        if (*(elements[i]) != *el) {
            return false;
        }
    }

    return true;
}

bool Array::operator!=(Array& other) {
    if (elements.size() != other.elements.size()) {
        return true;
    }

    for (int i=0; i<elements.size(); i++) {
        ArrayElement* el = other.getElement(elements[i]->getIndexes(), false);

        if (el == NULL) {
            return true;
        }

        if (*(elements[i]) != *el) {
            return true;
        }
    }

    return false;
}

Array Array::operator+(const Array& other) {
    Array ret = *this;

    if (areArraysCompatible(ret.getDatatypeString(), other.getDatatypeString()) ) {
        for(ArrayElement* el: other.elements) {
            ArrayElement* new_el = ret.getElement(el->getIndexes(), true);
            *new_el += *el;
        }
    }

    return ret;
}

Array& Array::operator+=(const Array& other) {
    if (areArraysCompatible(getDatatypeString(), other.getDatatypeString()) ) {
        for(ArrayElement* el: other.elements) {
            ArrayElement* new_el = getElement(el->getIndexes(), true);
            *new_el += *el;
        }
    }

    return *this;
}

const std::vector<char>& Array::getIndexDatatypes() const {
    return index_datatypes;
}

const std::string& Array::getElementDatatype() const {
    return element_datatype;
}

void Array::setIndexDatatypes(const std::vector<char>& datatypes) {
    index_datatypes = datatypes;
}
void Array::setElementDatatype(const std::string& datatype) {
    element_datatype = datatype;
}

// Search for the element with given index values
// If doesn't exists then create it and return it's address
ArrayElement* Array::getElement(const std::vector<ValuePointer>& indexes, bool create) {

    for (int i=0; i<elements.size(); i++) {
        int s = indexes.size();
        if (elements[i]->checkIndexes(indexes)) {
            return elements[i];
        }
    }

    // If we are here, the element doesn't exist.
    if (!create) {
        return NULL;
    }

    // Create the empty element

    ArrayElement* new_element = new ArrayElement();

    // Create indexes for the new element
    for (int i=0; i<index_datatypes.size(); i++) {
        switch(index_datatypes[i]) {
            case 'i':
                new_element->addIndex(*(long long int*)(indexes[i].pvalue));
                break;
            case 'f':
                new_element->addIndex(*(long double*)(indexes[i].pvalue));
                break;
            case 'b':
                new_element->addIndex(*(bool*)(indexes[i].pvalue));
                break;
            case 's':
                new_element->addIndex(*(std::string*)(indexes[i].pvalue));
                break;
        }
    }

    // Add an empty value to the new element
    void* p_var = NULL;

    switch (element_datatype[0]) {
        case 'i':
            p_var = new_element->setValue((long long int)0);
            break;
        case 'f':
            p_var = new_element->setValue((long double)0.0);
            break;
        case 'b':
            p_var = new_element->setValue(true);
            break;
        case 's':
            p_var = new_element->setValue(std::string());
            break;
        case 'a': {
            std::vector<char> subelement_index_datatypes;
            int i;
            int s_size = element_datatype.size();
            // Get index datatypes
            for(i=3; i<s_size; i++) {
                char c = element_datatype[i];

                if (c == ']') {
                    i+=2; // Skip ']' and following ' '
                    break;
                }
                if (c == ',') continue;
                subelement_index_datatypes.push_back(c);
            }
            // Get element datatype
            std::string subelement_datatype = std::string(element_datatype.substr(i)); // right(s_size - i));
            p_var = new_element->setValue(element_datatype, Array(subelement_index_datatypes, subelement_datatype));

            break;
        }
    } // ~switch

    elements.push_back(new_element);

    return new_element;
}

std::string Array::toString() const {
    std::string ret;

    for (int i=0; i<elements.size(); i++) {
        ret += elements[i]->toString() + "\n";
    }

    return ret;
}

std::string Array::getDatatypeString() const {
    std::string ret = "a [";
    bool first = true;

    for(char indx_type: index_datatypes) {
        ret += (first ? "" : ",") + indx_type;
        first = false;
    }

    ret += "] " + element_datatype;

    return ret;
}

bool Array::areArraysCompatible(const std::string& datatype1, const std::string& datatype2) {
    if (datatype1.size() != datatype2.size())
        return false;

    for (int i=0; i<datatype1.size(); i++) {
        if (datatype1[i] != datatype2[i]) {
            if (datatype1[i] != 'f' || datatype2[i] != 'i')
                return false;
        }
    }

    return true;
}

const std::vector<ArrayElement*> Array::getElements() const {
    return elements;
}