#include "TypeChecker.h"
#include <iostream>

using namespace std;

TypeLog* real, * integer, * boolean, * void_empty;
bool isTypeError;
map<string, TypeLog*> *localSymbolTable;

int areCompatible(ASTNode* leftNode, ASTNode* rightNode)
{
    TypeLog* left = leftNode->derived_type;
    TypeLog* right = rightNode->derived_type;
    while (leftNode && rightNode)
    {
        left = leftNode->derived_type;
        right = rightNode->derived_type;

        if (left != right || left == boolean || left == void_empty || !left || !right)
            return 0;

        leftNode = leftNode->sibling;
        rightNode = rightNode->sibling;
    }
    return !leftNode && !rightNode;
}

TypeLog* finalType(ASTNode* leftNode, ASTNode* rightNode, Token* opToken)
{
    TypeLog* left = leftNode->derived_type;
    TypeLog* right = rightNode ? rightNode->derived_type : nullptr;
    TokenType op = opToken->type;

    if (op == TokenType::TK_ASSIGNOP)
    {
        if (areCompatible(leftNode, rightNode))
            return void_empty;

        isTypeError = true;
        cerr << "Assignment with incompatible types at line no. " << opToken->line_number << endl;
        return nullptr;
    }

    if (op == TokenType::TK_PLUS || op == TokenType::TK_MINUS)
    {
        if (left == right && left != boolean && left != void_empty && left && right)
            return right;

        isTypeError = true;
        cerr << "Operation " << leftNode->token->lexeme << " " << opToken->lexeme << " " << rightNode->token->lexeme << " with incompatible types at line no. " << opToken->line_number << endl;
        return nullptr;
    }

    if (op == TokenType::TK_MUL)
    {
        if (left == right && (left == real || left == integer) && left != boolean && left != void_empty)
            return left;

        // TODO

        return nullptr;
    }

    if (op == TokenType::TK_DIV)
    {
        int first_type = left == real ? 0x01 : left == integer ? 0x02 : 0x04;
        int second_type = right == real ? 0x01 : right == integer ? 0x02 : 0x04;

        if ((first_type & 0x03) && (second_type & 0x03) && left != boolean && left != void_empty)
            return real;

        return nullptr;
    }

    if (op == TokenType::TK_AND || op == TokenType::TK_OR)
    {
        if (left == boolean && right == boolean)
            return boolean;

        isTypeError = true;
        cerr << "Operation " << leftNode->token->lexeme << " " << opToken->lexeme << " " << rightNode->token->lexeme << " with incompatible types at line no. " << opToken->line_number << endl;
        return nullptr;
    }

    if (op == TokenType::TK_EQ || op == TokenType::TK_NE || op == TokenType::TK_GE || op == TokenType::TK_LE || op == TokenType::TK_LT || op == TokenType::TK_GT)
    {
        if (left == real && right == real)
            return boolean;

        if (left == integer && right == integer)
            return boolean;

        isTypeError = true;
        cerr << "Operation " << leftNode->token->lexeme << " " << opToken->lexeme << " " << rightNode->token->lexeme << " with incompatible types at line no. " << opToken->line_number << endl;
        return nullptr;
    }

    if (op == TokenType::TK_NOT)
    {
        if (left == boolean)
            return boolean;

        isTypeError = true;
        cerr << "Operation " << leftNode->token->lexeme << " " << opToken->lexeme << " with incompatible types at line no. " << opToken->line_number << endl;
        return nullptr;
    }

    assert(false);
}

void typeChecker_init()
{
    real = globalSymbolTable["real"];
    integer = globalSymbolTable["int"];
    boolean = globalSymbolTable["##bool"];
    void_empty = globalSymbolTable["##void"];
    localSymbolTable = &globalSymbolTable;
}

void assignTypes(ASTNode* node)
{
    if (!node)
        return;

    if (node->sym_index == 57)
    {
        // program -> functions, main

        assignTypes(node->children[0]);
        assignTypes(node->children[1]);
    }
    else if (node->sym_index == 58 || node->sym_index == 60)
    {
        // function/main-function

        localSymbolTable = &(dynamic_cast<FuncEntry*>(globalSymbolTable[node->token->lexeme]->structure))->symbolTable;

        assignTypes(node->children[0]);
        assignTypes(node->children[1]);
        assignTypes(node->children[2]);
    }
    else if (node->sym_index == 68)
    {
        // stmts -> .. .. stmt ..

        assignTypes(node->children[1]);
        assignTypes(node->children[2]);
    }
    else if (node->sym_index == 81)
    {
        // assignment --> <identifier> = <expression>

        assignTypes(node->children[0]);
        assignTypes(node->children[1]);
        node->derived_type = finalType(node->children[0], node->children[1], node->token);
    }
    else if (node->sym_index == 86)
    {
        // function call statement

        assignTypes(node->children[0]);
        assignTypes(node->children[1]);

        node->derived_type = node->children[0]->derived_type;
    }
    else if (node->sym_index == 89)
    {
        // iterative statement, while
        assignTypes(node->children[0]);
        assignTypes(node->children[1]);

        node->derived_type = void_empty;
    }
    else if (node->sym_index == 90)
    {
        // if-else
        assignTypes(node->children[0]);
        assignTypes(node->children[1]);
        assignTypes(node->children[2]);

        node->derived_type = void_empty;
    }
    else if (node->sym_index == 92)
    {
        // io
        assignTypes(node->children[0]);

        node->derived_type = void_empty;
    }
    else if (node->sym_index == 63 || node->sym_index == 77)
    {
        // idList

        for (auto temp = node; temp; temp = temp->sibling)
        {
            TypeLog* mediator = (*localSymbolTable)[node->token->lexeme];

            if (mediator == nullptr)
                mediator = globalSymbolTable[node->token->lexeme];

            VariableEntry* entry = dynamic_cast<VariableEntry*>(mediator->structure);
            temp->derived_type = entry->type;
        }
    }
    else if (node->sym_index == 108)
    {
        // typedef

        node->derived_type = globalSymbolTable[node->children[0]->token->lexeme];
        node->children[1]->derived_type = node->children[2]->derived_type = node->derived_type;
    }
    else if (
        node->token->type == TokenType::TK_PLUS ||
        node->token->type == TokenType::TK_MINUS ||
        node->token->type == TokenType::TK_MUL ||
        node->token->type == TokenType::TK_DIV ||
        node->token->type == TokenType::TK_AND ||
        node->token->type == TokenType::TK_OR ||
        node->token->type == TokenType::TK_EQ ||
        node->token->type == TokenType::TK_NE ||
        node->token->type == TokenType::TK_GE ||
        node->token->type == TokenType::TK_GT ||
        node->token->type == TokenType::TK_LE ||
        node->token->type == TokenType::TK_LT)
    {
        assignTypes(node->children[0]);
        assignTypes(node->children[1]);

        node->derived_type = finalType(node->children[0], node->children[1], node->token);
    }
    else if (node->token->type == TokenType::TK_NOT)
    {
        assignTypes(node->children[0]);

        node->derived_type = finalType(node->children[0], nullptr, node->token);
    }
    else if (node->token->type == TokenType::TK_ID)
    {
        TypeLog* entry = (*localSymbolTable)[node->token->lexeme];

        if (entry == nullptr)
            entry = globalSymbolTable[node->token->lexeme];

        node->derived_type = (dynamic_cast<VariableEntry*>(entry->structure))->type;
    }
    else if (node->token->type == TokenType::TK_DOT)
    {
        // <dot> ===> <left> TK_DOT <right>
        assignTypes(node->children[0]);

        DerivedEntry* leftEntry = dynamic_cast<DerivedEntry*>(node->children[0]->derived_type->structure);

        // search for token on right of DOT
        for (auto& x : leftEntry->fields)
            if (x.first == node->children[1]->token->lexeme)
                node->children[1]->derived_type = x.second;

        assert(node->children[1]->derived_type != nullptr);

        node->derived_type = node->children[1]->derived_type;
    }
    else if (node->token->type == TokenType::TK_NUM)
        node->derived_type = globalSymbolTable["int"];
    else if (node->token->type == TokenType::TK_RNUM)
        node->derived_type = globalSymbolTable["real"];

    assignTypes(node->sibling);
}