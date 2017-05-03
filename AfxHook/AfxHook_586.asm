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

CPU 586 ; PENTIUM like
BITS 32 ; 32 BIT mode

jmp labelBoot

align 32

labelArgs:
	argGetModuleHandleW: dd 0
	argGetProcAddress: dd 0
	argDllDirectory: dd 0 ; length must not exceed MAX_PATH-2 (or MAX_PATH-1 if last char is '\')
	argDllFilePath: dd 0
	datKernel32Dll: dw __utf16__('kernel32.dll'), 0
	datGetProcessHeap: db 'GetProcessHeap', 0
	datHeapAlloc: db 'HeapAlloc', 0
	datHeapFree: db 'HeapFree', 0
	datGetCurrentDirectoryW: db 'GetCurrentDirectoryW', 0
	datSetCurrentDirectoryW: db 'SetCurrentDirectoryW', 0
	datLoadLibraryExW: db 'LoadLibraryExW', 0

labelOfs:
	mov ebx, [esp]
	sub ebx, labelGotOfs -labelArgs
	ret
	
labelBoot:
	push ebx
	
	call labelOfs
labelGotOfs:

	push ebp
	mov ebp, esp
	
	sub esp, 0x28
	; stack variables (pointed by ebp):
	; -0x4  : hKernel32Dll
	; -0x8  : pGetProcessHeap
	; -0x0c : pHeapAlloc
	; -0x10 : pHeapFree
	; -0x14 : pGetCurrentDirectoryW
	; -0x18 : pSetCurrentDirectoryW
	; -0x1c : pLoadLibraryExW
	; -0x20 : hHeap
	; -0x24 : nBufferLength
	; -0x28 : pMemory
	
	; get hKernel32Dll:
	mov eax, ebx
	add eax, datKernel32Dll -labelArgs
	push eax
	call [ebx +argGetModuleHandleW -labelArgs]
	cmp eax, 0
	jnz labelCont1
	mov eax, 1 ; error
	mov esp, ebp
	pop ebp
	pop ebx
	ret 4
labelCont1:
	mov [ebp -0x4], eax
	
	; get pGetProcessHeap:
	mov eax, ebx
	add eax, datGetProcessHeap -labelArgs
	push eax
	mov eax, [ebp -0x4]
	push eax
	call [ebx +argGetProcAddress -labelArgs]
	cmp eax, 0
	jnz labelCont2
	mov eax, 2 ; error
	mov esp, ebp
	pop ebp
	pop ebx
	ret 4
labelCont2:
	mov [ebp -0x8], eax
	
	; get pHeapAlloc:
	mov eax, ebx
	add eax, datHeapAlloc -labelArgs
	push eax
	mov eax, [ebp -0x4]
	push eax
	call [ebx +argGetProcAddress -labelArgs]
	cmp eax, 0
	jnz labelCont3
	mov eax, 3 ; error
	mov esp, ebp
	pop ebp
	pop ebx
	ret 4
labelCont3:
	mov [ebp -0x0c], eax
	
	; get pHeapFree:
	mov eax, ebx
	add eax, datHeapFree -labelArgs
	push eax
	mov eax, [ebp -0x4]
	push eax
	call [ebx +argGetProcAddress -labelArgs]
	cmp eax, 0
	jnz labelCont4
	mov eax, 4 ; error
	mov esp, ebp
	pop ebp
	pop ebx
	ret 4
labelCont4:
	mov [ebp -0x10], eax
	
	; get pGetCurrentDirectoryW:
	mov eax, ebx
	add eax, datGetCurrentDirectoryW -labelArgs
	push eax
	mov eax, [ebp -0x4]
	push eax
	call [ebx +argGetProcAddress -labelArgs]
	cmp eax, 0
	jnz labelCont5
	mov eax, 5 ; error
	mov esp, ebp
	pop ebp
	pop ebx
	ret 4
labelCont5:
	mov [ebp -0x14], eax
	
	; get pSetCurrentDirectoryW:
	mov eax, ebx
	add eax, datSetCurrentDirectoryW -labelArgs
	push eax
	mov eax, [ebp -0x4]
	push eax
	call [ebx +argGetProcAddress -labelArgs]
	cmp eax, 0
	jnz labelCont6
	mov eax, 6 ; error
	mov esp, ebp
	pop ebp
	pop ebx
	ret 4
labelCont6:
	mov [ebp -0x18], eax
	
	; get pLoadLibraryExW:
	mov eax, ebx
	add eax, datLoadLibraryExW -labelArgs
	push eax
	mov eax, [ebp -0x4]
	push eax
	call [ebx +argGetProcAddress -labelArgs]
	cmp eax, 0
	jnz labelCont7
	mov eax, 7 ; error
	mov esp, ebp
	pop ebp
	pop ebx
	ret 4
labelCont7:
	mov [ebp -0x1c], eax
	
	; get process heap:
	call [ebp -0x8]
	cmp eax, 0
	jnz labelCont8
	mov eax, 8 ; error
	mov esp, ebp
	pop ebp
	pop ebx
	ret 4
labelCont8:
	mov [ebp -0x20], eax
	
	; determine current directory length:
	push 0
	push 0
	call [ebp -0x14]
	cmp eax, 0
	jnz labelCont9
	mov eax, 9 ; error
	mov esp, ebp
	pop ebp
	pop ebx
	ret 4
labelCont9:
	shl eax, 1 ; utf16
	mov [ebp -0x24], eax
	
	; allocate memory:
	push eax
	push 0
	mov eax, [ebp -0x20]
	push eax
	call [ebp -0x0c]
	cmp eax, 0
	jnz labelCont10
	mov eax, 10 ; error
	mov esp, ebp
	pop ebp
	pop ebx
	ret 4
labelCont10:
	mov [ebp -0x28], eax
	
	; save directory to buffer:
	push eax
	mov eax, [ebp -0x24]
	push eax
	call [ebp -0x14]
	inc eax ; terminating 0
	shl eax, 1 ; utf16
	cmp eax, [ebp -0x24]
	je labelCont11
	call labelSubFree
	mov eax, 11 ; error
	mov esp, ebp
	pop ebp
	pop ebx
	ret 4
labelCont11:
	
	; Set new directory
	mov eax, [ebx +argDllDirectory -labelArgs]
	push eax
	call [ebp -0x18]
	cmp eax, 0
	jnz labelCont12
	call labelSubFree
	mov eax, 12 ; error
	mov esp, ebp
	pop ebp
	pop ebx
	ret 4
labelCont12:
	
	; LoadLibraryExW:
	push 0x00000008 ; LOAD_WITH_ALTERED_SEARCH_PATH
	push 0
	mov eax, [ebx +argDllFilePath -labelArgs]
	push eax
	call [ebp -0x1c]
	cmp eax, 0
	jnz labelCont13
	call labelSubRestoreCwd
	call labelSubFree
	mov eax, 13 ; error
	mov esp, ebp
	pop ebp
	pop ebx
	ret 4
labelCont13:
	
	; resore old Current Working Directory:
	call labelSubRestoreCwd
	cmp eax, 0
	jnz labelCont14
	call labelSubFree
	mov eax, 14 ; error
	mov esp, ebp
	pop ebp
	pop ebx
	ret 4
labelCont14:

	; Free memory:
	call labelSubFree
	cmp eax, 0
	jnz labelCont15
	mov eax, 15 ; error
	mov esp, ebp
	pop ebp
	pop ebx
	ret 4
labelCont15:
	
	; We are done:
	mov eax, 0 ; success
	mov esp, ebp
	pop ebp
	pop ebx
	ret 4
	
labelSubRestoreCwd:
	mov eax, [ebp -0x28]
	push eax
	call [ebp -0x18]
	ret
	
labelSubFree:
	mov eax, [ebp -0x28]
	push eax
	push 0
	mov eax, [ebp -0x20]
	push eax
	call [ebp -0x10]
	ret

