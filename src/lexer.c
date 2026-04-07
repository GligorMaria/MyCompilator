#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"
#include "utils.h"

Token *tokens = NULL;
Token *lastToken = NULL;
int line = 1;

static const char *pCrtCh = NULL;

// ── helpers ──────────────────────────────────────────────────────────────────

char *createString(const char *start, const char *end)
{
    int len = end - start;
    char *s;
    SAFEALLOC(s, char);
    s = (char *)malloc(len + 1);
    if (!s) err("not enough memory");
    memcpy(s, start, len);
    s[len] = '\0';
    return s;
}

Token *addTk(int code)
{
    Token *tk;
    SAFEALLOC(tk, Token);
    tk->code = code;
    tk->line = line;
    tk->next = NULL;
    if (lastToken) {
        lastToken->next = tk;
    } else {
        tokens = tk;
    }
    lastToken = tk;
    return tk;
}

void showTokens()
{
    Token *tk = tokens;
    while (tk) {
        switch (tk->code) {
            case ID:        printf("ID(%s)\n",    tk->text); break;
            case CT_INT:    printf("CT_INT(%ld)\n", tk->i);  break;
            case CT_REAL:   printf("CT_REAL(%g)\n", tk->r);  break;
            case CT_CHAR:   printf("CT_CHAR(%ld)\n", tk->i); break;
            case CT_STRING: printf("CT_STRING(%s)\n", tk->text); break;
            default:        printf("%d\n", tk->code); break;
        }
        tk = tk->next;
    }
}

// ── lexer init ────────────────────────────────────────────────────────────────

void initLexer(const char *p)
{
    pCrtCh = p;
}

// ── getNextToken ──────────────────────────────────────────────────────────────

int getNextToken()
{
    int state = 0;
    char ch;
    const char *pStartCh = NULL;
    Token *tk;

    while (1) {
        ch = *pCrtCh;
        switch (state) {

        // ── STATE 0: initial dispatch ─────────────────────────────────────────
        case 0:
            if (isalpha(ch) || ch == '_') {
                pStartCh = pCrtCh;
                pCrtCh++;
                state = 1;
            }
            // strings
            else if (ch == '"') {
                pStartCh = pCrtCh + 1; // skip opening quote
                pCrtCh++;
                state = 20;
            }
            // chars
            else if (ch == '\'') {
                pCrtCh++;
                state = 23;
            }
            // hex/octal/real starting with 0
            else if (ch == '0') {
                pStartCh = pCrtCh;
                pCrtCh++;
                state = 10;
            }
            // decimal integer or real starting with 1-9
            else if (ch >= '1' && ch <= '9') {
                pStartCh = pCrtCh;
                pCrtCh++;
                state = 8;
            }
            // operators
            else if (ch == '=') { pCrtCh++; state = 30; }
            else if (ch == '!') { pCrtCh++; state = 31; }
            else if (ch == '<') { pCrtCh++; state = 32; }
            else if (ch == '>') { pCrtCh++; state = 33; }
            else if (ch == '/') { pCrtCh++; state = 34; }
            else if (ch == '&') { pCrtCh++; state = 37; }
            else if (ch == '|') { pCrtCh++; state = 38; }
            // single-char tokens
            else if (ch == '+') { pCrtCh++; addTk(ADD);       return ADD; }
            else if (ch == '-') { pCrtCh++; addTk(SUB);       return SUB; }
            else if (ch == '*') { pCrtCh++; addTk(MUL);       return MUL; }
            else if (ch == '.') { pCrtCh++; addTk(DOT);       return DOT; }
            else if (ch == ',') { pCrtCh++; addTk(COMMA);     return COMMA; }
            else if (ch == ';') { pCrtCh++; addTk(SEMICOLON); return SEMICOLON; }
            else if (ch == '(') { pCrtCh++; addTk(LPAR);      return LPAR; }
            else if (ch == ')') { pCrtCh++; addTk(RPAR);      return RPAR; }
            else if (ch == '[') { pCrtCh++; addTk(LBRACKET);  return LBRACKET; }
            else if (ch == ']') { pCrtCh++; addTk(RBRACKET);  return RBRACKET; }
            else if (ch == '{') { pCrtCh++; addTk(LACC);      return LACC; }
            else if (ch == '}') { pCrtCh++; addTk(RACC);      return RACC; }
            // whitespace
            else if (ch == ' ' || ch == '\r' || ch == '\t') { pCrtCh++; }
            else if (ch == '\n') { line++; pCrtCh++; }
            // EOF
            else if (ch == '\0') { addTk(END); return END; }
            else tkerr(addTk(END), "invalid character: %c", ch);
            break;

        // ── STATES 1-2: identifier / keyword ─────────────────────────────────
        case 1:
            if (isalnum(ch) || ch == '_') pCrtCh++;
            else state = 2;
            break;

        case 2: {
            int nCh = pCrtCh - pStartCh;
            if      (nCh==5  && !memcmp(pStartCh,"break",5))  tk = addTk(BREAK);
            else if (nCh==4  && !memcmp(pStartCh,"char",4))   tk = addTk(CHAR);
            else if (nCh==6  && !memcmp(pStartCh,"double",6)) tk = addTk(DOUBLE);
            else if (nCh==4  && !memcmp(pStartCh,"else",4))   tk = addTk(ELSE);
            else if (nCh==3  && !memcmp(pStartCh,"for",3))    tk = addTk(FOR);
            else if (nCh==2  && !memcmp(pStartCh,"if",2))     tk = addTk(IF);
            else if (nCh==3  && !memcmp(pStartCh,"int",3))    tk = addTk(INT_KW);
            else if (nCh==6  && !memcmp(pStartCh,"return",6)) tk = addTk(RETURN);
            else if (nCh==6  && !memcmp(pStartCh,"struct",6)) tk = addTk(STRUCT);
            else if (nCh==4  && !memcmp(pStartCh,"void",4))   tk = addTk(VOID);
            else if (nCh==5  && !memcmp(pStartCh,"while",5))  tk = addTk(WHILE);
            else {
                tk = addTk(ID);
                tk->text = createString(pStartCh, pCrtCh);
            }
            return tk->code;
        }

        // ── STATES 8-9: decimal integer [1-9][0-9]* ──────────────────────────
        // can become CT_REAL if followed by '.' or 'e'/'E'
        case 8:
            if (ch >= '0' && ch <= '9') pCrtCh++;
            else if (ch == '.') { pCrtCh++; state = 15; }  // real: fractional part
            else if (ch == 'e' || ch == 'E') { pCrtCh++; state = 17; } // real: exponent
            else state = 9;
            break;

        case 9: {
            // CT_INT from decimal
            char tmp[64];
            int len = pCrtCh - pStartCh;
            memcpy(tmp, pStartCh, len);
            tmp[len] = '\0';
            tk = addTk(CT_INT);
            tk->i = strtol(tmp, NULL, 10);
            return CT_INT;
        }

        // ── STATES 10-14: starts with '0' → octal / hex / real ───────────────
        case 10:
            if (ch == 'x' || ch == 'X') { pCrtCh++; state = 11; }  // hex
            else if (ch >= '0' && ch <= '7') { pCrtCh++; state = 12; } // octal
            else if (ch == '.') { pCrtCh++; state = 15; }  // 0.something → real
            else if (ch == 'e' || ch == 'E') { pCrtCh++; state = 17; }
            else state = 13; // just "0" → CT_INT value 0
            break;

        case 11:
            // hex digits
            if (isxdigit(ch)) pCrtCh++;
            else state = 14;
            break;

        case 12:
            // octal digits
            if (ch >= '0' && ch <= '7') pCrtCh++;
            else if (ch == '8' || ch == '9')
                tkerr(addTk(END), "invalid octal digit: %c", ch);
            else state = 13;
            break;

        case 13: {
            // CT_INT from octal or just "0"
            char tmp[64];
            int len = pCrtCh - pStartCh;
            memcpy(tmp, pStartCh, len);
            tmp[len] = '\0';
            tk = addTk(CT_INT);
            tk->i = strtol(tmp, NULL, 8);
            return CT_INT;
        }

        case 14: {
            // CT_INT from hex
            char tmp[64];
            int len = pCrtCh - pStartCh;
            memcpy(tmp, pStartCh, len);
            tmp[len] = '\0';
            tk = addTk(CT_INT);
            tk->i = strtol(tmp, NULL, 16);
            return CT_INT;
        }

        // ── STATES 15-19: CT_REAL ─────────────────────────────────────────────
        // state 15: after '.' — fractional digits
        case 15:
            if (ch >= '0' && ch <= '9') pCrtCh++;
            else if (ch == 'e' || ch == 'E') { pCrtCh++; state = 17; }
            else state = 16;
            break;

        case 16: {
            // CT_REAL — no exponent
            char tmp[64];
            int len = pCrtCh - pStartCh;
            memcpy(tmp, pStartCh, len);
            tmp[len] = '\0';
            tk = addTk(CT_REAL);
            tk->r = atof(tmp);
            return CT_REAL;
        }

        // state 17: after 'e'/'E' — optional sign
        case 17:
            if (ch == '+' || ch == '-') { pCrtCh++; state = 18; }
            else if (ch >= '0' && ch <= '9') { pCrtCh++; state = 19; }
            else tkerr(addTk(END), "invalid exponent in real constant");
            break;

        // state 18: after sign — must have at least one digit
        case 18:
            if (ch >= '0' && ch <= '9') { pCrtCh++; state = 19; }
            else tkerr(addTk(END), "missing exponent digits in real constant");
            break;

        // state 19: exponent digits
        case 19:
            if (ch >= '0' && ch <= '9') pCrtCh++;
            else {
                char tmp[64];
                int len = pCrtCh - pStartCh;
                memcpy(tmp, pStartCh, len);
                tmp[len] = '\0';
                tk = addTk(CT_REAL);
                tk->r = atof(tmp);
                return CT_REAL;
            }
            break;

        // ── STATES 20-22: CT_STRING ───────────────────────────────────────────
        case 20:
            if (ch == '\\') { pCrtCh++; state = 21; } // escape sequence
            else if (ch == '"') {
                // closing quote
                tk = addTk(CT_STRING);
                tk->text = createString(pStartCh, pCrtCh);
                pCrtCh++; // consume closing "
                return CT_STRING;
            }
            else if (ch == '\0' || ch == '\n')
                tkerr(addTk(END), "unterminated string constant");
            else pCrtCh++;
            break;

        case 21:
            // after backslash — consume the escaped character and go back
            if (ch == '\0') tkerr(addTk(END), "unterminated escape sequence");
            pCrtCh++;
            state = 20;
            break;

        // ── STATES 23-25: CT_CHAR ─────────────────────────────────────────────
        case 23:
            if (ch == '\\') { pCrtCh++; state = 24; } // escape
            else if (ch == '\'' || ch == '\0')
                tkerr(addTk(END), "empty or invalid char constant");
            else { pCrtCh++; state = 25; }
            break;

        case 24:
            // escaped char: \n \t \\ \' etc.
            if (ch == '\0') tkerr(addTk(END), "unterminated char constant");
            pCrtCh++;
            state = 25;
            break;

        case 25:
            if (ch == '\'') {
                pCrtCh++; // consume closing '
                // compute char value from the single char (or escape)
                const char *p = pCrtCh - 2; // points to the char itself
                tk = addTk(CT_CHAR);
                tk->i = (unsigned char)*p;
                return CT_CHAR;
            }
            else tkerr(addTk(END), "too many characters in char constant");
            break;

        // ── STATES 30-33: == != <= >= ─────────────────────────────────────────
        case 30:
            if (ch == '=') { pCrtCh++; addTk(EQUAL); return EQUAL; }
            else { addTk(ASSIGN); return ASSIGN; }

        case 31:
            if (ch == '=') { pCrtCh++; addTk(NOTEQ); return NOTEQ; }
            else { addTk(NOT); return NOT; }

        case 32:
            if (ch == '=') { pCrtCh++; addTk(LESSEQ); return LESSEQ; }
            else { addTk(LESS); return LESS; }

        case 33:
            if (ch == '=') { pCrtCh++; addTk(GREATEREQ); return GREATEREQ; }
            else { addTk(GREATER); return GREATER; }

        // ── STATES 34-36: / or // or /* */ ───────────────────────────────────
        case 34:
            if (ch == '/') { pCrtCh++; state = 35; }      // line comment
            else if (ch == '*') { pCrtCh++; state = 36; }  // block comment
            else { addTk(DIV); return DIV; }
            break;

        case 35:
            // line comment: skip until newline
            if (ch == '\n') { line++; pCrtCh++; state = 0; }
            else if (ch == '\0') { state = 0; }
            else pCrtCh++;
            break;

        case 36:
            // block comment: skip until */
            if (ch == '*') { pCrtCh++; state = 39; }
            else if (ch == '\n') { line++; pCrtCh++; }
            else if (ch == '\0') tkerr(addTk(END), "unterminated block comment");
            else pCrtCh++;
            break;

        case 39:
            // seen '*' inside block comment — check for closing '/'
            if (ch == '/') { pCrtCh++; state = 0; }
            else if (ch == '*') pCrtCh++;  // another star, stay
            else { pCrtCh++; state = 36; }
            break;

        // ── STATES 37-38: && || ───────────────────────────────────────────────
        case 37:
            if (ch == '&') { pCrtCh++; addTk(AND); return AND; }
            else tkerr(addTk(END), "expected '&' after '&'");
            break;

        case 38:
            if (ch == '|') { pCrtCh++; addTk(OR); return OR; }
            else tkerr(addTk(END), "expected '|' after '|'");
            break;

        } // end switch
    } // end while
}