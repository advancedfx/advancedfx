DEFAULT REL
CPU X64
BITS 64

section .text

global afx_iface_passthrough
afx_iface_passthrough:
	mov rax, [rsp + 0x30]
	mov rcx, [rsp + 0x28]
	mov rsp, [rsp + 0x38]
	jmp [rax]
