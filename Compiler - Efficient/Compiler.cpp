#include <iostream>
#include "Lexer.h"
#include "Parser.h"

using namespace std;

int main()
{
	loadDFA();
	loadParser();

	Buffer buffer("testcase1.txt");
	parseInputSourceCode(buffer);
}