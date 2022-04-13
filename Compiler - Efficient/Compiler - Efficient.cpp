#include <iostream>
#include "Lexer.h"

int main()
{
	Lexer l("testcase1.txt");

	const Token* t;
	while ((t = l.getNextToken()) != nullptr)
	{
		cout << *t << endl;
		delete t;
	}
}