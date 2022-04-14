#pragma once
#include "Lexer.h"
#include <vector>

extern const char* GrammarLoc;

struct ParseTreeNode
{
	int symbol_index = 0;
	int parent_child_index = 0;
	int productionNumber = 0;

	Token* token = nullptr;

	int isLeaf = 0;
	ParseTreeNode* parent = nullptr;
	std::vector<ParseTreeNode*> children;
};

void loadParser();

const ParseTreeNode* parseInputSourceCode(Buffer&, bool&);