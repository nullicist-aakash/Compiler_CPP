#pragma once
#include "Parser.h"
#include <vector>

enum class NonTerminalType
{
	PROGRAM,
	FUNCTION,
	PARAMETER,
	STMTS,
	TYPE_DEFINITION,
	ID,
	FIELD_DEFINITION,
	VARIABLE_DEFINITION,
	ASSIGNMENT,
	FUNCTIONCALL,
	ITERATIVE,
	CONDITIONAL,
	READ,
	WRITE,
	OPERATOR,
	DEFINETYPE,

	GENERAL
};

struct ASTNode
{
	const NonTerminalType type;
	std::vector<ASTNode*> children;
	ASTNode* sibling;

	ASTNode(NonTerminalType type, ASTNode* sibling = nullptr) : type{ type }, sibling {sibling}
	{

	}
};

struct FuncNode : public ASTNode
{
	std::string Name;

	FuncNode(const std::string& token);
};

struct ParameterNode : public ASTNode
{
	std::string varName;
	ASTNode* varType;

	ParameterNode(const std::string&, ASTNode*);
};

struct IDNode : public ASTNode
{
	std::string varName;

	IDNode(const std::string&);
};

struct TypeDefinitionNode : public ASTNode
{
	bool isRecord;
	std::string name;

	TypeDefinitionNode(bool, const std::string&);
};

struct DefineTypeNode : public ASTNode
{
	bool isUnion;
	std::string from;
	std::string to;

	DefineTypeNode(bool isUnion, const std::string& from, const std::string& to) : ASTNode(NonTerminalType::DEFINETYPE)
	{
		this->isUnion = isUnion;
		this->from = from;
		this->to = to;
	}
};

struct FieldDefinitionNode : public ASTNode
{
	std::string varName;
	ASTNode* varType;

	FieldDefinitionNode(const std::string&, ASTNode*);
};

struct VariableDefinitionNode : public ASTNode
{
	bool isGlobal;
	std::string varName;
	ASTNode* varType;

	VariableDefinitionNode(bool, const std::string&, ASTNode*);
};

struct AssignmentNode : public ASTNode
{
	ASTNode* target;

	AssignmentNode(ASTNode*);
};

struct FunctionCallNode : public ASTNode
{
	std::string name;

	FunctionCallNode(const std::string &name) : ASTNode(NonTerminalType::FUNCTIONCALL)
	{
		this->name = name;
	}
};

struct ReadNode : public ASTNode
{
	ASTNode* target;

	ReadNode(ASTNode* target) :ASTNode(NonTerminalType::READ)
	{
		this->target = target;
	}
};

struct WriteNode : public ASTNode
{
	ASTNode* target;

	WriteNode(ASTNode* target) :ASTNode(NonTerminalType::WRITE)
	{
		this->target = target;
	}
};

struct OperatorNode : ASTNode
{
	TokenType op;

	OperatorNode(TokenType op) : ASTNode(NonTerminalType::OPERATOR)
	{
		this->op = op;
	}
};

ASTNode* createAST(const ParseTreeNode* input, const ParseTreeNode* = nullptr, ASTNode* = nullptr);