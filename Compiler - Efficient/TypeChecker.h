#pragma once
#include "SymbolTable.h"


extern bool isTypeError;
void typeChecker_init();
void assignTypes(ASTNode*);