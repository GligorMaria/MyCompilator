#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

// variabile globale
extern Token *crtTk;       // tokenul curent
extern Token *consumedTk;  // ultimul token consumat

// functia de consume
int consume(int code);

// prototipuri pentru toate regulile
// (necesare pentru referinte circulare ex: stm <-> stmCompound)
int unit();
int structDef();
int varDef();
int typeBase();
int arrayDecl();
int fnDef();
int fnParam();
int stm();
int stmCompound();
int expr();
int exprAssign();
int exprOr();
int exprAnd();
int exprEq();
int exprRel();
int exprAdd();
int exprMul();
int exprCast();
int exprUnary();
int exprPostfix();
int exprPrimary();

#endif