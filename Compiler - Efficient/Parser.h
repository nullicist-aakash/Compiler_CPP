#pragma once
#include "Lexer.h"
#include <vector>

extern const char* GrammarLoc;

struct ParseTreeNode
{
	int symbol_index;
	int parent_child_index;
	int productionNumber;

	const Token* token;

	int isLeaf;
	const ParseTreeNode* parent;
	std::vector<const ParseTreeNode*> children;
};

void loadParser();