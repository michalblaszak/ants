#ifndef ARRAY_H
#define ARRAY_H

#include <string>
#include <vector>

class Array;

// The helper structure to pass pointer to values with datatype association
// The value MUST NOT be cloned!!!
// This is the shared pointer.
// It doesn't own the pvalue
struct ValuePointer
{
    std::string datatype;
    void* pvalue;

    ValuePointer();
    ValuePointer(const std::string& datatype, void* pvalue);
    ValuePointer(const ValuePointer& other);

    bool operator==(const ValuePointer& other);
    bool operator!=(const ValuePointer& other);
    ValuePointer& operator+=(const ValuePointer& other);

    std::string toString() const;
};

class ArrayElement
{
private:
    std::vector<ValuePointer> indexes;
    ValuePointer value;

public:
    ArrayElement();
    ArrayElement(const ValuePointer& pval);
    ArrayElement(const ArrayElement& other);
    ~ArrayElement();
    bool operator==(const ArrayElement& other);
    bool operator!=(const ArrayElement& other);
    void clear();
    void clear_value();
    void operator= (const ArrayElement& other);
    ArrayElement& operator+=(const ArrayElement& other);
    void addIndex(long long int idx);
    void addIndex(long double idx);
    void addIndex(bool idx);
    void addIndex(const std::string& idx);
    void* setValue(long long int v);
    void* setValue(long double v);
    void* setValue(bool v);
    void* setValue(const std::string& v);
    void* setValue(const std::string& datatype, const Array& v);
    ValuePointer& getValue();
    bool checkIndexes(const std::vector<ValuePointer>& search_indexes);
    std::string indexesToString() const;
    std::string valueToString() const;
    std::string toString() const;
    const std::vector<ValuePointer>& getIndexes();
};

class Array
{
private:
    std::vector<char> index_datatypes;
    std::string element_datatype;
    std::vector<ArrayElement*> elements;

public:
    Array();
    Array(const std::vector<char>& index_datatypes, const std::string& element_datatype);
    Array(const Array& other);
    ~Array();
    void addElement(ArrayElement* element);

    void clear();
    void clearElements();

    void operator=(const Array& other);
    bool operator==(Array& other);
    bool operator!=(Array& other);
    Array operator+(const Array& other);
    Array& operator+=(const Array& other);

    const std::vector<char>& getIndexDatatypes() const;
    const std::string& getElementDatatype() const;

    void setIndexDatatypes(const std::vector<char>& datatypes);
    void setElementDatatype(const std::string& datatype);

    const std::vector<ArrayElement*> getElements() const;

    // Search for the element with given index values
    // If doesn't exists then create it and return it's address
    ArrayElement* getElement(const std::vector<ValuePointer>& indexes, bool create=true);

    std::string toString() const;

    std::string getDatatypeString() const;
    static bool areArraysCompatible(const std::string& datatype1, const std::string& datatype2);
};



#endif // ARRAY_H
