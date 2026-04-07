#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

typedef struct _Token Token;

#define SAFEALLOC(var, Type) \
    if ((var = (Type*)malloc(sizeof(Type))) == NULL) err("not enough memory")

void err(const char *fmt, ...);
void tkerr(const Token *tk, const char *fmt, ...);

#endif