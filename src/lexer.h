typedef enum {
    ID, END,
    CT_INT, CT_REAL, CT_CHAR, CT_STRING,
    
    // Operatori 
    ASSIGN, EQUAL,
    NOT, NOTEQ,
    LESS, LESSEQ,
    GREATER, GREATEREQ,
    ADD, SUB, MUL, DIV,
    AND, OR,
    DOT, COMMA,
    LPAR, RPAR,
    LBRACKET, RBRACKET,
    LACC, RACC,
    
    // Keywords
    BREAK, CHAR, DOUBLE, ELSE, FOR, IF, INT,
    RETURN, STRUCT, VOID, WHILE
} TokenCode;

typedef struct _Token {
    int code;
    union {
        char *text;    // ID, CT_STRING
        long int i;    // CT_INT, CT_CHAR
        double r;      // CT_REAL
    };
    int line;
    struct _Token *next;
} Token;