#include "SymbolTable.h"
#include <iostream>
#include <string>
#include <map>
using namespace std;

map<string, TypeLog*> globalSymbolTable;
map<string, TokenType> prefixTable;

int dataTypeCount = 0;
int identifierCount = 0;

vector<TypeLog*> structList;
vector<vector<int>> adj;

std::ostream& operator<<(std::ostream& out, const TypeEntry& entry)
{
    out << "{ name: " << entry.name << " }";
    return out;
}

std::ostream& operator<<(std::ostream& out, const TypeLog& type)
{
    out << "{ refCount: " << type.refCount << ", index: " << type.index << ", width: " << type.width << ", entryType: " << (
        type.entryType == TypeTag::INT ? "int" :
        type.entryType == TypeTag::REAL ? "real" :
        type.entryType == TypeTag::BOOL ? "##bool" :
        type.entryType == TypeTag::VOID ? "##void" :
        type.entryType == TypeTag::DERIVED ? "derived" :
        type.entryType == TypeTag::FUNCTION ? "function" :
        type.entryType == TypeTag::VARIABLE ? "variable" :
        "unknown") << ", structure: ";

    if (type.structure)
        out << *type.structure;
    else
        out << "(null)";

    out << " }";

    return out;
}

void dfs(int v, vector<bool>& visited, vector<int>& ans)
{
    visited[v] = true;

    for (int i = 0; i < dataTypeCount; i++)
        if (!visited[i] && adj[v][i] > 0)
            dfs(i, visited, ans);

    ans.push_back(v);
}

vector<int> topological_sort()
{
    vector<bool> visited;
    vector<int> ans;

    visited.assign(dataTypeCount, false);
    ans.clear();

    for (int i = 0; i < dataTypeCount; ++i)
        if (!visited[i])
            dfs(i, visited, ans);

    reverse(ans.begin(), ans.end());
    return ans;
}

void firstPass(const ASTNode* node, bool processTypedef = false)
{
    if (!node)
        return;

    if (node->sym_index == 57)
    {
        // <program> -> <funcList> <mainFunction>

        firstPass(node->children[0]);
        firstPass(node->children[1]);

        firstPass(node->children[0], true);
        firstPass(node->children[1], true);
    }
    else if (node->sym_index == 60 || node->sym_index == 58)
    {
        // <function> -> <inputList><outputList> <stmts>

        globalSymbolTable[node->token->lexeme] = new TypeLog
        {
            1,
            identifierCount++,
            -1,
            TypeTag::FUNCTION,
            new FuncEntry(node->token->lexeme)
        };

        firstPass(node->children[2], processTypedef);
    }
    else if (node->sym_index == 68)
    {
        // <stmts> -> <definitions> <declarations> <funcBody> <return>

        firstPass(node->children[0], processTypedef);
    }
    else if (node->sym_index == 71 && !processTypedef)
    {
        // typedefinition
        prefixTable[node->children[0]->token->lexeme] = node->token->type;

        globalSymbolTable[node->children[0]->token->lexeme] = new TypeLog
        {
            1,
            dataTypeCount++,
            -1,
            TypeTag::DERIVED,
            new DerivedEntry
            {
                node->children[0]->token->lexeme,
                node->token->type == TokenType::TK_UNION
            }
        };
    }
    else if (node->sym_index == 108 && processTypedef)
    {
        // Type alias

        const string& oldName = node->children[1]->token->lexeme;
        const string& newName = node->children[2]->token->lexeme;

        globalSymbolTable[oldName]->refCount++;
        globalSymbolTable[newName] = globalSymbolTable[oldName];
        prefixTable[newName] = node->token->type;
    }

    firstPass(node->sibling, processTypedef);
}

void secondPass(const ASTNode* node, map<string, TypeLog*>& symTable)
{
    static FuncEntry* local_func = nullptr;

    if (!node)
        return;

    if (node->sym_index == 57)
    {
        // <program> -> <funcList> <mainFunction>

        secondPass(node->children[0], symTable);
        secondPass(node->children[1], symTable);
    }
    else if (node->sym_index == 60 || node->sym_index == 58)
    {
        // <function> -> <inputList><outputList> <stmts>
        // Fill input argument 

        FuncEntry* entry = dynamic_cast<FuncEntry*>(globalSymbolTable[node->token->lexeme]->structure);

        for (auto arg = node->children[0]; arg; arg = arg->sibling)
        {
            entry->argTypes.push_back(
            {
                arg->token->lexeme,
                globalSymbolTable[arg->type->sibling ? arg->type->sibling->token->lexeme : arg->type->token->lexeme]
            });

            entry->argTypes.back().second->refCount++;
        }

        for (auto ret = node->children[1]; ret; ret = ret->sibling)
        {
            entry->retTypes.push_back(
                {
                    ret->token->lexeme,
                    globalSymbolTable[ret->type->sibling ? ret->type->sibling->token->lexeme : ret->type->token->lexeme]
                });

            entry->argTypes.back().second->refCount++;
        }

        local_func = entry;
        secondPass(node->children[0], local_func->symbolTable);
        secondPass(node->children[1], local_func->symbolTable);
        secondPass(node->children[2], local_func->symbolTable);
    }
    else if (node->sym_index == 68)
    {
        // <stmts> -> <definitions> <declarations> <funcBody> <return>

        secondPass(node->children[0], symTable);
        secondPass(node->children[1], symTable);
    }
    else if (node->sym_index == 71)
    {
        // <typeDefinition> -> TK_RUID <fieldDefinitions>

        TypeLog* mediator = globalSymbolTable[node->children[0]->token->lexeme];

        DerivedEntry* entry = dynamic_cast<DerivedEntry*>(mediator->structure);

        for (auto field = node->children[1]; field; field = field->sibling)
        {
            entry->fields.push_back(
                {
                    field->token->lexeme,
                    globalSymbolTable[field->type->token->lexeme]
                });

            entry->fields.back().second->refCount++;
            adj[entry->fields.back().second->index][mediator->index]++;
        }

        structList[mediator->index] = mediator;
    }
    else if ((node->sym_index == 63 || node->sym_index == 77))
    {
        // <declaration> ===> { token: TK_ID, type: <dataType> }
        // <dataType> ==> { TK_INT, TK_REAL, { TK_RECORD/TK_UNION, TK_RUID } }

        auto &table = node->isGlobal ? globalSymbolTable : symTable;

        table[node->token->lexeme] = new TypeLog
        {
            1,
            node->isGlobal ? identifierCount++ : local_func->identifierCount++,
            -1,
            TypeTag::VARIABLE,
            new VariableEntry(node->token->lexeme)
        };

        auto entry = dynamic_cast<VariableEntry*>(table[node->token->lexeme]->structure);
        entry->isGlobal = node->isGlobal;
        entry->type = globalSymbolTable[node->type->sibling == nullptr ? node->type->token->lexeme : node->type->sibling->token->lexeme];
    }

    secondPass(node->sibling, symTable);
}

void calculateWidth()
{
    auto width_cal_order = topological_sort();

    for (int i = 0; i < structList.size(); ++i)
    {
        int width = 0;
        int actualIndex = width_cal_order[i];

        DerivedEntry* entry = dynamic_cast<DerivedEntry*>(structList[actualIndex]->structure);
        if (!entry)
            continue;

        int isUnion = entry->isUnion;

        for (int i = 0; i < dataTypeCount; i++)
        {
            int size = adj[i][actualIndex] * structList[i]->width;

            width = isUnion ? max(width, size) : width + size;
        }

        structList[actualIndex]->width = width;
    }
}

void loadSymbolTable(const ASTNode* node)
{
	globalSymbolTable["int"] = new TypeLog
	{
		1,
		dataTypeCount++,
		2,
		TypeTag::INT,
		nullptr
	};

	globalSymbolTable["real"] = new TypeLog
	{
		1,
		dataTypeCount++,
		4,
		TypeTag::REAL,
		nullptr
	};

	globalSymbolTable["##bool"] = new TypeLog
	{
		1,
		dataTypeCount++,
		0,
		TypeTag::BOOL,
		nullptr
	};

	globalSymbolTable["##void"] = new TypeLog
	{
		1,
		dataTypeCount++,
		0,
		TypeTag::VOID,
		nullptr
	};

    firstPass(node);

    structList.resize(dataTypeCount);
    structList[0] = globalSymbolTable["int"];
    structList[1] = globalSymbolTable["real"];
    structList[2] = globalSymbolTable["##bool"];
    structList[3] = globalSymbolTable["##void"];

    adj.clear();
    adj.resize(dataTypeCount, vector<int>(dataTypeCount, 0));

    secondPass(node, globalSymbolTable);

    calculateWidth();
}