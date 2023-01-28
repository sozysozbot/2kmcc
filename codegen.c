#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "9cc.h"

int labelCounter = 0;

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

void EvaluateLValueAddressIntoRax(Expr *expr) {
    if (expr->expr_kind == EK_Identifier) {
        if (!findLVar(expr->name)) {
            insertLVar(expr->name);
        }
        LVar *local = findLVar(expr->name);
        printf("  mov rax, rbp\n");
        printf("  sub rax, %d\n", local->offset);
    } else {
        fprintf(stderr, "not lvalue");
        exit(1);
    }
}

void CodegenStmt(Stmt *stmt) {
    if (stmt->stmt_kind == enum4('e', 'x', 'p', 'r')) {
        EvaluateExprIntoRax(stmt->expr);
    } else if (stmt->stmt_kind == enum4('n', 'e', 'x', 't')) {
        CodegenStmt(stmt->first_child);
        CodegenStmt(stmt->second_child);
    } else if (stmt->stmt_kind == enum3('r', 'e', 't')) {
        EvaluateExprIntoRax(stmt->expr);
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
    } else if (stmt->stmt_kind == enum2('i', 'f')) {
        int label = ++labelCounter;

        EvaluateExprIntoRax(stmt->expr);
        printf("  cmp rax, 0\n");
        printf("  je  .Lelse%d\n", label);
        CodegenStmt(stmt->second_child);
        printf("  jmp .Lend%d\n", label);
        printf(".Lelse%d:\n", label);
        if (stmt->third_child != NULL) {
            CodegenStmt(stmt->third_child);
        }
        printf(".Lend%d:\n", label);
    } else if (stmt->stmt_kind == enum4('w', 'h', 'i', 'l')) {
        int label = ++labelCounter;

        printf(".Lbegin%d:\n", label);
        EvaluateExprIntoRax(stmt->expr);
        printf("  cmp rax, 0\n");
        printf("  je  .Lend%d\n", label);
        CodegenStmt(stmt->second_child);
        printf("  jmp  .Lbegin%d\n", label);
        printf(".Lend%d:\n", label);
    } else if (stmt->stmt_kind == enum3('f', 'o', 'r')) {
        int label = ++labelCounter;

        if (stmt->expr) {
            EvaluateExprIntoRax(stmt->expr);
        }
        printf(".Lbegin%d:\n", label);
        if (stmt->expr1) {
            EvaluateExprIntoRax(stmt->expr1);
        } else {
            printf("  mov rax, 1\n");
        }
        printf("  cmp rax, 0\n");
        printf("  je  .Lend%d\n", label);
        CodegenStmt(stmt->second_child);
        if (stmt->expr2) {
            EvaluateExprIntoRax(stmt->expr2);
        }
        printf("  jmp  .Lbegin%d\n", label);
        printf(".Lend%d:\n", label);
    }
}

void CodegenFunc(FuncDef *funcdef) {
    printf(".globl %s\n", funcdef->name);
    printf("%s:\n", funcdef->name);
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, 208\n");
    CodegenStmt(funcdef->content);
}

const char *nth_arg_reg(int n) {
    return "rdi\0rsi\0rdx\0rcx\0r8 \0r9" + 4 * n;
}

void EvaluateExprIntoRax(Expr *expr) {
    if (expr->expr_kind == EK_Identifier) {
        EvaluateLValueAddressIntoRax(expr);
        printf("  mov rax,[rax]\n");
        return;
    } else if (expr->expr_kind == EK_Call) {
        for (int i = 0; i < expr->func_arg_len; i++) {
            EvaluateExprIntoRax(expr->func_args[i]);
            printf("    push rax\n");
        }
        for (int i = expr->func_arg_len - 1; i >= 0; i--) {
            printf("    pop %s\n", nth_arg_reg(i));
        }
        printf(" call %s\n", expr->name);
        return;
    } else if (expr->expr_kind == EK_Number) {
        printf("  mov rax, %d\n", expr->value);
        return;
    } else if (expr->expr_kind == EK_Operator) {
        if (expr->binary_op == '=') {
            EvaluateLValueAddressIntoRax(expr->first_child);
            printf("    push rax\n");
            EvaluateExprIntoRax(expr->second_child);
            printf("    push rax\n");
            printf("    pop rdi\n");
            printf("    pop rax\n");
            printf("    mov [rax], rdi\n");
        } else {
            EvaluateExprIntoRax(expr->first_child);
            printf("    push rax\n");
            EvaluateExprIntoRax(expr->second_child);
            printf("    push rax\n");
            printf("    pop rdi\n");
            printf("    pop rax\n");

            if (expr->binary_op == '+') {
                printf("    add rax,rdi\n");
            } else if (expr->binary_op == '-') {
                printf("    sub rax,rdi\n");

            } else if (expr->binary_op == '*') {
                printf("    imul rax,rdi\n");
            } else if (expr->binary_op == '/') {
                printf("  cqo\n");
                printf("  idiv rdi\n");
            } else if (expr->binary_op == enum2('=', '=')) {
                printf("  cmp rax, rdi\n");
                printf("  sete al\n");
                printf("  movzb rax, al\n");
            } else if (expr->binary_op == enum2('!', '=')) {
                printf("  cmp rax, rdi\n");
                printf("  setne al\n");
                printf("  movzb rax, al\n");
            } else if (expr->binary_op == '>') {
                printf("  cmp rax, rdi\n");
                printf("  setg al\n");
                printf("  movzb rax, al\n");
            } else if (expr->binary_op == enum2('>', '=')) {
                printf("  cmp rax, rdi\n");
                printf("  setge al\n");
                printf("  movzb rax, al\n");
            } else {
                fprintf(stderr, "Invalid binaryop kind:%d", expr->binary_op);
                exit(1);
            }
        }
    } else {
        fprintf(stderr, "Invalid expr kind:%d", expr->expr_kind);
        exit(1);
    }
}
