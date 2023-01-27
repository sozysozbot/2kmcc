#include <stdio.h>
#include <stdlib.h>

#include "9cc.h"

int labelCounter = 0;

void EvaluateLValueAddressIntoRax(Expr *expr) {
    if (expr->expr_kind == EK_Identifier) {
        LVar *local = findLVar(expr->name);
        printf("  mov rax, rbp\n");
        printf("  sub rax, %d\n", local->offset);
    } else {
        fprintf(stderr, "not lvalue");
        exit(1);
    }
}

void Codegen(Stmt *stmt) {
    if (stmt->stmt_kind == aaaa('e', 'x', 'p', 'r')) {
        EvaluateExprIntoRax(stmt->expr);
    } else if (stmt->stmt_kind == aaaa('n', 'e', 'x', 't')) {
        Codegen(stmt->first_child);
        Codegen(stmt->second_child);
    } else if (stmt->stmt_kind == aaa('r', 'e', 't')) {
        EvaluateExprIntoRax(stmt->expr);
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
    } else if (stmt->stmt_kind == aa('i', 'f')) {
        int label = ++labelCounter;

        EvaluateExprIntoRax(stmt->expr);
        printf("  cmp rax, 0\n");
        printf("  je  .Lelse%d\n", label);
        Codegen(stmt->second_child);
        printf("  jmp .Lend%d\n", label);
        printf(".Lelse%d:\n", label);
        if (stmt->third_child != NULL) {
            Codegen(stmt->third_child);
        }
        printf(".Lend%d:\n", label);
    } else if (stmt->stmt_kind == aaaa('w', 'h', 'i', 'l')) {
        int label = ++labelCounter;

        printf(".Lbegin%d:\n", label);
        EvaluateExprIntoRax(stmt->expr);
        printf("  cmp rax, 0\n");
        printf("  je  .Lend%d\n", label);
        Codegen(stmt->second_child);
        printf("  jmp  .Lbegin%d\n", label);
        printf(".Lend%d:\n", label);
    } else if (stmt->stmt_kind == aaa('f', 'o', 'r')) {
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
        Codegen(stmt->second_child);
        if (stmt->expr2) {
            EvaluateExprIntoRax(stmt->expr2);
        }
        printf("  jmp  .Lbegin%d\n", label);
        printf(".Lend%d:\n", label);
    }
}

void EvaluateExprIntoRax(Expr *expr) {
    if (expr->expr_kind == EK_Identifier) {
        EvaluateLValueAddressIntoRax(expr);
        printf("  mov rax,[rax]\n");
        return;
    } else if (expr->expr_kind == EK_Call) {
        EvaluateExprIntoRax(expr->first_child);
        printf("    push rax\n");

        // todo
        printf("    pop rdi\n");
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
            } else if (expr->binary_op == aa('=', '=')) {
                printf("  cmp rax, rdi\n");
                printf("  sete al\n");
                printf("  movzb rax, al\n");
            } else if (expr->binary_op == aa('!', '=')) {
                printf("  cmp rax, rdi\n");
                printf("  setne al\n");
                printf("  movzb rax, al\n");
            } else if (expr->binary_op == '>') {
                printf("  cmp rax, rdi\n");
                printf("  setg al\n");
                printf("  movzb rax, al\n");
            } else if (expr->binary_op == aa('>', '=')) {
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
