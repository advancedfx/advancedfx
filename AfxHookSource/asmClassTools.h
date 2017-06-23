#pragma once

#define NAKED_JMP_IFACE_FN(iface,index) \
	__asm mov ecx, iface \
	__asm mov eax, [ecx] \
	__asm mov eax, [eax +4*index] \
	__asm jmp eax

#define NAKED_JMP_IFACE_FN_DBG NAKED_JMP_IFACE_FN

#define NAKED_JMP_CLASSMEMBERIFACE_FN(classType,classMemberIface,index) \
	__asm mov ecx, [ecx]classType.classMemberIface \
	__asm mov eax, [ecx] \
	__asm mov eax, [eax +4*index] \
	__asm jmp eax

#define NAKED_JMP_CLASSMEMBERIFACE_FN_DBG NAKED_JMP_CLASSMEMBERIFACE_FN

#define NAKED_JMP_CLASSMEMBERIFACE_OFS_FN(classType,classMemberIface,ofs,index) \
	__asm sub ecx, 4*ofs \
	__asm mov ecx, [ecx]classType.classMemberIface \
	__asm add ecx, 4*ofs \
	__asm mov eax, [ecx] \
	__asm mov eax, [eax +4*index] \
	__asm jmp eax

#define NAKED_JMP_CLASSMEMBERIFACE_OFS_FN_DBG NAKED_JMP_CLASSMEMBERIFACE_OFS_FN