#pragma once
#include "AST.h"
#include <list>
#include <string>
#include <map>

enum class TypeTag
{
    INT,
    REAL,
    BOOL,
    VOID,

    DERIVED,
    FUNCTION,
    VARIABLE
};

class TypeEntry
{
    // only for dynamic cast to work
    virtual void foo() { }
    
public:
    const std::string name;

    TypeEntry(const std::string& name) : name{ name }
    {

    }

    friend std::ostream& operator<<(std::ostream&, const TypeEntry&);
};

struct TypeLog
{
    int refCount;
    int index;
    int width;

    TypeTag entryType;
    TypeEntry* structure;

    friend std::ostream& operator<<(std::ostream&, const TypeLog&);
};

// Derived Classes
class FuncEntry : public TypeEntry
{
public:
    int identifierCount = 0;
    std::list<std::pair<std::string, TypeLog*>> argTypes;
    std::list<std::pair<std::string, TypeLog*>> retTypes;
    std::map<std::string, TypeLog*> symbolTable;

    FuncEntry(const std::string& name) : TypeEntry(name)
    {

    }
};

class VariableEntry : public TypeEntry
{
public:
    int offset = 0;
    bool isGlobal = 0;
    TypeLog* type = nullptr;

    VariableEntry(const std::string& name) : TypeEntry(name)
    {

    }
};

class DerivedEntry : public TypeEntry
{
public:
    const bool isUnion;
    std::list<std::pair<std::string, TypeLog*>> fields;

    DerivedEntry(const std::string& name, bool isUnion) : TypeEntry(name), isUnion {isUnion}
    {

    }
};

extern int dataTypeCount;
extern int identifierCount;
extern std::map<std::string, TypeLog*> globalSymbolTable;
extern std::map<std::string, TokenType> prefixTable;

void loadSymbolTable(const ASTNode*);