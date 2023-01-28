#include <stdio.h>
#include <stdlib.h>

#include "9cc.h"

Token all_tokens[1000];
Token *tokens_end;
Token *tokens;

FuncDef *all_funcdefs[100];

int enum2(int a, int b) {
    return a + b * 256;
}

int enum3(int a, int b, int c) {
    return enum2(a, enum2(b, c));
}

int enum4(int a, int b, int c, int d) {
    return enum2(a, enum3(b, c, d));
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }

    char *p = argv[1];

    int tokens_length = tokenize(p);

    if (tokens_length == 0) {
        fprintf(stderr, "No token found");
        return 1;
    }

    tokens = all_tokens;
    tokens_end = all_tokens + tokens_length;

    parseProgram();

    printf(".intel_syntax noprefix\n");
    for (int i = 0; all_funcdefs[i]; i++) {
        FuncDef *funcdef = all_funcdefs[i];
        CodegenFunc(funcdef);
    }
    return 0;
}
