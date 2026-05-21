#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "lexer.h"
#include "utils.h"

//verifică dacă ordinea tokenilor respectă regulile gramaticale
Token *crtTk = NULL;
Token *consumedTk = NULL;

int consume(int code)
{
    if (crtTk->code == code) {
        consumedTk = crtTk;
        crtTk = crtTk->next;
        return 1;
    }
    return 0;
}

//unit: ( structDef | fnDef | varDef )* END 
int unit()
{
    while (1) {
        if (structDef()) {}
        else if (fnDef()) {}
        else if (varDef()) {}
        else break;
    }
    if (!consume(END))
        tkerr(crtTk, "unexpected token at top level");
    return 1;
}

// structDef: STRUCT ID LACC varDef* RACC SEMICOLON 
/*
Recunoaște o declarație de structură. Verifică că începe cu cuvântul struct, urmat de un nume, urmat de acoladă deschisă, 
apoi câmpurile structurii, 
și în final acoladă închisă cu punct și virgulă. Dacă nu vede struct la început, spune că nu e o structură și se oprește.
*/
int structDef()
{
    Token *startTk = crtTk;
    if (!consume(STRUCT)) return 0;
    if (!consume(ID))
        tkerr(crtTk, "expected identifier after struct");
    if (!consume(LACC))
        tkerr(crtTk, "expected { after struct name");
    while (varDef()) {}
    if (!consume(RACC))
        tkerr(crtTk, "expected } after struct body");
    if (!consume(SEMICOLON))
        tkerr(crtTk, "expected ; after struct definition");
    return 1;
}

// varDef: typeBase ID arrayDecl? SEMICOLON
int varDef()
{
    Token *startTk = crtTk;
    if (!typeBase()) return 0;
    if (!consume(ID)) {
        crtTk = startTk;
        
        return 0;
    }
    arrayDecl(); // optional
    if (!consume(SEMICOLON))
        tkerr(crtTk, "expected ; after variable definition");
    return 1;
}

// typeBase: INT | DOUBLE | CHAR | STRUCT ID 
int typeBase()
{
    Token *startTk = crtTk;
    if (consume(INT_KW))   return 1;
    if (consume(DOUBLE))   return 1;
    if (consume(CHAR))     return 1;
    if (consume(STRUCT)) {
        if (!consume(ID))
            tkerr(crtTk, "expected identifier after struct");
        return 1;
    }
    return 0;
}

// arrayDecl: LBRACKET CT_INT? RBRACKET 
int arrayDecl()
{
    Token *startTk = crtTk;
    if (!consume(LBRACKET)) return 0;
    consume(CT_INT); // optional
    if (!consume(RBRACKET))
        tkerr(crtTk, "expected ] in array declaration");
    return 1;
}

// fnDef: ( typeBase | VOID ) ID LPAR ( fnParam ( COMMA fnParam )* )? RPAR stmCompound
int fnDef()
{
    Token *startTk = crtTk;
    if (!typeBase()) {
        if (!consume(VOID)) return 0;
    }
    if (!consume(ID)) {
        crtTk = startTk;
        return 0;
    }
    if (!consume(LPAR)) {
        crtTk = startTk;
        return 0;
    }
    // parametri optionali
    if (fnParam()) {
        while (consume(COMMA)) {
            if (!fnParam())
                tkerr(crtTk, "expected parameter after ,");
        }
    }
    if (!consume(RPAR))
        tkerr(crtTk, "expected ) after function parameters");
    if (!stmCompound())
        tkerr(crtTk, "expected function body");
    return 1;
}

// fnParam: typeBase ID arrayDecl?
int fnParam()
{
    Token *startTk = crtTk;
    if (!typeBase()) return 0;
    if (!consume(ID)) {
        crtTk = startTk;
        return 0;
    }
    arrayDecl(); // optional
    return 1;
}

// stmCompound: LACC ( varDef | stm )* RACC 
int stmCompound()
{
    Token *startTk = crtTk;
    if (!consume(LACC)) return 0;
    while (1) {
        if (varDef()) {}
        else if (stm()) {}
        else break;
    }
    if (!consume(RACC))
        tkerr(crtTk, "expected } or syntax error in block");
    return 1;
}

// stm 
// stm: stmCompound
//    | IF LPAR expr RPAR stm ( ELSE stm )?
//    | WHILE LPAR expr RPAR stm
//    | FOR LPAR expr? SEMICOLON expr? SEMICOLON expr? RPAR stm
//    | BREAK SEMICOLON
//    | RETURN expr? SEMICOLON
//    | expr? SEMICOLON
int stm()
{
    Token *startTk = crtTk;

    // stmCompound
    if (stmCompound()) return 1;

    // IF
    if (consume(IF)) {
        if (!consume(LPAR))
            tkerr(crtTk, "expected ( after if");
        if (!expr())
            tkerr(crtTk, "expected expression in if condition");
        if (!consume(RPAR))
            tkerr(crtTk, "expected ) after if condition");
        if (!stm())
            tkerr(crtTk, "expected statement after if");
        if (consume(ELSE)) {
            if (!stm())
                tkerr(crtTk, "expected statement after else");
        }
        return 1;
    }

    // WHILE
    if (consume(WHILE)) {
        if (!consume(LPAR))
            tkerr(crtTk, "expected ( after while");
        if (!expr())
            tkerr(crtTk, "expected expression in while condition");
        if (!consume(RPAR))
            tkerr(crtTk, "expected ) after while condition");
        if (!stm())
            tkerr(crtTk, "expected statement after while");
        return 1;
    }

    // FOR
    if (consume(FOR)) {
        if (!consume(LPAR))
            tkerr(crtTk, "expected ( after for");
        expr(); // optional
        if (!consume(SEMICOLON))
            tkerr(crtTk, "expected ; in for");
        expr(); // optional
        if (!consume(SEMICOLON))
            tkerr(crtTk, "expected ; in for");
        expr(); // optional
        if (!consume(RPAR))
            tkerr(crtTk, "expected ) after for");
        if (!stm())
            tkerr(crtTk, "expected statement after for");
        return 1;
    }

    // BREAK
    if (consume(BREAK)) {
        if (!consume(SEMICOLON))
            tkerr(crtTk, "expected ; after break");
        return 1;
    }

    // RETURN
    if (consume(RETURN)) {
        expr(); // optional
        if (!consume(SEMICOLON))
            tkerr(crtTk, "expected ; after return");
        return 1;
    }

    // expr? SEMICOLON
    expr(); // optional
    if (consume(SEMICOLON)) return 1;

    crtTk = startTk;
    return 0;
}

// expr: exprAssign 
int expr()
{
    return exprAssign();
}

// exprAssign: exprUnary ASSIGN exprAssign | exprOr
int exprAssign()
{
    Token *startTk = crtTk;
    if (exprUnary()) {
        if (consume(ASSIGN)) {
            if (!exprAssign())
                tkerr(crtTk, "expected expression after =");
            return 1;
        }
        crtTk = startTk; // nu e assign, incearca exprOr
    }
    return exprOr();
}

// exprOr (stanga-recursivitate eliminata)
// exprOr  ::= exprAnd exprOr1
// exprOr1 ::= OR exprAnd exprOr1 | ε
int exprOr()
{
    if (!exprAnd()) return 0;
    // exprOr1
    while (consume(OR)) {
        if (!exprAnd())
            tkerr(crtTk, "expected expression after ||");
    }
    return 1;
}

//exprAnd (stanga-recursivitate eliminata) 
int exprAnd()
{
    if (!exprEq()) return 0;
    while (consume(AND)) {
        if (!exprEq())
            tkerr(crtTk, "expected expression after &&");
    }
    return 1;
}

// exprEq (stanga-recursivitate eliminata)
int exprEq()
{
    if (!exprRel()) return 0;
    while (crtTk->code == EQUAL || crtTk->code == NOTEQ) {
        consume(crtTk->code);
        if (!exprRel())
            tkerr(crtTk, "expected expression after == or !=");
    }
    return 1;
}

// exprRel (stanga-recursivitate eliminata) 
int exprRel()
{
    if (!exprAdd()) return 0;
    while (crtTk->code == LESS   || crtTk->code == LESSEQ ||
           crtTk->code == GREATER || crtTk->code == GREATEREQ) {
        consume(crtTk->code);
        if (!exprAdd())
            tkerr(crtTk, "expected expression after relational operator");
    }
    return 1;
}

// exprAdd (stanga-recursivitate eliminata) 
int exprAdd()
{
    if (!exprMul()) return 0;
    while (crtTk->code == ADD || crtTk->code == SUB) {
        consume(crtTk->code);
        if (!exprMul())
            tkerr(crtTk, "expected expression after + or -");
    }
    return 1;
}

// exprMul (stanga-recursivitate eliminata)
int exprMul()
{
    if (!exprCast()) return 0;
    while (crtTk->code == MUL || crtTk->code == DIV) {
        consume(crtTk->code);
        if (!exprCast())
            tkerr(crtTk, "expected expression after * or /");
    }
    return 1;
}

// exprCast: LPAR typeBase arrayDecl? RPAR exprCast | exprUnary
int exprCast()
{
    Token *startTk = crtTk;
    if (consume(LPAR)) {
        if (typeBase()) {
            arrayDecl(); // optional
            if (consume(RPAR)) {
                if (!exprCast())
                    tkerr(crtTk, "expected expression after cast");
                return 1;
            }
        }
        crtTk = startTk; // nu e cast, da inapoi
    }
    return exprUnary();
}

// exprUnary: ( SUB | NOT ) exprUnary | exprPostfix 
int exprUnary()
{
    if (crtTk->code == SUB || crtTk->code == NOT) {
        consume(crtTk->code);
        if (!exprUnary())
            tkerr(crtTk, "expected expression after unary operator");
        return 1;
    }
    return exprPostfix();
}

// exprPostfix (stanga-recursivitate eliminata) 
// exprPostfix: exprPrimary ( LBRACKET expr RBRACKET | DOT ID )*
int exprPostfix()
{
    if (!exprPrimary()) return 0;
    while (1) {
        if (consume(LBRACKET)) {
            if (!expr())
                tkerr(crtTk, "expected expression inside []");
            if (!consume(RBRACKET))
                tkerr(crtTk, "expected ] after index");
        }
        else if (consume(DOT)) {
            if (!consume(ID))
                tkerr(crtTk, "expected field name after .");
        }
        else break;
    }
    return 1;
}

// exprPrimary 
// exprPrimary: ID ( LPAR ( expr ( COMMA expr )* )? RPAR )?
//            | CT_INT | CT_REAL | CT_CHAR | CT_STRING
//            | LPAR expr RPAR
int exprPrimary()
{
    Token *startTk = crtTk;

    // ID — poate fi variabila sau apel de functie
    if (consume(ID)) {
        if (consume(LPAR)) {
            // apel de functie: argumente optionale
            if (expr()) {
                while (consume(COMMA)) {
                    if (!expr())
                        tkerr(crtTk, "expected expression after ,");
                }
            }
            if (!consume(RPAR))
                tkerr(crtTk, "expected ) after function arguments");
        }
        return 1;
    }

    // constante
    if (consume(CT_INT))    return 1;
    if (consume(CT_REAL))   return 1;
    if (consume(CT_CHAR))   return 1;
    if (consume(CT_STRING)) return 1;

    // expresie intre paranteze
    if (consume(LPAR)) {
        if (!expr())
            tkerr(crtTk, "expected expression after (");
        if (!consume(RPAR))
            tkerr(crtTk, "expected ) after expression");
        return 1;
    }

    return 0;
}