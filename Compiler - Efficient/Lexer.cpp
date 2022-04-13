#include "Lexer.h"
#include <map>
#include <set>
#include <vector>
#include <cassert>
#include <iomanip>

vector<string> tokenType2tokenStr;

int num_tokens;
int num_states;
int num_transitions;
int num_finalstates;
int num_keywords;

vector<vector<int>> productions;
vector<TokenType> finalStates;
map<string, TokenType> tokenStr2tokenType;
map<string, TokenType> lookupTable;	// move to global in future

void loadLexer()
{
	std::ifstream dfa{ LexerLoc };
	assert(dfa);
	dfa >> num_tokens >> num_states >> num_transitions >> num_finalstates >> num_keywords;

	// Load Tokens
	tokenType2tokenStr.resize(num_tokens);
	for (int i = 0; i < num_tokens; ++i)
	{
		dfa >> tokenType2tokenStr[i];
		tokenStr2tokenType[tokenType2tokenStr[i]] = (TokenType)i;
	}

	// Load Transitions
	productions.assign(num_states, vector<int>(128, -1));

	for (int i = 0; i < num_transitions; ++i)
	{
		int from, to;
		string symbols;
		dfa >> from >> to >> symbols;

		for (char c : symbols)
			productions[from][c] = to;
	}

	for (int i = 0; i < 128; i++)
		productions[48][i] = 48;		// state 48 - accept state for comment

	productions[48]['\n'] = -1;

	productions[0][' ']
		= productions[0]['\t']
		= productions[0]['\r']
		= productions[50][' ']
		= productions[50]['\t']
		= productions[50]['\r']
		= 50;

	// Load Final States
	finalStates.assign(num_states, TokenType::UNINITIALISED);

	for (int i = 0; i < num_finalstates; i++)
	{
		int state;
		string BUFF;
		dfa >> state >> BUFF;

		finalStates[state] = tokenStr2tokenType[BUFF];
	}

	// Load Keywords
	for (int i = 0; i < num_keywords; ++i)
	{
		string keyword, token_name;
		dfa >> keyword >> token_name;
		lookupTable[keyword] = tokenStr2tokenType[token_name];
	}
}

std::ostream& operator<< (std::ostream& out, const Token& token)
{
	out << "Line no: " << token.line_number << "\tToken: " << setw(20) << tokenType2tokenStr[(int)token.type] << "\tLexeme: " << token.lexeme;
	return out;
}

Token* Lexer::DFA(int start_index)
{
	TokenType ttype;
	int last_final = -1;
	int input_final_pos = start_index - 1;

	int len = 0;

	int cur_state = 0;
	// start index = index of character to read next

	while (1)
	{
		char input = buffer.getChar(start_index);

		last_final = cur_state;
		ttype = finalStates[cur_state];
		input_final_pos = start_index - 1;

		cur_state = productions[cur_state][input];

		if (cur_state == -1)    // return
		{
			if (input_final_pos == start_index - len - 1)
			{
				Token* token = new Token;
				token->type = TokenType::TK_ERROR_SYMBOL;
				token->length = 1;
				return token;
			}
			if (finalStates[last_final] == TokenType::UNINITIALISED && last_final != 0)
			{
				Token* token = new Token;
				token->type = TokenType::TK_ERROR_PATTERN;
				token->length = len;
				return token;
			}

			Token* token = new Token;
			token->type = ttype;
			token->length = input_final_pos - (start_index - len) + 1;
			return token;
		}

		start_index++;
		len++;
	}

	// this should not be reachable as our DFA is capable of handling every case
	assert(false);
}

Lexer::Lexer(const char* fileLoc) : buffer{ fileLoc }
{
	static bool isLexerLoaded = false;
	if (!isLexerLoaded)
	{
		loadLexer();
		isLexerLoaded = true;
	}
}

const Token* Lexer::getNextToken()
{
	while (buffer.getTopChar() != '\0')
	{
		if (buffer.getTopChar() == '\n')
		{
			buffer.line_number++;
			buffer.start_index++;
			continue;
		}

		Token* token = DFA(buffer.start_index);

		assert(token != nullptr);

		token->start_index = buffer.start_index;
		token->line_number = buffer.line_number;

		buffer.start_index += token->length;

		if (token->type == TokenType::TK_COMMENT || token->type == TokenType::TK_WHITESPACE)
		{
			delete token;
			continue;
		}

		token->lexeme.resize(token->length + 1);
		for (int i = 0; i < token->length; i++)
			token->lexeme[i] = buffer.getChar(buffer.start_index - token->length + i);

		if (token->type == TokenType::TK_ID || token->type == TokenType::TK_FUNID || token->type == TokenType::TK_FIELDID)
		{
			if (lookupTable.find(token->lexeme) != lookupTable.end())
				token->type = lookupTable.at(token->lexeme);
		}

		// Assign error types

		if (token->type == TokenType::TK_ID && token->length > 20)
			token->type = TokenType::TK_ERROR_LENGTH;

		if (token->type == TokenType::TK_FUNID && token->length > 30)
			token->type = TokenType::TK_ERROR_LENGTH;

		return token;
	}

	return nullptr;
}