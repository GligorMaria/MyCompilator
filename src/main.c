#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "utils.h"

int main(int argc, char **argv)
{
    // 1. verificam ca a fost dat un fisier ca argument
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return -1;
    }

    // 2. deschidem fisierul
    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        fprintf(stderr, "Cannot open file: %s\n", argv[1]);
        return -1;
    }

    // 3. aflam dimensiunea fisierului
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    // 4. alocam memorie si citim tot fisierul
    char *buf = (char *)malloc(fsize + 1);
    if (!buf) {
        fprintf(stderr, "Not enough memory\n");
        fclose(f);
        return -1;
    }
    fread(buf, 1, fsize, f);
    fclose(f);

    // 5. adaugam terminatorul de sir dupa ultimul caracter
    buf[fsize] = '\0';

    // 6. initializam lexerul cu bufferul
    initLexer(buf);

    // 7. apelam getNextToken() pana dam de END
    while (getNextToken() != END);

    // 8. afisam toti tokenii gasiti
    showTokens();

    // 9. eliberam memoria
    free(buf);

    return 0;
}