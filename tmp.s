.intel_syntax noprefix
  .text
  .section .rodata
.LC0:
  .string "a=%d, b=%d; "
.LC1:
  .string "res=%d"
  .text
.text
.globl enum2
enum2:
  push rbp
  mov rbp, rsp
  sub rsp, 32
# newlocal->offset_from_rbp = sz;
# sz: 8
  mov [rbp - 8], rdi
# newlocal->offset_from_rbp = last->offset_from_rbp + sz;
# last->offset_from_rbp: 8
# sz: 8
  mov [rbp - 16], rsi
# newlocal->offset_from_rbp = last->offset_from_rbp + sz;
# last->offset_from_rbp: 16
# sz: 8
# newlocal->offset_from_rbp = last->offset_from_rbp + sz;
# last->offset_from_rbp: 24
# sz: 8
  mov rax, 1
  mov eax, OFFSET FLAT:.LC0
    push rax
  lea rax, [rbp - 8]
  mov eax,[rax]
    push rax
  lea rax, [rbp - 16]
  mov eax,[rax]
    push rax
    pop rdx
    pop rsi
    pop rdi
  mov rax, 0
 call printf
  lea rax, [rbp - 8]
  mov eax,[rax]
    push rax
  lea rax, [rbp - 16]
  mov eax,[rax]
    push rax
  mov rax, 10
    push rax
    pop rdi
    pop rax
    imul rax,rdi
    push rax
    pop rdi
    pop rax
    add rax,rdi
  mov rsp, rbp
  pop rbp
  ret
.globl enum3
enum3:
  push rbp
  mov rbp, rsp
  sub rsp, 48
# newlocal->offset_from_rbp = last->offset_from_rbp + sz;
# last->offset_from_rbp: 32
# sz: 8
  mov [rbp - 8], rdi
# newlocal->offset_from_rbp = last->offset_from_rbp + sz;
# last->offset_from_rbp: 40
# sz: 8
  mov [rbp - 16], rsi
# newlocal->offset_from_rbp = last->offset_from_rbp + sz;
# last->offset_from_rbp: 48
# sz: 8
  mov [rbp - 56], rdx
# newlocal->offset_from_rbp = last->offset_from_rbp + sz;
# last->offset_from_rbp: 56
# sz: 8
# newlocal->offset_from_rbp = last->offset_from_rbp + sz;
# last->offset_from_rbp: 64
# sz: 8
# newlocal->offset_from_rbp = last->offset_from_rbp + sz;
# last->offset_from_rbp: 72
# sz: 8
  mov rax, 1
  lea rax, [rbp - 8]
  mov eax,[rax]
    push rax
  lea rax, [rbp - 16]
  mov eax,[rax]
    push rax
  lea rax, [rbp - 56]
  mov eax,[rax]
    push rax
  mov rax, 10
    push rax
    pop rdi
    pop rax
    imul rax,rdi
    push rax
    pop rdi
    pop rax
    add rax,rdi
    push rax
    pop rsi
    pop rdi
  mov rax, 0
 call enum2
  mov rsp, rbp
  pop rbp
  ret
.globl main
main:
  push rbp
  mov rbp, rsp
  sub rsp, 0
  mov rax, 1
  mov eax, OFFSET FLAT:.LC1
    push rax
  mov rax, 1
    push rax
  mov rax, 2
    push rax
  mov rax, 3
    push rax
    pop rdx
    pop rsi
    pop rdi
  mov rax, 0
 call enum3
    push rax
    pop rsi
    pop rdi
  mov rax, 0
 call printf
  mov rax, 0
  mov rsp, rbp
  pop rbp
  ret
