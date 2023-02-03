.intel_syntax noprefix
  .text
  .section .rodata
  .text
.text
.globl main
main:
  push rbp
  mov rbp, rsp
  sub rsp, 8
  mov rax, 1
  lea rax, [rbp - 8]
    push rax
  mov rax, 0
    pop rdi
    mov [rdi], eax
  mov rax, 0
    push rax
  lea rax, [rbp - 8]
  mov rax,[rax]
    push rax
    pop rdi
    pop rax
  cmp rax, rdi
  setne al
  movzb rax, al
  mov rsp, rbp
  pop rbp
  ret
