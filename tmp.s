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
# inserting a variable named `str` at offset 8
  mov [rbp - 8], rdi
# inserting a variable named `str` at offset 16
# inserting a variable named `result` at offset 24
# inserting a variable named `digit` at offset 32
  mov rax, 1
  lea rax, [rbp - 24] # result
    push rax
  mov rax, 0
    pop rdi
    mov [rdi], eax
.Lbegin0:
  lea rax, [rbp - 8] # str
  mov rax,[rax]
  mov rax, [rax]
    push rax
    pop rdi
  mov rax, 0
 call isDigit
  cmp rax, 0
  je  .Lend0
  mov rax, 42
  lea rax, [rbp - 32] # digit
    push rax
  lea rax, [rbp - 8] # str
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
  lea rax, [rbp - 24] # result
    push rax
  lea rax, [rbp - 24] # result
  mov eax,[rax]
    push rax
  mov rax, 10
    push rax
    pop rdi
    pop rax
    imul rax,rdi
    push rax
  lea rax, [rbp - 32] # digit
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
  lea rax, [rbp - 8] # str
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
  lea rax, [rbp - 24] # result
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
# inserting a variable named `str` at offset 8
  mov [rbp - 8], rdi
# inserting a variable named `str` at offset 16
# inserting a variable named `length` at offset 24
  mov rax, 1
  lea rax, [rbp - 24] # length
    push rax
  mov rax, 0
    pop rdi
    mov [rdi], eax
.Lbegin1:
  lea rax, [rbp - 8] # str
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
  lea rax, [rbp - 24] # length
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
  lea rax, [rbp - 8] # str
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
  lea rax, [rbp - 24] # length
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
# inserting a variable named `c` at offset 8
  mov [rbp - 8], rdi
# inserting a variable named `c` at offset 16
  mov rax, 1
  lea rax, [rbp - 8] # c
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
  lea rax, [rbp - 8] # c
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
  sub rsp, 56
# inserting a variable named `p` at offset 8
# inserting a variable named `parsednum_` at offset 16
# inserting a variable named `parsedlength_` at offset 24
# inserting a variable named `parsednum` at offset 32
# inserting a variable named `parsedlength` at offset 40
# inserting a variable named `parsednum2` at offset 48
# inserting a variable named `parsedlength2` at offset 56
  mov rax, 1
  lea rax, [rbp - 8] # p
    push rax
  mov rax, 3
    push rax
  mov rax, 1
    push rax
    pop rsi
    pop rdi
  mov rax, 0
 call calloc
    pop rdi
    mov [rdi], rax
  lea rax, [rbp - 8] # p
  mov rax,[rax]
    push rax
  mov rax, 1
    push rax
  mov rax, 0
    push rax
    pop rdi
    pop rax
    imul rax,rdi
    push rax
    pop rdi
    pop rax
    add rax,rdi
    push rax
  mov rax, 52
    pop rdi
    mov ecx, eax
    mov [rdi], cl
  lea rax, [rbp - 8] # p
  mov rax,[rax]
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
    add rax,rdi
    push rax
  mov rax, 50
    pop rdi
    mov ecx, eax
    mov [rdi], cl
  lea rax, [rbp - 8] # p
  mov rax,[rax]
    push rax
  mov rax, 1
    push rax
  mov rax, 2
    push rax
    pop rdi
    pop rax
    imul rax,rdi
    push rax
    pop rdi
    pop rax
    add rax,rdi
    push rax
  mov rax, 0
    pop rdi
    mov ecx, eax
    mov [rdi], cl
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
  lea rax, [rbp - 16] # parsednum_
    push rax
  lea rax, [rbp - 8] # p
  mov rax,[rax]
    push rax
    pop rdi
  mov rax, 0
 call parseInt
    pop rdi
    mov [rdi], eax
  lea rax, [rbp - 24] # parsedlength_
    push rax
  lea rax, [rbp - 8] # p
  mov rax,[rax]
    push rax
    pop rdi
  mov rax, 0
 call intLength
    pop rdi
    mov [rdi], eax
  mov rax, 1
    push rax
  lea rax, [rbp - 24] # parsedlength_
  mov eax,[rax]
    push rax
    pop rdi
    pop rax
    imul rax,rdi
    push rax
  lea rax, [rbp - 8] # p
    mov rsi, rax
    mov rax, [rax]
    pop rdi
    add rax,rdi
    mov rdi, rsi
    mov [rdi], eax
  mov eax, OFFSET FLAT:.LC3
    push rax
  lea rax, [rbp - 16] # parsednum_
  mov eax,[rax]
    push rax
    pop rsi
    pop rdi
  mov rax, 0
 call printf
.Lbegin3:
  lea rax, [rbp - 8] # p
  mov rax,[rax]
  mov rax, [rax]
  cmp rax, 0
  je  .Lend3
  mov rax, 42
  lea rax, [rbp - 8] # p
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
  je  .Lelse4
  mov rax, 42
  mov rax, 1
    push rax
  mov rax, 1
    push rax
    pop rdi
    pop rax
    imul rax,rdi
    push rax
  lea rax, [rbp - 8] # p
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
  lea rax, [rbp - 32] # parsednum
    push rax
  lea rax, [rbp - 8] # p
  mov rax,[rax]
    push rax
    pop rdi
  mov rax, 0
 call parseInt
    pop rdi
    mov [rdi], eax
  lea rax, [rbp - 40] # parsedlength
    push rax
  lea rax, [rbp - 8] # p
  mov rax,[rax]
    push rax
    pop rdi
  mov rax, 0
 call intLength
    pop rdi
    mov [rdi], eax
  mov rax, 1
    push rax
  lea rax, [rbp - 40] # parsedlength
  mov eax,[rax]
    push rax
    pop rdi
    pop rax
    imul rax,rdi
    push rax
  lea rax, [rbp - 8] # p
    mov rsi, rax
    mov rax, [rax]
    pop rdi
    add rax,rdi
    mov rdi, rsi
    mov [rdi], eax
  mov eax, OFFSET FLAT:.LC4
    push rax
  lea rax, [rbp - 32] # parsednum
  mov eax,[rax]
    push rax
    pop rsi
    pop rdi
  mov rax, 0
 call printf
  jmp .Lend4
.Lelse4:
  lea rax, [rbp - 8] # p
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
  lea rax, [rbp - 8] # p
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
  lea rax, [rbp - 48] # parsednum2
    push rax
  lea rax, [rbp - 8] # p
  mov rax,[rax]
    push rax
    pop rdi
  mov rax, 0
 call parseInt
    pop rdi
    mov [rdi], eax
  lea rax, [rbp - 56] # parsedlength2
    push rax
  lea rax, [rbp - 8] # p
  mov rax,[rax]
    push rax
    pop rdi
  mov rax, 0
 call intLength
    pop rdi
    mov [rdi], eax
  mov rax, 1
    push rax
  lea rax, [rbp - 56] # parsedlength2
  mov eax,[rax]
    push rax
    pop rdi
    pop rax
    imul rax,rdi
    push rax
  lea rax, [rbp - 8] # p
    mov rsi, rax
    mov rax, [rax]
    pop rdi
    add rax,rdi
    mov rdi, rsi
    mov [rdi], eax
  mov eax, OFFSET FLAT:.LC5
    push rax
  lea rax, [rbp - 48] # parsednum2
  mov eax,[rax]
    push rax
    pop rsi
    pop rdi
  mov rax, 0
 call printf
  jmp .Lend5
.Lelse5:
  mov rax, 42
  mov rax, 2
  mov rsp, rbp
  pop rbp
  ret
.Lend5:
.Lend4:
  jmp  .Lbegin3
.Lend3:
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
