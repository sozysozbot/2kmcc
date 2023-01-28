#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

int labelCounter = 0;

LVar *locals;

LVar *findLVar(char *name) {
    LVar *local = locals;
    if (!local) {
        return 0;
    }
    while (local) {
        if (!strcmp(name, local->name)) {
            return local;
        }
        local = local->next;
    }
    return 0;
}

LVar *insertLVar(char *name) {
    LVar *newlocal = calloc(1, sizeof(LVar));
    LVar *last = lastLVar();
    newlocal->name = name;
    if (!last) {
        newlocal->offset_from_rbp = 8;
    } else {
        newlocal->offset_from_rbp = last->offset_from_rbp + 8;  // offset+last size
    }
    newlocal->next = 0;

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
        return 0;
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
        printf("  sub rax, %d\n", local->offset_from_rbp);
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
    } else if (stmt->stmt_kind == enum3('R', 'E', 'T')) {
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
        if (stmt->third_child != 0) {
            CodegenStmt(stmt->third_child);
        }
        printf(".Lend%d:\n", label);
    } else if (stmt->stmt_kind == enum4('W', 'H', 'I', 'L')) {
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

const char *nth_arg_reg(int n) {
    return "rdi\0rsi\0rdx\0rcx\0r8 \0r9" + 4 * n;
}

void CodegenFunc(FuncDef *funcdef) {
    printf(".globl %s\n", funcdef->name);
    printf("%s:\n", funcdef->name);
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, 208\n");
    for (int i = 0; i < funcdef->param_len; i++) {
        char *param_name = funcdef->params[i];
        insertLVar(param_name);
        LVar *local = findLVar(param_name);
        printf("  mov rax, rbp\n");
        printf("  sub rax, %d\n", local->offset_from_rbp);
        printf("  mov [rax], %s\n", nth_arg_reg(i));
    }
    CodegenStmt(funcdef->content);
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
    } else if (expr->expr_kind == EK_UnaryOperator) {
        if (expr->op == '*') {
            EvaluateExprIntoRax(expr->first_child);
            printf("  mov rax, [rax]\n");
        } else if (expr->op == '&') {
            EvaluateLValueAddressIntoRax(expr->first_child);
        } else {
            fprintf(stderr, "Invalid unaryop kind:%d", expr->op);
            exit(1);
        }
    } else if (expr->expr_kind == EK_BinaryOperator) {
        if (expr->op == '=') {
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

            if (expr->op == '+') {
                printf("    add rax,rdi\n");
            } else if (expr->op == '-') {
                printf("    sub rax,rdi\n");

            } else if (expr->op == '*') {
                printf("    imul rax,rdi\n");
            } else if (expr->op == '/') {
                printf("  cqo\n");
                printf("  idiv rdi\n");
            } else if (expr->op == enum2('=', '=')) {
                printf("  cmp rax, rdi\n");
                printf("  sete al\n");
                printf("  movzb rax, al\n");
            } else if (expr->op == enum2('!', '=')) {
                printf("  cmp rax, rdi\n");
                printf("  setne al\n");
                printf("  movzb rax, al\n");
            } else if (expr->op == '>') {
                printf("  cmp rax, rdi\n");
                printf("  setg al\n");
                printf("  movzb rax, al\n");
            } else if (expr->op == enum2('>', '=')) {
                printf("  cmp rax, rdi\n");
                printf("  setge al\n");
                printf("  movzb rax, al\n");
            } else {
                fprintf(stderr, "Invalid binaryop kind:%d", expr->op);
                exit(1);
            }
        }
    } else {
        fprintf(stderr, "Invalid expr kind:%d", expr->expr_kind);
        exit(1);
    }
}
