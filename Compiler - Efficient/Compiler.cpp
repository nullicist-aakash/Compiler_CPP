#include <iostream>
#include <iomanip>
#include "AST.h"

using namespace std;

void printParseTree(const ParseTreeNode& node)
{
	cout << node << endl;
	for (auto& x : node.children)
		printParseTree(*x);
}

void cleanParseTree(ParseTreeNode* node)
{
	if (node->token)
		delete node->token;

	for (auto& x : node->children)
		cleanParseTree(x);

	delete node;
}

int main()
{
	std::ofstream out("outfile.txt");
	std::cerr.rdbuf(out.rdbuf());

	loadDFA();
	loadParser();

	bool b;

	Buffer buffer("testcase5.txt");
	auto parseNode = parseInputSourceCode(buffer, b);
	
	auto astNode = createAST(parseNode);

	cleanParseTree(parseNode);

	/*
	printAST(astNode);*/
}