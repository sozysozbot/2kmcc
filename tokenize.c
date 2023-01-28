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
            Token token = {enum3('r', 'e', 't'), 0, NULL};
            all_tokens[token_index] = token;
            token_index++;
            i += 6;
            continue;
        }
        if (strncmp(ptr, "if", 2) == 0 && !is_alnum(ptr[2])) {
            Token token = {enum2('i', 'f'), 0, NULL};
            all_tokens[token_index] = token;
            token_index++;
            i += 2;
            continue;
        }
        if (strncmp(ptr, "while", 5) == 0 && !is_alnum(ptr[5])) {
            Token token = {enum4('w', 'h', 'i', 'l'), 0, NULL};
            all_tokens[token_index] = token;
            token_index++;
            i += 5;
            continue;
        }
        if (strncmp(ptr, "else", 4) == 0 && !is_alnum(ptr[4])) {
            Token token = {enum4('e', 'l', 's', 'e'), 0, NULL};
            all_tokens[token_index] = token;
            token_index++;
            i += 4;
            continue;
        }
        if (strncmp(ptr, "for", 3) == 0 && !is_alnum(ptr[3])) {
            Token token = {enum3('f', 'o', 'r'), 0, NULL};
            all_tokens[token_index] = token;
            token_index++;
            i += 3;
            continue;
        }
        if (c == '+') {
            Token token = {'+', 0, NULL};
            all_tokens[token_index] = token;
            token_index++;
            i++;
        } else if (c == ';' || c == '(' || c == ')' || c == '{' || c == '}' || c == ',') {
            all_tokens[token_index].kind = c;
            token_index++;
            i++;
        } else if (c == '-') {
            Token token = {'-', 0, NULL};
            all_tokens[token_index] = token;
            token_index++;
            i++;
        } else if (c == '*') {
            Token token = {'*', 0, NULL};
            all_tokens[token_index] = token;
            token_index++;
            i++;
        } else if (c == '/') {
            Token token = {'/', 0, NULL};
            all_tokens[token_index] = token;
            token_index++;
            i++;
        } else if (c == '>') {
            i++;
            char c = str[i];
            if (c != '=') {
                Token token = {'>', 0, NULL};
                all_tokens[token_index] = token;
                token_index++;
            } else {
                i++;
                Token token = {enum2('>', '='), 0, NULL};
                all_tokens[token_index] = token;
                token_index++;
            }
        } else if (c == '<') {
            i++;
            char c = str[i];
            if (c != '=') {
                Token token = {'<', 0, NULL};
                all_tokens[token_index] = token;
                token_index++;
            } else {
                i++;
                Token token = {enum2('<', '='), 0, NULL};
                all_tokens[token_index] = token;
                token_index++;
            }
        } else if (c == '=') {
            i++;
            char c = str[i];
            if (c != '=') {
                Token token = {'=', 0, NULL};
                all_tokens[token_index] = token;
                token_index++;
            } else {
                i++;
                Token token = {enum2('=', '='), 0, NULL};
                all_tokens[token_index] = token;
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
            Token token = {enum2('!', '='), 0, NULL};
            all_tokens[token_index] = token;
            token_index++;
        } else if ('0' <= c && c <= '9') {
            int parsednum = parseInt(&str[i]);
            int parsedlength = intLength(&str[i]);
            i += parsedlength;
            Token token = {enum3('n', 'u', 'm'), parsednum, NULL};
            all_tokens[token_index] = token;
            token_index++;
        } else if (c == ' ') {
            i++;
        } else if (('a' <= c && c <= 'z') || c == '_') {
            char *start = str + i;
            i++;
            for (;;) {
                char c1 = str[i];
                if (('a' <= c1 && c1 <= 'z') || c1 == '_' || ('0' <= c1 && c1 <= '9')) {
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
            Token token = {enum4('i', 'd', 'n', 't'), 0, NULL};
            token.identifier_name = name;
            all_tokens[token_index] = token;
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
