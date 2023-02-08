.intel_syntax noprefix
  .text
  .section .rodata
.LC0:
  .string ".intel_syntax noprefix\n"
.LC1:
  .string ".globl main\n"
.LC2:
  .string "main:\n"
.LC3:
  .string "  mov rax, %d\n"
.LC4:
  .string "  add rax, %d\n"
.LC5:
  .string "  sub rax, %d\n"
.LC6:
  .string "  ret\n"
  .text
.text
.globl parseInt
parseInt:
  push rbp
  mov rbp, rsp
  sub rsp, 32
  mov [rbp - 8], rdi
  mov rax, 1
  lea rax, [rbp - 24]
    push rax
  mov rax, 0
    pop rdi
    mov [rdi], eax
.Lbegin0:
  lea rax, [rbp - 8]
  mov rax,[rax]
  mov rax, [rax]
    push rax
    pop rdi
  mov rax, 0
 call isDigit
  cmp rax, 0
  je  .Lend0
  mov rax, 42
  lea rax, [rbp - 32]
    push rax
  lea rax, [rbp - 8]
  mov rax,[rax]
  mov rax, [rax]
    push rax
  mov rax, 48
    push rax
    pop rdi
    pop rax
    sub rax,rdi
    pop rdi
    mov [rdi], eax
  lea rax, [rbp - 24]
    push rax
  lea rax, [rbp - 24]
  mov eax,[rax]
    push rax
  mov rax, 10
    push rax
    pop rdi
    pop rax
    imul rax,rdi
    push rax
  lea rax, [rbp - 32]
  mov eax,[rax]
    push rax
    pop rdi
    pop rax
    add rax,rdi
    pop rdi
    mov [rdi], eax
  mov rax, 1
    push rax
  mov rax, 1
    push rax
    pop rdi
    pop rax
    imul rax,rdi
    push rax
  lea rax, [rbp - 8]
    mov rsi, rax
    mov rax, [rax]
    pop rdi
    add rax,rdi
    mov rdi, rsi
    mov [rdi], eax
    push rax
  mov rax, 1
    push rax
  mov rax, 1
    push rax
    pop rdi
    pop rax
    imul rax,rdi
    push rax
    pop rdi
    pop rax
    sub rax,rdi
  jmp  .Lbegin0
.Lend0:
  lea rax, [rbp - 24]
  mov eax,[rax]
  mov rsp, rbp
  pop rbp
  ret
  mov rax, 42
  mov rsp, rbp
  pop rbp
  ret
.globl intLength
intLength:
  push rbp
  mov rbp, rsp
  sub rsp, 24
  mov [rbp - 8], rdi
  mov rax, 1
  lea rax, [rbp - 24]
    push rax
  mov rax, 0
    pop rdi
    mov [rdi], eax
.Lbegin1:
  lea rax, [rbp - 8]
  mov rax,[rax]
  mov rax, [rax]
    push rax
    pop rdi
  mov rax, 0
 call isDigit
  cmp rax, 0
  je  .Lend1
  mov rax, 42
  mov rax, 1
    push rax
  lea rax, [rbp - 24]
    mov rsi, rax
    mov rax, [rax]
    pop rdi
    add rax,rdi
    mov rdi, rsi
    mov [rdi], eax
    push rax
  mov rax, 1
    push rax
    pop rdi
    pop rax
    sub rax,rdi
  mov rax, 1
    push rax
  mov rax, 1
    push rax
    pop rdi
    pop rax
    imul rax,rdi
    push rax
  lea rax, [rbp - 8]
    mov rsi, rax
    mov rax, [rax]
    pop rdi
    add rax,rdi
    mov rdi, rsi
    mov [rdi], eax
    push rax
  mov rax, 1
    push rax
  mov rax, 1
    push rax
    pop rdi
    pop rax
    imul rax,rdi
    push rax
    pop rdi
    pop rax
    sub rax,rdi
  jmp  .Lbegin1
.Lend1:
  lea rax, [rbp - 24]
  mov eax,[rax]
  mov rsp, rbp
  pop rbp
  ret
  mov rax, 42
  mov rsp, rbp
  pop rbp
  ret
.globl isDigit
isDigit:
  push rbp
  mov rbp, rsp
  sub rsp, 16
  mov [rbp - 8], rdi
  mov rax, 1
  lea rax, [rbp - 8]
  movzx ecx, BYTE PTR [rax]
  mov eax, ecx
    push rax
  mov rax, 48
    push rax
    pop rdi
    pop rax
  cmp rax, rdi
  setge al
  movzb rax, al
    test rax, rax
    je .Landfalse2
  mov rax, 57
    push rax
  lea rax, [rbp - 8]
  movzx ecx, BYTE PTR [rax]
  mov eax, ecx
    push rax
    pop rdi
    pop rax
  cmp rax, rdi
  setge al
  movzb rax, al
    test rax, rax
    je  .Landfalse2
    mov eax, 1
    jmp .Landend2
.Landfalse2:
    mov     eax, 0
.Landend2:
  mov rsp, rbp
  pop rbp
  ret
  mov rax, 42
  mov rsp, rbp
  pop rbp
  ret
.globl main
main:
  push rbp
  mov rbp, rsp
  sub rsp, 88
  mov [rbp - 8], rdi
  mov [rbp - 16], rsi
  mov rax, 1
  lea rax, [rbp - 8]
  mov eax,[rax]
    push rax
  mov rax, 2
    push rax
    pop rdi
    pop rax
  cmp rax, rdi
  setne al
  movzb rax, al
  cmp rax, 0
  je  .Lelse3
  mov rax, 42
  mov rax, 1
  mov rsp, rbp
  pop rbp
  ret
  jmp .Lend3
.Lelse3:
.Lend3:
  lea rax, [rbp - 40]
    push rax
  lea rax, [rbp - 16]
  mov rax,[rax]
    push rax
  mov rax, 8
    push rax
  mov rax, 1
    push rax
    pop rdi
    pop rax
    imul rax,rdi
    push rax
    pop rdi
    pop rax
    add rax,rdi
  mov rax, [rax]
    pop rdi
    mov [rdi], rax
  mov eax, OFFSET FLAT:.LC0
    push rax
    pop rdi
  mov rax, 0
 call printf
  mov eax, OFFSET FLAT:.LC1
    push rax
    pop rdi
  mov rax, 0
 call printf
  mov eax, OFFSET FLAT:.LC2
    push rax
    pop rdi
  mov rax, 0
 call printf
  lea rax, [rbp - 48]
    push rax
  lea rax, [rbp - 40]
  mov rax,[rax]
    push rax
    pop rdi
  mov rax, 0
 call parseInt
    pop rdi
    mov [rdi], eax
  lea rax, [rbp - 56]
    push rax
  lea rax, [rbp - 40]
  mov rax,[rax]
    push rax
    pop rdi
  mov rax, 0
 call intLength
    pop rdi
    mov [rdi], eax
  mov rax, 1
    push rax
  lea rax, [rbp - 56]
  mov eax,[rax]
    push rax
    pop rdi
    pop rax
    imul rax,rdi
    push rax
  lea rax, [rbp - 40]
    mov rsi, rax
    mov rax, [rax]
    pop rdi
    add rax,rdi
    mov rdi, rsi
    mov [rdi], eax
  mov eax, OFFSET FLAT:.LC3
    push rax
  lea rax, [rbp - 48]
  mov eax,[rax]
    push rax
    pop rsi
    pop rdi
  mov rax, 0
 call printf
.Lbegin4:
  lea rax, [rbp - 40]
  mov rax,[rax]
  mov rax, [rax]
  cmp rax, 0
  je  .Lend4
  mov rax, 42
  lea rax, [rbp - 40]
  mov rax,[rax]
  mov rax, [rax]
    push rax
  mov rax, 43
    push rax
    pop rdi
    pop rax
  cmp rax, rdi
  sete al
  movzb rax, al
  cmp rax, 0
  je  .Lelse5
  mov rax, 42
  mov rax, 1
    push rax
  mov rax, 1
    push rax
    pop rdi
    pop rax
    imul rax,rdi
    push rax
  lea rax, [rbp - 40]
    mov rsi, rax
    mov rax, [rax]
    pop rdi
    add rax,rdi
    mov rdi, rsi
    mov [rdi], eax
    push rax
  mov rax, 1
    push rax
  mov rax, 1
    push rax
    pop rdi
    pop rax
    imul rax,rdi
    push rax
    pop rdi
    pop rax
    sub rax,rdi
  lea rax, [rbp - 64]
    push rax
  lea rax, [rbp - 40]
  mov rax,[rax]
    push rax
    pop rdi
  mov rax, 0
 call parseInt
    pop rdi
    mov [rdi], eax
  lea rax, [rbp - 72]
    push rax
  lea rax, [rbp - 40]
  mov rax,[rax]
    push rax
    pop rdi
  mov rax, 0
 call intLength
    pop rdi
    mov [rdi], eax
  mov rax, 1
    push rax
  lea rax, [rbp - 72]
  mov eax,[rax]
    push rax
    pop rdi
    pop rax
    imul rax,rdi
    push rax
  lea rax, [rbp - 40]
    mov rsi, rax
    mov rax, [rax]
    pop rdi
    add rax,rdi
    mov rdi, rsi
    mov [rdi], eax
  mov eax, OFFSET FLAT:.LC4
    push rax
  lea rax, [rbp - 64]
  mov eax,[rax]
    push rax
    pop rsi
    pop rdi
  mov rax, 0
 call printf
  jmp .Lend5
.Lelse5:
  lea rax, [rbp - 40]
  mov rax,[rax]
  mov rax, [rax]
    push rax
  mov rax, 45
    push rax
    pop rdi
    pop rax
  cmp rax, rdi
  sete al
  movzb rax, al
  cmp rax, 0
  je  .Lelse6
  mov rax, 42
  mov rax, 1
    push rax
  mov rax, 1
    push rax
    pop rdi
    pop rax
    imul rax,rdi
    push rax
  lea rax, [rbp - 40]
    mov rsi, rax
    mov rax, [rax]
    pop rdi
    add rax,rdi
    mov rdi, rsi
    mov [rdi], eax
    push rax
  mov rax, 1
    push rax
  mov rax, 1
    push rax
    pop rdi
    pop rax
    imul rax,rdi
    push rax
    pop rdi
    pop rax
    sub rax,rdi
  lea rax, [rbp - 64]
    push rax
  lea rax, [rbp - 40]
  mov rax,[rax]
    push rax
    pop rdi
  mov rax, 0
 call parseInt
    pop rdi
    mov [rdi], eax
  lea rax, [rbp - 72]
    push rax
  lea rax, [rbp - 40]
  mov rax,[rax]
    push rax
    pop rdi
  mov rax, 0
 call intLength
    pop rdi
    mov [rdi], eax
  mov rax, 1
    push rax
  lea rax, [rbp - 72]
  mov eax,[rax]
    push rax
    pop rdi
    pop rax
    imul rax,rdi
    push rax
  lea rax, [rbp - 40]
    mov rsi, rax
    mov rax, [rax]
    pop rdi
    add rax,rdi
    mov rdi, rsi
    mov [rdi], eax
  mov eax, OFFSET FLAT:.LC5
    push rax
  lea rax, [rbp - 64]
  mov eax,[rax]
    push rax
    pop rsi
    pop rdi
  mov rax, 0
 call printf
  jmp .Lend6
.Lelse6:
  mov rax, 42
  mov rax, 2
  mov rsp, rbp
  pop rbp
  ret
.Lend6:
.Lend5:
  jmp  .Lbegin4
.Lend4:
  mov eax, OFFSET FLAT:.LC6
    push rax
    pop rdi
  mov rax, 0
 call printf
  mov rax, 0
  mov rsp, rbp
  pop rbp
  ret
  mov rax, 42
  mov rsp, rbp
  pop rbp
  ret
