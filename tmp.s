.intel_syntax noprefix
  .text
  .section .rodata
  .text
.text
.globl main
main:
  push rbp
  mov rbp, rsp
  sub rsp, 0
  mov rax, 1
  mov rax, 8
    push rax
  mov rax, 7
    push rax
  mov rax, 2
    push rax
    pop rdi
    pop rax
  cmp rax, rdi
  setne al
  movzb rax, al
    push rax
    pop rdi
    pop rax
    add rax,rdi
  mov rsp, rbp
  pop rbp
  ret
