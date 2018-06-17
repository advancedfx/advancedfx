; This hook is a bit complicated, because we cannot simply relay on
; SetDllDirectoryW to work, since this won't work well with UCRT DLL function
; forwarders:
; Basically parts of the UCRT seem to completely ignore the alternate DLL
; search order.
; Thus:
; - Instead we change the Current Working Directory and
;   change it back afterwards.
; - Also we use LoadLibraryExW with LOAD_WITH_ALTERED_SEARCH_PATH,
;   to get the best priority for at least the other DLLs.

DEFAULT REL
CPU X64
BITS 64

jmp labelBoot

align 32

labelArgs:
	argGetModuleHandleW: dq 0
	argGetProcAddress: dq 0
	argDllDirectory: dq 0 ; length must not exceed MAX_PATH-2 (or MAX_PATH-1 if last char is '\')
	argDllFilePath: dq 0
	datKernel32Dll: dw __utf16__('kernel32.dll'), 0
	datGetProcessHeap: db 'GetProcessHeap', 0
	datHeapAlloc: db 'HeapAlloc', 0
	datHeapFree: db 'HeapFree', 0
	datGetCurrentDirectoryW: db 'GetCurrentDirectoryW', 0
	datSetCurrentDirectoryW: db 'SetCurrentDirectoryW', 0
	datLoadLibraryExW: db 'LoadLibraryExW', 0

labelBoot:
	; (Initial rsp is 8 mod 16.)
	push rbp
	mov rbp, rsp
	sub rsp, 0x70 ; Final rsp shall be dividable by 16.
	; stack variables (pointed by rsp):
	; -0x8  : hKernel32Dll
	; -0x10 : pGetProcessHeap
	; -0x18 : pHeapAlloc
	; -0x20 : pHeapFree
	; -0x28 : pGetCurrentDirectoryW
	; -0x30 : pSetCurrentDirectoryW
	; -0x38 : pLoadLibraryExW
	; -0x40 : hHeap
	; -0x48 : nBufferLength
	; -0x4c : 
	; -0x50 : pMemory
	; ... Shadow space as required by windows
	
	; get hKernel32Dll:
	lea rcx, [datKernel32Dll]
	call [argGetModuleHandleW]
	cmp rax, 0
	jnz labelCont1
	mov eax, 1 ; error
	mov rsp, rbp
	pop rbp
	ret
labelCont1:
	mov [rbp -0x8], rax
	
	; get pGetProcessHeap:
	lea rdx, [datGetProcessHeap]
	mov rcx, [rbp -0x8] 
	call [argGetProcAddress]
	cmp rax, 0
	jnz labelCont2
	mov eax, 2 ; error
	mov rsp, rbp
	pop rbp
	ret
labelCont2:
	mov [rbp -0x10], rax
	
	; get pHeapAlloc:
	lea rdx, [datHeapAlloc]
	mov rcx, [rbp -0x8] 
	call [argGetProcAddress]
	cmp rax, 0
	jnz labelCont3
	mov eax, 3 ; error
	mov rsp, rbp
	pop rbp
	ret
labelCont3:
	mov [rbp -0x18], rax
	
	; get pHeapFree:
	lea rdx, [datHeapFree]
	mov rcx, [rbp -0x8]
	call [argGetProcAddress]
	cmp rax, 0
	jnz labelCont4
	mov eax, 4 ; error
	mov rsp, rbp
	pop rbp
	ret
labelCont4:
	mov [rbp -0x20], rax
	
	; get pGetCurrentDirectoryW:
	lea rdx, [datGetCurrentDirectoryW]
	mov rcx, [rbp -0x8]
	call [argGetProcAddress]
	cmp rax, 0
	jnz labelCont5
	mov eax, 5 ; error
	mov rsp, rbp
	pop rbp
	ret
labelCont5:
	mov [rbp -0x28], rax
	
	; get pSetCurrentDirectoryW:
	lea rdx, [datSetCurrentDirectoryW]
	mov rcx, [rbp -0x8]
	call [argGetProcAddress]
	cmp rax, 0
	jnz labelCont6
	mov eax, 6 ; error
	mov rsp, rbp
	pop rbp
	ret
labelCont6:
	mov [rbp -0x30], rax
	
	; get pLoadLibraryExW:
	lea rdx, [datLoadLibraryExW]
	mov rcx, [rbp -0x8]
	call [argGetProcAddress]
	cmp rax, 0
	jnz labelCont7
	mov eax, 7 ; error
	mov rsp, rbp
	pop rbp
	ret
labelCont7:
	mov [rbp -0x38], rax
	
	; get process heap:
	call [rbp -0x10]
	cmp rax, 0
	jnz labelCont8
	mov eax, 8 ; error
	mov rsp, rbp
	pop rbp
	ret
labelCont8:
	mov [rbp -0x40], rax
	
	; determine current directory length:
	mov rdx, 0
	mov ecx, 0
	call [rbp -0x28]
	cmp eax, 0
	jnz labelCont9
	mov eax, 9 ; error
	mov rsp, rbp
	pop rbp
	ret
labelCont9:
	shl eax, 1 ; utf16
	mov [rbp -0x48], eax
	
	; allocate memory:
	mov r8d, eax ; (zero extended)
	mov edx, 0
	mov rcx, [rbp -0x40]
	call [rbp -0x18]
	cmp rax, 0
	jnz labelCont10
	mov eax, 10 ; error
	mov rsp, rbp
	pop rbp
	ret
labelCont10:
	mov [rbp -0x50], rax
	
	; save directory to buffer:
	mov rdx, rax
	mov ecx, [rbp -0x48]
	call [rbp -0x28]
	inc eax ; terminating 0
	shl eax, 1 ; utf16
	cmp eax, [rbp -0x48]
	je labelCont11
	call labelSubFree
	mov eax, 11 ; error
	mov rsp, rbp
	pop rbp
	ret
labelCont11:
	
	; Set new directory
	mov rcx, [argDllDirectory]
	call [rbp -0x30]
	cmp eax, 0
	jnz labelCont12
	call labelSubFree
	mov eax, 12 ; error
	mov rsp, rbp
	pop rbp
	ret
labelCont12:
	
	; LoadLibraryExW:
	mov r8d, 0x00000008 ; LOAD_WITH_ALTERED_SEARCH_PATH
	mov rdx, 0
	mov rcx, [argDllFilePath]
	call [rbp -0x38]
	cmp rax, 0
	jnz labelCont13
	call labelSubRestoreCwd
	call labelSubFree
	mov eax, 13 ; error
	mov rsp, rbp
	pop rbp
	ret
labelCont13:
	
	; resore old Current Working Directory:
	call labelSubRestoreCwd
	cmp eax, 0
	jnz labelCont14
	call labelSubFree
	mov eax, 14 ; error
	mov rsp, rbp
	pop rbp
	ret
labelCont14:

	; Free memory:
	call labelSubFree
	cmp eax, 0
	jnz labelCont15
	mov eax, 15 ; error
	mov rsp, rbp
	pop rbp
	ret
labelCont15:
	
	; We are done:
	mov eax, 0 ; success
	mov rsp, rbp
	pop rbp
	ret
	
labelSubRestoreCwd:
	sub rsp, 0x28 ; align + shadow space
	mov rcx, [rbp -0x50]
	call [rbp -0x30]
	add rsp, 0x28 ; unwind :-)
	ret
	
labelSubFree:
	sub rsp, 0x28 ; align + shadow space
	mov r8, [rbp -0x50]
	mov edx, 0
	mov rcx, [rbp -0x40]
	call [rbp -0x20]
	add rsp, 0x28 ; unwind :-)
	ret

