#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdlib.h>

typedef enum {
    ID, END,
    CT_INT, CT_REAL, CT_CHAR, CT_STRING,

    // Operatori
    ASSIGN, EQUAL,
    NOT, NOTEQ,
    LESS, LESSEQ,
    SEMICOLON,
    GREATER, GREATEREQ,
    ADD, SUB, MUL, DIV,
    AND, OR,
    DOT, COMMA,
    LPAR, RPAR,
    LBRACKET, RBRACKET,
    LACC, RACC,

    // Keywords
    BREAK, CHAR, DOUBLE, ELSE, FOR, IF, INT_KW,
    RETURN, STRUCT, VOID, WHILE
} TokenCode;

typedef struct _Token {
    int code;
    union {
        char *text;
        long int i;
        double r;
    };
    int line;
    struct _Token *next;
} Token;

// variabile globale din lexer.c
extern Token *tokens;
extern Token *lastToken;
extern int line;

// functii din lexer.c
void initLexer(const char *p);
int getNextToken();
void showTokens();
Token *addTk(int code);
char *createString(const char *start, const char *end);

#endif