.intel_syntax noprefix
  .text
  .section .rodata
.LC0:
  .string "%d"
.LC1:
  .string "42"
  .text
.text
.globl isDigit
isDigit:
  push rbp
  mov rbp, rsp
  sub rsp, 16
# inserting a variable named `c` at offset 8
  mov [rbp - 8], rdi
  mov rax, 1
  lea rax, [rbp - 8] # rax = &c
  movzx ecx, BYTE PTR [rax]
  mov eax, ecx
  push rax
  mov rax, 48
  mov rdi, rax
  pop rax
  cmp rax, rdi
  setge al
  movzb rax, al
  test rax, rax
  je .Landfalse0
  mov rax, 57
  push rax
  lea rax, [rbp - 8] # rax = &c
  movzx ecx, BYTE PTR [rax]
  mov eax, ecx
  mov rdi, rax
  pop rax
  cmp rax, rdi
  setge al
  movzb rax, al
  test rax, rax
  je  .Landfalse0
  mov eax, 1
  jmp .Landend0
.Landfalse0:
  mov     eax, 0
.Landend0:
  mov rsp, rbp
  pop rbp
  ret
  mov rax, 42
  mov rsp, rbp
  pop rbp
  ret
.globl parseInt
parseInt:
  push rbp
  mov rbp, rsp
  sub rsp, 32
# inserting a variable named `str` at offset 8
  mov [rbp - 8], rdi
# inserting a variable named `result` at offset 16
# inserting a variable named `digit` at offset 24
  mov rax, 1
  lea rax, [rbp - 16] # rax = &result
  push rax
  mov rax, 0
  pop rdi
  mov [rdi], eax
.Lbegin1:
  lea rax, [rbp - 8] # rax = &str
  mov rax,[rax]
  mov rax, [rax]
  push rax
  pop rdi
  mov rax, 0
  call isDigit
  cmp rax, 0
  je  .Lend1
  mov rax, 42
  lea rax, [rbp - 24] # rax = &digit
  push rax
  lea rax, [rbp - 8] # rax = &str
  mov rax,[rax]
  mov rax, [rax]
  push rax
  mov rax, 48
  mov rdi, rax
  pop rax
  sub rax,rdi
  pop rdi
  mov [rdi], eax
  lea rax, [rbp - 16] # rax = &result
  push rax
  lea rax, [rbp - 16] # rax = &result
  mov eax,[rax]
  push rax
  mov rax, 10
  mov rdi, rax
  pop rax
  imul rax,rdi
  push rax
  lea rax, [rbp - 24] # rax = &digit
  mov eax,[rax]
  mov rdi, rax
  pop rax
  add rax,rdi
  pop rdi
  mov [rdi], eax
  mov rax, 1
  push rax
  mov rax, 1
  mov rdi, rax
  pop rax
  imul rax,rdi
  push rax
  lea rax, [rbp - 8] # rax = &str
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
  mov rdi, rax
  pop rax
  imul rax,rdi
  mov rdi, rax
  pop rax
  sub rax,rdi
  jmp  .Lbegin1
.Lend1:
  lea rax, [rbp - 16] # rax = &result
  mov eax,[rax]
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
  sub rsp, 0
  mov rax, 1
  mov eax, OFFSET FLAT:.LC0
  push rax
  mov eax, OFFSET FLAT:.LC1
  push rax
  pop rdi
  mov rax, 0
  call parseInt
  push rax
  pop rsi
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
