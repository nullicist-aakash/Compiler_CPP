#include <iostream>
#include "Lexer.h"
#include "Parser.h"

using namespace std;

int main()
{
	loadDFA();
	loadParser();

	Buffer buffer("testcase1.txt");
	const Token* t;
	while ((t = getNextToken(buffer)) != nullptr)
	{
		cout << *t << endl;
		delete t;
	}
}