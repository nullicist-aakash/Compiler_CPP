#include <iostream>
#include <iomanip>
#include "TypeChecker.h"

using namespace std;

void printAST(ASTNode* node, int tab = 0)
{
	if (node == nullptr)
		return;

	for (int i = 0; i < tab; ++i)
		cerr << '\t';
	cerr << *node << endl;

	for (auto& child : node->children)
		printAST(child, tab + 1);

	printAST(node->sibling, tab);
}

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

	loadSymbolTable(astNode);

	typeChecker_init();
	assignTypes(astNode);
	printAST(astNode);
}