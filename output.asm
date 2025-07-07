; Transpiled Assembly Code
section .text
global _start
_start:
; Initialize Variable: global_var
; Number Literal: 100
  push 100
  pop rax
  mov [rel global_var], rax
; Expression Statement
; Identifier: println
  push qword [rel println]
; Expression Statement
; Identifier: global_var
  push qword [rel global_var]
; Initialize Variable: local_c
; Binary Expression
; Identifier: b
  push qword [rel b]
  pop rbx
  pop rax
  imul rax, rbx
  push rax
  pop rax
  mov [rel local_c], rax
; Return Statement
; Identifier: local_c
  push qword [rel local_c]
  pop rax
  mov rsp, rbp
  pop rbp
  ret
; Expression Statement
; Identifier: product
  push qword [rel product]
; Expression Statement
; Identifier: println
  push qword [rel println]
; Expression Statement
; Identifier: println
  push qword [rel println]
; Expression Statement
; Identifier: product
  push qword [rel product]
; Initialize Variable: char_A
; ASCII Literal: 65a
  push 65
  pop rax
  mov [rel char_A], rax
; Expression Statement
; Identifier: println
  push qword [rel println]
; Initialize Variable: counter
; Number Literal: 0
  push 0
  pop rax
  mov [rel counter], rax
; Expression Statement
; Identifier: println
  push qword [rel println]
; Expression Statement
; Identifier: counter
  push qword [rel counter]
; Expression Statement
; Identifier: println
  push qword [rel println]
; Expression Statement
; Identifier: counter
  push qword [rel counter]
; Expression Statement
; Identifier: println
  push qword [rel println]
  mov rax, 60  ; syscall number for exit
  xor rdi, rdi ; exit code 0
  syscall
