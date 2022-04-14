#include <iostream>
#include "Lexer.h"
#include "Parser.h"

using namespace std;

int main()
{
	std::ofstream out("outfile.txt");
	std::cerr.rdbuf(out.rdbuf());

	loadDFA();
	loadParser();

	bool b;

	Buffer buffer("testcase6.txt");
	parseInputSourceCode(buffer, b);
}