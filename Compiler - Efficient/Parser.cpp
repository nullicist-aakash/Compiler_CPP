#include "Parser.h"
#include <vector>
#include <iostream>
#include <string>
#include <bitset>
#include <map>

const char* GrammarLoc = "grammar.txt";
using namespace std;

class Parser
{
public:
	int num_non_terminals;
	int num_terminals;
	int start_index;

	vector<vector<int>> productions;
	vector<string> symbolType2symbolStr;
	map<string, int> symbolStr2symbolType;
	bitset<128> nullable;
	vector<bitset<128>> firstSet;
	vector<bitset<128>> followSet;
	vector<vector<int>> parseTable;

	Parser() : num_non_terminals{ 0 }, num_terminals{ 0 }, start_index{ 0 }
	{

	}

	void computeNullables()
	{
		nullable.reset();
		nullable.set(0);

		// Get BIT representation of productions
		vector<bitset<128>> productionBitset(productions.size());

		for (int i = 0; i < productions.size(); ++i)
			for (int j = 1; j < productions[i].size(); ++j)
				productionBitset[i].set(productions[i][j]);

		// Base case
		for (int i = 0; i < productions.size(); ++i)
			if (productions[i].size() == 2 && productions[i][1] == 0)
				nullable.set(productions[i][0]);

		// iterate untill no update
		bool isUpdated = true;
		while (isUpdated)
		{
			isUpdated = false;

			for (int i = 0; i < productions.size(); ++i)
			{
				// rhs is nullable and is not already captured
				if ((productionBitset[i] & nullable) == productionBitset[i] &&
					!nullable.test(productions[i][0]))
				{
					nullable.set(productions[i][0]);
					isUpdated = true;
				}
			}
		}

		cout << "Nullables: " << endl;
		for (int i = 0; i < 128; ++i)
			if (nullable.test(i))
				cout << "\t" << symbolType2symbolStr[i] << endl;
	}

	void computeFirstSets()
	{
		firstSet.clear();
		firstSet.resize(symbolType2symbolStr.size());

		// Base - Add eps to first set
		for (int i = 0; i < firstSet.size(); ++i)
			if (nullable.test(i))
				firstSet[i].set(0);

		// Base - First of terminals
		for (int i = 0; i < num_terminals; ++i)
			firstSet[i].set(i);

		// Iterate untill no update
		bool isUpdated = true;
		while (isUpdated)
		{
			isUpdated = false;

			for (const auto &production: productions)
			{
				bitset<128> bits = firstSet[production[0]];

				for (int j = 1; j < production.size(); ++j)
				{
					bits |= firstSet[production[j]];

					if (!nullable.test(production[j]))
						break;
				}

				if ((bits ^ firstSet[production[0]]).any())
				{
					firstSet[production[0]] |= bits;
					isUpdated = true;
				}
			}
		}

		cout << "First sets: " << endl;
		for (int i = 0; i < firstSet.size(); ++i)
		{
			cout << "FIRST(" << symbolType2symbolStr[i] << ")\t { ";

			if (!firstSet[i].any())
			{
				cout << " }\n";
				continue;
			}

			for (int j = 0; j < 128; ++j)
				if (firstSet[i].test(j))
					cout << symbolType2symbolStr[j] << ", ";

			cout << "\b\b }" << endl;
		}
	}

	void computeFollowSets()
	{
		followSet.clear();
		followSet.resize(symbolType2symbolStr.size());

		// Iterate untill no update
		bool isUpdated = true;
		while (isUpdated)
		{
			isUpdated = false;

			for (const auto& production : productions)
			{
				for (int j = 1; j < production.size(); ++j)
				{
					if (production[j] < num_terminals)
						continue;

					bitset<128> bits = followSet[production[j]];

					for (int k = j + 1; k < production.size(); ++k)
					{
						bits |= firstSet[production[k]];

						if (!nullable.test(production[k]))
							break;

						if (k == production.size() - 1)
							bits |= followSet[production[0]];
					}

					if ((bits ^ followSet[production[j]]).any())
					{
						followSet[production[j]] |= bits;
						isUpdated = true;
					}
				}
			}
		}

		// remove eps from follow set
		for (auto& follow : followSet)
			follow.reset(0);

		cout << "Follow sets: " << endl;
		for (int i = 0; i < followSet.size(); ++i)
		{
			cout << "FOLLOW(" << symbolType2symbolStr[i] << ")\t { ";

			if (!followSet[i].any())
			{
				cout << " }\n";
				continue;
			}

			for (int j = 0; j < 128; ++j)
				if (followSet[i].test(j))
					cout << symbolType2symbolStr[j] << ", ";

			cout << "\b\b }" << endl;
		}
	}

	void computeParseTable()
	{
		parseTable.clear();
		parseTable.resize(symbolType2symbolStr.size(), vector<int>(num_terminals, -1));

		vector<bitset<128>> select(symbolType2symbolStr.size());

		for (auto& production : productions)
		{
			for (int j = 1; j < production.size(); ++j)
			{
				select[production[0]] |= firstSet[production[j]];

				if (!nullable.test(production[j]))
					break;

				if (j == production.size() - 1)
					select[production[0]] |= followSet[production[0]];
			}
		}

		// fill parse table
		for (int i = 0; i < productions.size(); ++i)
		{
			for (int j = 0; j < num_terminals; ++j)
			{
				if (!select[productions[i][0]].test(j))
					continue;

				parseTable[productions[i][0]][j] = i;
			}
		}

		// fill for sync sets
		for (int i = 0; i < parseTable.size(); ++i)
		{
			for (int j = 0; j < num_terminals; ++j)
			{
				if (parseTable[i][j] > -1)
					continue;

				if (followSet[i].test(j))
					parseTable[i][j] = -2;
			}
		}

		for (auto& keyword : dfa.keywordTokens)
		{
			int col = symbolStr2symbolType[keyword];

			assert(col > 0);

			for (auto& row : parseTable)
				if (row[col] == -1)
					row[col] = -2;
		}
	}
};

Parser parser;

void loadParser()
{
	ifstream grammarReader{ GrammarLoc };
	assert(grammarReader);

	int num_productions;
	grammarReader >> parser.num_terminals >> parser.num_non_terminals >> num_productions >> parser.start_index;
	parser.symbolType2symbolStr.clear();
	parser.symbolType2symbolStr.resize(parser.num_terminals + parser.num_non_terminals);

	for (int i = 0; i < parser.symbolType2symbolStr.size(); ++i)
	{
		string BUFF;
		grammarReader >> BUFF;
		parser.symbolType2symbolStr[i] = BUFF;
		parser.symbolStr2symbolType[BUFF] = i;
	}

	parser.productions.clear();
	parser.productions.resize(num_productions);

	for (int i = 0; i < parser.productions.size(); i++)
	{
		string BUFF;
		std::getline(grammarReader >> std::ws, BUFF);

		for (size_t pos = 0; (pos = BUFF.find(" ")) != std::string::npos; BUFF.erase(0, pos + 1))
			parser.productions[i].push_back(
				parser.symbolStr2symbolType[BUFF.substr(0, pos)]);

		parser.productions[i].push_back(
			parser.symbolStr2symbolType[BUFF]);
	}

	parser.computeNullables();
	parser.computeFirstSets();
	parser.computeFollowSets();
	parser.computeParseTable();
}

