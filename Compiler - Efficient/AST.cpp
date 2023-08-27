#include "AST.h"
#include <iostream>
#include <cassert>
using namespace std;

Token* copy_token(const Token* old_token)
{
	Token* token = new Token;
	token->length = old_token->length;
	token->lexeme = old_token->lexeme;
	token->line_number = old_token->line_number;
	token->start_index = old_token->start_index;
	token->type = old_token->type;
	return token;
}

FuncNode::FuncNode(const string& token) : ASTNode(NonTerminalType::FUNCTION)
{
	this->Name = token;
}

ParameterNode::ParameterNode(const string& var, ASTNode* type) : ASTNode(NonTerminalType::PARAMETER)
{
	this->varName = var;
	this->varType = type;
}

IDNode::IDNode(const string& var) : ASTNode(NonTerminalType::ID)
{
	this->varName = var;
}

TypeDefinitionNode::TypeDefinitionNode(bool isRecord, const string& name) : ASTNode(NonTerminalType::TYPE_DEFINITION)
{
	this->isRecord = isRecord;
	this->name = name;
}

FieldDefinitionNode::FieldDefinitionNode(const string& var, ASTNode* type) : ASTNode(NonTerminalType::FIELD_DEFINITION)
{
	this->varName = var;
	this->varType = type;
}

VariableDefinitionNode::VariableDefinitionNode(bool isGlobal, const string& var, ASTNode* type) : ASTNode(NonTerminalType::VARIABLE_DEFINITION)
{
	this->isGlobal = isGlobal;
	this->varName = var;
	this->varType = type;
}

AssignmentNode::AssignmentNode(ASTNode* target) : ASTNode(NonTerminalType::ASSIGNMENT)
{
	this->target = target;
}

ASTNode* createAST(const ParseTreeNode* input, const ParseTreeNode* parent, ASTNode* inherited)
{
	assert(input != nullptr);

	if (input->isLeaf)
	{
		assert(input != nullptr && input->isLeaf);

		cout << "Not implemented yet!!" << endl;
		return nullptr;
	}

	ASTNode* node = nullptr;

	if (input->productionNumber == 0)
	{
		// <program> ===> <otherFunctions> <mainFunction> TK_EOF

		node = new ASTNode
		{
			NonTerminalType::PROGRAM
		};

		node->children.resize(2);
		node->children[0] = createAST(input->children[0], input);
		node->children[1] = createAST(input->children[1], input);
	}
	else if (input->productionNumber == 1)
	{
		// <mainFunction> ===> TK_MAIN <stmts> TK_END

		node = new FuncNode
		{
			input->children[0]->token->lexeme
		};

		node->children.resize(3, nullptr);
		node->children[2] = createAST(input->children[1], input);
	}
	else if (input->productionNumber == 2)
	{
		// <otherFunctions> ===> <function> <otherFunctions[1]>

		ASTNode* func = createAST(input->children[0], input);
		func->sibling = createAST(input->children[1], input);
		return func;
	}
	else if (input->productionNumber == 3)
	{
		// <otherFunctions> ===> eps

		return nullptr;
	}
	else if (input->productionNumber == 4)
	{
		// <function> ===> TK_FUNID <input_par> <output_par> TK_SEM <stmts> TK_END

		node = new FuncNode
		{
			input->children[0]->token->lexeme
		};

		node->children.resize(3);
		node->children[0] = createAST(input->children[1], input);
		node->children[1] = createAST(input->children[2], input);
		node->children[2] = createAST(input->children[4], input);
	}
	else if (input->productionNumber == 5)
	{
		// <input_par> ===> TK_INPUT TK_PARAMETER TK_LIST TK_SQL <parameter_list> TK_SQR

		return createAST(input->children[4], input);
	}
	else if (input->productionNumber == 6)
	{
		// <output_par> ===> TK_OUTPUT TK_PARAMETER TK_LIST TK_SQL <parameter_list> TK_SQR

		return createAST(input->children[4], input);
	}
	else if (input->productionNumber == 7)
	{
		// <output_par> ===> eps

		return nullptr;
	}
	else if (input->productionNumber == 8)
	{
		// <parameter_list> ===> <dataType> TK_ID <remaining_list>

		node = new ParameterNode
		{
			input->children[1]->token->lexeme,
			createAST(input->children[0], input)
		};
		
		node->sibling = createAST(input->children[2], input);
	}
	else if (input->productionNumber == 9)
	{
		// <dataType> ===> <primitiveDatatype>

		return createAST(input->children[0], input);
	}
	else if (input->productionNumber == 10)
	{
		// <dataType> ===> <constructedDatatype>

		return createAST(input->children[0], input);
	}
	else if (input->productionNumber == 11)
	{
		// <primitiveDatatype> ===> TK_INT

		node = new ASTNode
		{
			NonTerminalType::GENERAL
		};

		return createAST(input->children[0], input);
	}
	else if (input->productionNumber == 12)
	{
		// <primitiveDatatype> ===> TK_REAL

		return createAST(input->children[0], input);
	}
	else if (input->productionNumber == 13)
	{
		// <constructedDatatype> ===> TK_RECORD TK_RUID

		ASTNode* temp = createAST(input->children[0], input);
		temp->sibling = createAST(input->children[1], input);
		return temp;
	}
	else if (input->productionNumber == 14)
	{
		// <constructedDatatype> ===> TK_UNION TK_RUID

		ASTNode* temp = createAST(input->children[0], input);
		temp->sibling = createAST(input->children[1], input);
		return temp;
	}
	else if (input->productionNumber == 15)
	{
		// <constructedDatatype> ===> TK_RUID

		return createAST(input->children[0], input);
	}
	else if (input->productionNumber == 16)
	{
		// <remaining_list> ===> TK_COMMA <parameter_list>

		return createAST(input->children[1], input);
	}
	else if (input->productionNumber == 17)
	{
		// <remaining_list> ===> eps

		return nullptr;
	}
	else if (input->productionNumber == 18)
	{
		// <stmts> ===> <typeDefinitions> <declarations> <otherStmts> <returnStmt>

		node = new ASTNode
		{
			NonTerminalType::STMTS
		};

		node->children.resize(4);
		node->children[0] = createAST(input->children[0], input);
		node->children[1] = createAST(input->children[1], input);
		node->children[2] = createAST(input->children[2], input);
		node->children[3] = createAST(input->children[3], input);
	}
	else if (input->productionNumber == 19)
	{
		// <typeDefinitions> ===> <actualOrRedefined> <typeDefinitions>1

		ASTNode* temp = createAST(input->children[0], input);
		temp->sibling = createAST(input->children[1], input);
		return temp;
	}
	else if (input->productionNumber == 20)
	{
		// <typeDefinitions> ===> eps

		return nullptr;
	}
	else if (input->productionNumber == 21)
	{
		// <actualOrRedefined> ===> <typeDefinition>
		
		return createAST(input->children[0], input);
	}
	else if (input->productionNumber == 22)
	{
		// <actualOrRedefined> ===> <definetypestmt>
		
		return createAST(input->children[0], input);
	}
	else if (input->productionNumber == 23)
	{
		// <typeDefinition> ===> TK_RECORD TK_RUID <fieldDefinitions> TK_ENDRECORD

		node = new TypeDefinitionNode
		{
			true,
			input->children[1]->token->lexeme
		};

		node->children.resize(1);
		node->children[0] = createAST(input->children[2], input);
	}
	else if (input->productionNumber == 24)
	{
		// <typeDefinition> ===> TK_UNION TK_RUID <fieldDefinitions> TK_ENDUNION
		
		node = new TypeDefinitionNode
		{
			false,
			input->children[1]->token->lexeme
		};

		node->children.resize(1);
		node->children[0] = createAST(input->children[2], input);
	}
	else if (input->productionNumber == 25)
	{
		// <fieldDefinitions> ===> <fieldDefinition>1 <fieldDefinition>2 <moreFields>
	
		ASTNode* first = createAST(input->children[0], input);
		first->sibling = createAST(input->children[1], input);
		first->sibling->sibling = createAST(input->children[2], input);
		return first;
	}
	else if (input->productionNumber == 26)
	{
		// <fieldDefinition> ===> TK_TYPE <fieldType> TK_COLON TK_FIELDID TK_SEM
		
		node = new FieldDefinitionNode
		{
			input->children[3]->token->lexeme,
			createAST(input->children[1], input)
		};
	}
	else if (input->productionNumber == 27)
	{
		// <fieldType> ===> <primitiveDatatype>
		
		return createAST(input->children[0], input);
	}
	else if (input->productionNumber == 28)
	{
		// <fieldType> ===> TK_RUID
		
		return createAST(input->children[0], input);
	}
	else if (input->productionNumber == 29)
	{
		// <moreFields> ===> <fieldDefinition> <moreFields>

		ASTNode* first = createAST(input->children[0], input);
		first->sibling = createAST(input->children[1], input);
		return first;
	}
	else if (input->productionNumber == 30)
	{
		// <moreFields> ===> eps
		
		return nullptr;
	}
	else if (input->productionNumber == 31)
	{
		// <declarations> ===> <declaration> <declarations>1
		
		ASTNode* first = createAST(input->children[0], input);
		first->sibling = createAST(input->children[1], input);
		return first;
	}
	else if (input->productionNumber == 32)
	{
		// <declarations> ===> eps
		
		return nullptr;
	}
	else if (input->productionNumber == 33)
	{
		// <declaration> ===> TK_TYPE <dataType> TK_COLON TK_ID <global_or_not> TK_SEM

		node = new VariableDefinitionNode
		{
			input->children[4]->productionNumber == 34,
			input->children[3]->token->lexeme,
			createAST(input->children[1], input)
		};
	}
	else if (input->productionNumber == 34)
	{
		// <global_or_not> ===> TK_COLON TK_GLOBAL

		return nullptr;
	}
	else if (input->productionNumber == 35)
	{
		// <global_or_not> ===> eps

		return nullptr;
	}
	else if (input->productionNumber == 36)
	{
		// <otherStmts> ===> <stmt> <otherStmts>1
		
		ASTNode* first = createAST(input->children[0], input);
		first->sibling = createAST(input->children[1], input);
		return first;
	}
	else if (input->productionNumber == 37)
	{
		// <otherStmts> ===> eps
		
		return nullptr;
	}
	else if (input->productionNumber == 38)
	{
		// <stmt> ===> <assignmentStmt>
		
		return createAST(input->children[0], input);
	}
	else if (input->productionNumber == 39)
	{
		// <stmt> ===> <iterativeStmt>
		
		return createAST(input->children[0], input);
	}
	else if (input->productionNumber == 40)
	{
		// <stmt> ===> <conditionalStmt>
		
		return createAST(input->children[0], input);
	}
	else if (input->productionNumber == 41)
	{
		// <stmt> ===> <ioStmt>
		
		return createAST(input->children[0], input);
	}
	else if (input->productionNumber == 42)
	{
		// <stmt> ===> <funCallStmt>
		
		return createAST(input->children[0], input);
	}
	else if (input->productionNumber == 43)
	{
		// <assignmentStmt> ===> <singleOrRecId> TK_ASSIGNOP <arithmeticExpression> TK_SEM

		node = new AssignmentNode
		{
			createAST(input->children[0], input)
		};

		node->children.resize(1);
		node->children[0] = createAST(input->children[2], input);
	}
	else if (input->productionNumber == 44)
	{
		// <oneExpansion> ===> TK_DOT TK_FIELDID

		node = new OperatorNode
		{
			TokenType::TK_DOT
		};

		node->children.resize(2);
		node->children[0] = createAST(input->children[0], input);
		node->children[1] = createAST(input->children[1], input);
	}
	else if (input->productionNumber == 45)
	{
		// <moreExpansions> ===> <oneExpansion> <moreExpansions>1

		ASTNode* oneExp = createAST(input->children[0], input);
		assert(oneExp->children.size() == 2);
		ASTNode* dot = oneExp->children[0];
		ASTNode* id = oneExp->children[1];
		delete oneExp;

		dot->children.resize(2);
		dot->children[0] = inherited;
		dot->children[1] = id;

		return createAST(input->children[1], input, dot);
	}
	else if (input->productionNumber == 46)
	{
		// <moreExpansions> ===> eps

		return inherited;
	}
	else if (input->productionNumber == 47)
	{
		// <singleOrRecId> ===> TK_ID <option_single_constructed>

		ASTNode* idNode = createAST(input->children[0], input);
		ASTNode* constructed = createAST(input->children[1], input, idNode);
		return constructed;
	}
	else if (input->productionNumber == 48)
	{
		// <option_single_constructed> ===> eps

		return inherited;
	}
	else if (input->productionNumber == 49)
	{
		// <option_single_constructed> ===> <oneExpansion> <moreExpansions>

		ASTNode* oneExp = createAST(input->children[0], input);
		assert(oneExp->children.size() == 2);
		ASTNode* dot = oneExp->children[0];
		ASTNode* id = oneExp->children[1];
		delete oneExp;

		dot->children.resize(2);
		dot->children[0] = inherited;
		dot->children[1] = id;

		return createAST(input->children[1], input, dot);
	}
	else if (input->productionNumber == 50)
	{
		// <funCallStmt> ===> <outputParameters> TK_CALL TK_FUNID TK_WITH TK_PARAMETERS <inputParameters> TK_SEM

		node = new FunctionCallNode
		{
			input->children[2]->token->lexeme
		};

		node->children.resize(2);
		node->children[0] = createAST(input->children[0], input);
		node->children[1] = createAST(input->children[5], input);
	}
	else if (input->productionNumber == 51)
	{
		// <outputParameters> ===> TK_SQL <idList> TK_SQR TK_ASSIGNOP
		
		return createAST(input->children[1], input);
	}
	else if (input->productionNumber == 52)
	{
		// <outputParameters> ===> eps
		
		return nullptr;
	}
	else if (input->productionNumber == 53)
	{
		// <inputParameters> ===> TK_SQL <idList> TK_SQR
		
		return createAST(input->children[1], input);
	}
	else if (input->productionNumber == 54)
	{
		// <iterativeStmt> ===> TK_WHILE TK_OP <booleanExpression> TK_CL <stmt> <otherStmts> TK_ENDWHILE

		node = new ASTNode
		{
			NonTerminalType::ITERATIVE
		};

		node->children.resize(2);
		node->children[0] = createAST(input->children[2], input);
		node->children[1] = createAST(input->children[4], input);
		node->children[1]->sibling = createAST(input->children[5], input);
	}
	else if (input->productionNumber == 55)
	{
		// <conditionalStmt> ===> TK_IF TK_OP<booleanExpression> TK_CL TK_THEN<stmt><otherStmts><elsePart>
		
		node = new ASTNode
		{
			NonTerminalType::CONDITIONAL
		};

		node->children.resize(3);
		node->children[0] = createAST(input->children[2], input);
		node->children[1] = createAST(input->children[5], input);
		node->children[2] = createAST(input->children[7], input);
		node->children[1]->sibling = createAST(input->children[6], input);
	}
	else if (input->productionNumber == 56)
	{
		// <elsePart> ===> TK_ELSE <stmt> <otherStmts> TK_ENDIF

		ASTNode* first = createAST(input->children[1], input);
		first->sibling = createAST(input->children[2], input);
		return first;
	}
	else if (input->productionNumber == 57)
	{
		// <elsePart> ===> TK_ENDIF
		
		return nullptr;
	}
	else if (input->productionNumber == 58)
	{
		// <ioStmt> ===> TK_READ TK_OP <var> TK_CL TK_SEM

		node = new ReadNode
		{
			createAST(input->children[2], input)
		};
	}
	else if (input->productionNumber == 59)
	{
		// <ioStmt> == = > TK_WRITE TK_OP<var> TK_CL TK_SEM

		node = new WriteNode
		{
			createAST(input->children[2], input)
		};
	}
	else if (input->productionNumber == 60)
	{
		// <arithmeticExpression> ===> <term> <expPrime>
		// <arithmeticExpression>.treenode = <expPrime>.syn
		// <expPrime>.inh = <term>.treenode

		ASTNode* termNode = createAST(input->children[0], input);
		ASTNode* expPrime = createAST(input->children[1], input, termNode);
		return expPrime;
	}
	else if (input->productionNumber == 61)
	{
		// <expPrime> ===> <lowPrecedenceOperators> <term> <expPrime[1]>
		// <expPrime[1]>.inh = createTreeNode(<lowPrecedenceOperators>.data, <expPrime>.inh, <term>.treenode)
		// <expPrime>.syn = <expPrime[1]>.syn

		ASTNode* op = createAST(input->children[0], input);
		ASTNode* term = createAST(input->children[1], input);

		op->children.resize(2);
		op->children[0] = inherited;
		op->children[1] = term;

		return createAST(input->children[2], input, op);
	}
	else if (input->productionNumber == 62)
	{
		// <expPrime> ===> eps
		// <expPrime>.syn = <expPrime>.inh

		return inherited;
	}
	else if (input->productionNumber == 63)
	{
		// <term> ===> <factor> <termPrime>
		// <term>.treenode = <termPrime>.syn
		// <termPrime>.inh = <factor>.treenode

		ASTNode* factorNode = createAST(input->children[0], input);
		ASTNode* termPrime = createAST(input->children[1], input, factorNode);
		return termPrime;
	}
	else if (input->productionNumber == 64)
	{
		// <termPrime> ===> <highPrecedenceOperators> <factor> <termPrime[1]>
		// <termPrime[1]>.treenode = createTreeNode(<highPrecedenceOperators>.data, <termPrime>.inh, <factor>.treenode)
		// <termPrime>.syn = <termPrime[1]>.syn

		ASTNode* op = createAST(input->children[0], input);
		ASTNode* term = createAST(input->children[1], input);

		op->children.resize(2);
		op->children[0] = inherited;
		op->children[1] = term;

		return createAST(input->children[2], input, op);
	}
	else if (input->productionNumber == 65)
	{
		// <termPrime> ===> eps

		return inherited;
	}
	else if (input->productionNumber == 66)
	{
		// <factor> ===> TK_OP <arithmeticExpression> TK_CL

		return createAST(input->children[1], input);
	}
	else if (input->productionNumber == 67)
	{
		// <factor> ===> <var>

		return createAST(input->children[0], input);
	}
	else if (input->productionNumber == 68)
	{
		// <highPrecedenceOperators> ===> TK_MUL

		return createAST(input->children[0], input);
	}
	else if (input->productionNumber == 69)
	{
		// <highPrecedenceOperators> ===> TK_DIV
		
		return createAST(input->children[0], input);
	}
	else if (input->productionNumber == 70)
	{
		// <lowPrecedenceOperators> ===> TK_PLUS

		return createAST(input->children[0], input);
	}
	else if (input->productionNumber == 71)
	{
		// <lowPrecedenceOperators> ===> TK_MINUS

		return createAST(input->children[0], input);
	}
	else if (input->productionNumber == 72)
	{
		// <booleanExpression> ===> TK_OP <booleanExpression>1 TK_CL <logicalOp> TK_OP <booleanExpression>2 TK_CL

		ASTNode* op = createAST(input->children[3], input);
		op->children.resize(2);
		op->children[0] = createAST(input->children[1], input);
		op->children[1] = createAST(input->children[5], input);
		return op;
	}
	else if (input->productionNumber == 73)
	{
		// <booleanExpression> ===> <var>1 <relationalOp> <var>2

		ASTNode* op = createAST(input->children[1], input);
		op->children.resize(2);
		op->children[0] = createAST(input->children[0], input);
		op->children[1] = createAST(input->children[2], input);
		return op;
	}
	else if (input->productionNumber == 74)
	{
		// <booleanExpression> ===> TK_NOT TK_OP <booleanExpression> TK_CL

		ASTNode* op = createAST(input->children[0], input);
		op->children.resize(1);
		op->children[0] = createAST(input->children[2], input);
		return op;
	}
	else if (input->productionNumber == 75)
	{
		// <var> ===> <singleOrRecId>

		return createAST(input->children[0], input);
	}
	else if (input->productionNumber == 76)
	{
		// <var> ===> TK_NUM

		return createAST(input->children[0], input);
	}
	else if (input->productionNumber == 77)
	{
		// <var> ===> TK_RNUM

		return createAST(input->children[0], input);
	}
	else if (input->productionNumber == 78)
	{
		// <logicalOp> ===> TK_AND

		return createAST(input->children[0], input);
	}
	else if (input->productionNumber == 79)
	{
		//  <logicalOp> ===> TK_OR

		return createAST(input->children[0], input);
	}
	else if (input->productionNumber == 80)
	{
		// <relationalOp> ===> TK_LT

		return createAST(input->children[0], input);
	}
	else if (input->productionNumber == 81)
	{
		// <relationalOp> ===> TK_LE
		
		return createAST(input->children[0], input);
	}
	else if (input->productionNumber == 82)
	{
		// <relationalOp> ===> TK_EQ
		
		return createAST(input->children[0], input);
	}
	else if (input->productionNumber == 83)
	{
		// <relationalOp> ===> TK_GT
		
		return createAST(input->children[0], input);
	}
	else if (input->productionNumber == 84)
	{
		// <relationalOp> ===> TK_GE
		
		return createAST(input->children[0], input);
	}
	else if (input->productionNumber == 85)
	{
		// <relationalOp> ===> TK_NE
		
		return createAST(input->children[0], input);
	}
	else if (input->productionNumber == 86)
	{
		// <returnStmt> ===> TK_RETURN <optionalReturn> TK_SEM

		return createAST(input->children[1], input);
	}
	else if (input->productionNumber == 87)
	{
		// <optionalReturn> ===> TK_SQL <idList> TK_SQR

		return createAST(input->children[1], input);
	}
	else if (input->productionNumber == 88)
	{
		// <optionalReturn> ===> eps

		return nullptr;
	}
	else if (input->productionNumber == 89)
	{
		// <idList> ===> TK_ID <more_ids>

		node = new IDNode
		{
			input->children[0]->token->lexeme
		};

		node->sibling = createAST(input->children[1], input);
	}
	else if (input->productionNumber == 90)
	{
		// <more_ids> ===> TK_COMMA <idList>
		
		return createAST(input->children[1], input);
	}
	else if (input->productionNumber == 91)
	{
		// <more_ids> ===> eps
		
		return nullptr;
	}
	else if (input->productionNumber == 92)
	{
		// <definetypestmt> ===> TK_DEFINETYPE <A> TK_RUID1 TK_AS TK_RUID2

		node = new DefineTypeNode
		{
			input->children[1]->productionNumber == 94,
			input->children[2]->token->lexeme,
			input->children[4]->token->lexeme
		};
	}
	else if (input->productionNumber == 93)
	{
		// <A> ===> TK_RECORD

		return nullptr;
	}
	else if (input->productionNumber == 94)
	{
		// <A> ===> TK_UNION

		return nullptr;
	}

	return node;
}