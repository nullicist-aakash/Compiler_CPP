#pragma once
#include "Lexer.h"
#include <vector>

extern const char* GrammarLoc;

struct TreeNode
{
	int symbol_index;
	int parent_child_index;
	int productionNumber;

	const Token* token;

	int isLeaf;
	TreeNode* parent;
	std::vector<TreeNode*> children;
};

void loadParser();