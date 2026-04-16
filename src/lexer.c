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
    char *s = (char *)malloc(len + 1);
    if (!s) err("not enough memory");
    memcpy(s, start, len);
    s[len] = '\0';
    return s;
}

// creaza un token nou si il adauga la sfarsitul listei inlantuite
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
            // literale
            case ID:
                printf("ID(%s)\n", tk->text);
                break;
            case CT_INT:
                printf("CT_INT(%ld)\n", tk->i);
                break;
            case CT_REAL:
                printf("CT_REAL(%g)\n", tk->r);
                break;
            case CT_CHAR:
                if      (tk->i == '\n') printf("CT_CHAR(\\n)\n");
                else if (tk->i == '\t') printf("CT_CHAR(\\t)\n");
                else if (tk->i == '\\') printf("CT_CHAR(\\\\)\n");
                else if (tk->i == '\'') printf("CT_CHAR(\\')\n");
                else if (tk->i == '\0') printf("CT_CHAR(\\0)\n");
                else printf("CT_CHAR(%c)\n", (char)tk->i);
                break;
            case CT_STRING:
                printf("CT_STRING(%s)\n", tk->text);
                break;
            // keywords
            case BREAK:     printf("BREAK\n");     break;
            case CHAR:      printf("CHAR\n");      break;
            case DOUBLE:    printf("DOUBLE\n");    break;
            case ELSE:      printf("ELSE\n");      break;
            case FOR:       printf("FOR\n");       break;
            case IF:        printf("IF\n");        break;
            case INT_KW:    printf("INT\n");       break;
            case RETURN:    printf("RETURN\n");    break;
            case STRUCT:    printf("STRUCT\n");    break;
            case VOID:      printf("VOID\n");      break;
            case WHILE:     printf("WHILE\n");     break;
            // operatori
            case ASSIGN:    printf("ASSIGN\n");    break;
            case EQUAL:     printf("EQUAL\n");     break;
            case NOT:       printf("NOT\n");       break;
            case NOTEQ:     printf("NOTEQ\n");     break;
            case LESS:      printf("LESS\n");      break;
            case LESSEQ:    printf("LESSEQ\n");    break;
            case GREATER:   printf("GREATER\n");   break;
            case GREATEREQ: printf("GREATEREQ\n"); break;
            case ADD:       printf("ADD\n");       break;
            case SUB:       printf("SUB\n");       break;
            case MUL:       printf("MUL\n");       break;
            case DIV:       printf("DIV\n");       break;
            case AND:       printf("AND\n");       break;
            case OR:        printf("OR\n");        break;
            case DOT:       printf("DOT\n");       break;
            case COMMA:     printf("COMMA\n");     break;
            case SEMICOLON: printf("SEMICOLON\n"); break;
            // paranteze
            case LPAR:      printf("LPAR\n");      break;
            case RPAR:      printf("RPAR\n");      break;
            case LBRACKET:  printf("LBRACKET\n");  break;
            case RBRACKET:  printf("RBRACKET\n");  break;
            case LACC:      printf("LACC\n");      break;
            case RACC:      printf("RACC\n");      break;
            case END:       printf("END\n");       break;
            default:        printf("UNKNOWN(%d)\n", tk->code); break;
        }
        tk = tk->next;
    }
}

// init pointerul la caracterul curent din fisier
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
            // operatori cu doua caractere posibile
            else if (ch == '=') { pCrtCh++; state = 30; }
            else if (ch == '!') { pCrtCh++; state = 31; }
            else if (ch == '<') { pCrtCh++; state = 32; }
            else if (ch == '>') { pCrtCh++; state = 33; }
            else if (ch == '/') { pCrtCh++; state = 34; }
            else if (ch == '&') { pCrtCh++; state = 37; }
            else if (ch == '|') { pCrtCh++; state = 38; }
            // operatori cu un singur caracter
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
        // poate deveni CT_REAL daca e urmat de '.' sau 'e'/'E'
        case 8:
            if (ch >= '0' && ch <= '9') pCrtCh++;
            else if (ch == '.') { pCrtCh++; state = 15; }
            else if (ch == 'e' || ch == 'E') { pCrtCh++; state = 17; }
            else state = 9;
            break;

        case 9: {
            // CT_INT din zecimal
            char tmp[64];
            int len = pCrtCh - pStartCh;
            if (len > 63) len = 63;
            memcpy(tmp, pStartCh, len);
            tmp[len] = '\0';
            tk = addTk(CT_INT);
            tk->i = strtol(tmp, NULL, 10);
            return CT_INT;
        }

        // ── STATES 10-14: incepe cu '0' → octal / hex / real ─────────────────
        case 10:
            if (ch == 'x' || ch == 'X') { pCrtCh++; state = 11; }     // hex
            else if (ch >= '0' && ch <= '7') { pCrtCh++; state = 12; } // octal
            else if (ch == '.') { pCrtCh++; state = 15; }              // 0.ceva → real
            else if (ch == 'e' || ch == 'E') { pCrtCh++; state = 17; } // 0e... → real
            else state = 13;                                             // doar "0"
            break;

        case 11:
            // cifre hex
            if (isxdigit(ch)) pCrtCh++;
            else state = 14;
            break;

        case 12:
            // cifre octale
            if (ch >= '0' && ch <= '7') pCrtCh++;
            else if (ch == '8' || ch == '9')
                tkerr(addTk(END), "invalid octal digit: %c", ch);
            else state = 13;
            break;

        case 13: {
            // CT_INT din octal sau "0"
            // baza 0 = auto-detectie: 0x→hex, 0→octal, altfel→zecimal
            char tmp[64];
            int len = pCrtCh - pStartCh;
            if (len > 63) len = 63;
            memcpy(tmp, pStartCh, len);
            tmp[len] = '\0';
            tk = addTk(CT_INT);
            tk->i = strtol(tmp, NULL, 0);
            return CT_INT;
        }

        case 14: {
            // CT_INT din hex (ex: 0xAFd9)
            // baza 0 = auto-detectie, recunoaste prefixul 0x automat
            char tmp[64];
            int len = pCrtCh - pStartCh;
            if (len > 63) len = 63;
            memcpy(tmp, pStartCh, len);
            tmp[len] = '\0';
            tk = addTk(CT_INT);
            tk->i = strtol(tmp, NULL, 0);
            return CT_INT;
        }

        // ── STATES 15-19: CT_REAL ─────────────────────────────────────────────
        // state 15: dupa '.' — cifre fractionale
        case 15:
            if (ch >= '0' && ch <= '9') pCrtCh++;
            else if (ch == 'e' || ch == 'E') { pCrtCh++; state = 17; }
            else state = 16;
            break;

        case 16: {
            // CT_REAL fara exponent (ex: 3.14)
            char tmp[64];
            int len = pCrtCh - pStartCh;
            if (len > 63) len = 63;
            memcpy(tmp, pStartCh, len);
            tmp[len] = '\0';
            tk = addTk(CT_REAL);
            tk->r = atof(tmp);
            return CT_REAL;
        }

        // state 17: dupa 'e'/'E' — semn optional
        case 17:
            if (ch == '+' || ch == '-') { pCrtCh++; state = 18; }
            else if (ch >= '0' && ch <= '9') { pCrtCh++; state = 19; }
            else tkerr(addTk(END), "invalid exponent in real constant");
            break;

        // state 18: dupa semn — trebuie cel putin o cifra
        case 18:
            if (ch >= '0' && ch <= '9') { pCrtCh++; state = 19; }
            else tkerr(addTk(END), "missing exponent digits in real constant");
            break;

        // state 19: cifre exponent
        case 19:
            if (ch >= '0' && ch <= '9') pCrtCh++;
            else {
                char tmp[64];
                int len = pCrtCh - pStartCh;
                if (len > 63) len = 63;
                memcpy(tmp, pStartCh, len);
                tmp[len] = '\0';
                tk = addTk(CT_REAL);
                tk->r = atof(tmp);
                return CT_REAL;
            }
            break;

        // ── STATES 20-21: CT_STRING ───────────────────────────────────────────
        case 20:
            if (ch == '\\') { pCrtCh++; state = 21; }  // escape sequence
            else if (ch == '"') {
                // ghilimele de inchidere
                tk = addTk(CT_STRING);
                tk->text = createString(pStartCh, pCrtCh);
                pCrtCh++; // consuma "
                return CT_STRING;
            }
            else if (ch == '\0' || ch == '\n')
                tkerr(addTk(END), "unterminated string constant");
            else pCrtCh++;
            break;

        case 21:
            // dupa backslash — consuma caracterul escaped si revine
            if (ch == '\0') tkerr(addTk(END), "unterminated escape sequence");
            pCrtCh++;
            state = 20;
            break;

        // ── STATES 23-25: CT_CHAR ─────────────────────────────────────────────
        case 23:
            if (ch == '\\') { pCrtCh++; state = 24; }  // escape
            else if (ch == '\'' || ch == '\0')
                tkerr(addTk(END), "empty or invalid char constant");
            else { pCrtCh++; state = 25; }
            break;

        case 24:
            // caracter escaped: \n \t \\ \' \0 etc.
            if (ch == '\0') tkerr(addTk(END), "unterminated char constant");
            pCrtCh++;
            state = 25;
            break;

        case 25:
            if (ch == '\'') {
                pCrtCh++; // consuma ghilimelele simple de inchidere
                tk = addTk(CT_CHAR);
                // verifica daca a fost escape sequence (ex: '\n')
                // pCrtCh - 3 = backslash-ul, pCrtCh - 2 = litera dupa backslash
                if (*(pCrtCh - 3) == '\\') {
                    char escaped = *(pCrtCh - 2);
                    switch (escaped) {
                        case 'n':  tk->i = '\n'; break;
                        case 't':  tk->i = '\t'; break;
                        case '\\': tk->i = '\\'; break;
                        case '\'': tk->i = '\''; break;
                        case '0':  tk->i = '\0'; break;
                        case 'r':  tk->i = '\r'; break;
                        default:   tk->i = (unsigned char)escaped; break;
                    }
                } else {
                    // caracter normal (ex: 'A')
                    tk->i = (unsigned char)*(pCrtCh - 2);
                }
                return CT_CHAR;
            }
            else tkerr(addTk(END), "too many characters in char constant");
            break;

        // ── STATES 30-33: == != <= >= ─────────────────────────────────────────
        case 30:
            if (ch == '=') { pCrtCh++; addTk(EQUAL);     return EQUAL; }
            else {                      addTk(ASSIGN);    return ASSIGN; }

        case 31:
            if (ch == '=') { pCrtCh++; addTk(NOTEQ);     return NOTEQ; }
            else {                      addTk(NOT);       return NOT; }

        case 32:
            if (ch == '=') { pCrtCh++; addTk(LESSEQ);    return LESSEQ; }
            else {                      addTk(LESS);      return LESS; }

        case 33:
            if (ch == '=') { pCrtCh++; addTk(GREATEREQ); return GREATEREQ; }
            else {                      addTk(GREATER);   return GREATER; }

        // ── STATES 34-36, 39: / sau // sau /* */ ─────────────────────────────
        case 34:
            if (ch == '/')      { pCrtCh++; state = 35; }  // comentariu linie
            else if (ch == '*') { pCrtCh++; state = 36; }  // comentariu bloc
            else { addTk(DIV); return DIV; }
            break;

        case 35:
            // comentariu linie: sari pana la newline
            if (ch == '\n') { line++; pCrtCh++; state = 0; }
            else if (ch == '\0') { state = 0; }
            else pCrtCh++;
            break;

        case 36:
            // comentariu bloc: sari pana la */
            if (ch == '*')       { pCrtCh++; state = 39; }
            else if (ch == '\n') { line++; pCrtCh++; }
            else if (ch == '\0') tkerr(addTk(END), "unterminated block comment");
            else pCrtCh++;
            break;

        case 39:
            // am vazut '*' in comentariu bloc — verificam '/'
            if (ch == '/')      { pCrtCh++; state = 0; }   // sfarsit comentariu
            else if (ch == '*')   pCrtCh++;                 // alt '*', ramane
            else                { pCrtCh++; state = 36; }  // altceva, inapoi in bloc
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