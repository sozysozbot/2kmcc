#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

LVar *locals;

int tokenize(char *str) {
    int token_index = 0;
    for (int i = 0; str[i];) {
        char c = str[i];
        char *ptr = str + i;
        if (strncmp(ptr, "return", 6) == 0 && !is_alnum(ptr[6])) {
            Token token = {aaa('r', 'e', 't'), 0, NULL};
            tokens[token_index] = token;
            token_index++;
            i += 6;
            continue;
        }
        if (strncmp(ptr, "if", 2) == 0 && !is_alnum(ptr[2])) {
            Token token = {aa('i', 'f'), 0, NULL};
            tokens[token_index] = token;
            token_index++;
            i += 2;
            continue;
        }
        if (strncmp(ptr, "while", 5) == 0 && !is_alnum(ptr[5])) {
            Token token = {aaaa('w', 'h', 'i', 'l'), 0, NULL};
            tokens[token_index] = token;
            token_index++;
            i += 5;
            continue;
        }
        if (strncmp(ptr, "else", 4) == 0 && !is_alnum(ptr[4])) {
            Token token = {aaaa('e', 'l', 's', 'e'), 0, NULL};
            tokens[token_index] = token;
            token_index++;
            i += 4;
            continue;
        }
        if (strncmp(ptr, "for", 3) == 0 && !is_alnum(ptr[3])) {
            Token token = {aaa('f', 'o', 'r'), 0, NULL};
            tokens[token_index] = token;
            token_index++;
            i += 3;
            continue;
        }
        if (c == '+') {
            Token token = {'+', 0, NULL};
            tokens[token_index] = token;
            token_index++;
            i++;
        } else if (c == ';' || c == '(' || c == ')' || c == '{' || c == '}') {
            tokens[token_index].kind = c;
            token_index++;
            i++;
        } else if (c == '-') {
            Token token = {'-', 0, NULL};
            tokens[token_index] = token;
            token_index++;
            i++;
        } else if (c == '*') {
            Token token = {'*', 0, NULL};
            tokens[token_index] = token;
            token_index++;
            i++;
        } else if (c == '/') {
            Token token = {'/', 0, NULL};
            tokens[token_index] = token;
            token_index++;
            i++;
        } else if (c == '>') {
            i++;
            char c = str[i];
            if (c != '=') {
                Token token = {'>', 0, NULL};
                tokens[token_index] = token;
                token_index++;
            } else {
                i++;
                Token token = {aa('>', '='), 0, NULL};
                tokens[token_index] = token;
                token_index++;
            }
        } else if (c == '<') {
            i++;
            char c = str[i];
            if (c != '=') {
                Token token = {'<', 0, NULL};
                tokens[token_index] = token;
                token_index++;
            } else {
                i++;
                Token token = {aa('<', '='), 0, NULL};
                tokens[token_index] = token;
                token_index++;
            }
        } else if (c == '=') {
            i++;
            char c = str[i];
            if (c != '=') {
                Token token = {'=', 0, NULL};
                tokens[token_index] = token;
                token_index++;
            } else {
                i++;
                Token token = {aa('=', '='), 0, NULL};
                tokens[token_index] = token;
                token_index++;
            }
        } else if (c == '!') {
            i++;
            char c = str[i];
            if (c != '=') {
                fprintf(stderr, "%s: unknown token !%c(%d)\n", __FUNCTION__, c, c);
                return -1;
            }
            i++;
            Token token = {aa('!', '='), 0, NULL};
            tokens[token_index] = token;
            token_index++;
        } else if ('0' <= c && c <= '9') {
            int parsednum = parseInt(&str[i]);
            int parsedlength = intLength(&str[i]);
            i += parsedlength;
            Token token = {aaa('n', 'u', 'm'), parsednum, NULL};
            tokens[token_index] = token;
            token_index++;
        } else if (c == ' ') {
            i++;
        } else if ('a' <= c && c <= 'z') {
            char *start = str + i;
            i++;
            for (;;) {
                char c1 = str[i];
                if ('a' <= c1 && c1 <= 'z') {
                    i++;
                } else {
                    break;
                }
            }
            char *end = str + i;

            int length = end - start;
            char *name = malloc(length + 1);
            memcpy(name, start, length);
            name[length] = '\0';
            if (!findLVar(name)) {
                insertLVar(name);
            }
            Token token = {aaaa('i', 'd', 'n', 't'), 0, NULL};
            token.identifier_name = name;
            tokens[token_index] = token;
            token_index++;
        } else {
            fprintf(stderr, "%s: unknown character %c(%d)\n", __FUNCTION__, c, c);
            return -1;
        }
    }
    return token_index;
}
int is_alnum(char c) {
    return ('a' <= c && c <= 'z') ||
           ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') ||
           (c == '_');
}

int parseInt(char *str) {
    int result = 0;
    while (1) {
        if (!isDigit(*str)) {
            break;
        } else {
            int digit = *str - '0';
            result = result * 10 + digit;
        }
        str++;
    }
    return result;
}
int intLength(char *str) {
    int length = 0;
    while (1) {
        if (!isDigit(*str)) {
            break;
        }
        length++;
        str++;
    }
    return length;
}
int isDigit(char c) {
    return '0' <= c && c <= '9';
}

LVar *findLVar(char *name) {
    LVar *local = locals;
    if (!local) {
        return NULL;
    }
    while (local) {
        if (!strcmp(name, local->name)) {
            return local;
        }
        local = local->next;
    }
    return NULL;
}

LVar *insertLVar(char *name) {
    LVar *newlocal = calloc(1, sizeof(LVar));
    LVar *last = lastLVar();
    newlocal->len = strlen(name);
    newlocal->name = name;
    if (!last) {
        newlocal->offset = 0;
    } else {
        newlocal->offset = last->offset + 8;  // offset+last size
    }
    newlocal->next = NULL;

    if (!last) {
        locals = newlocal;
    } else {
        last->next = newlocal;
    }
    return newlocal;
}

LVar *lastLVar() {
    LVar *local = locals;
    if (!local) {
        return NULL;
    }
    while (1) {
        if (!local->next) {
            return local;
        }
        local = local->next;
    }
}