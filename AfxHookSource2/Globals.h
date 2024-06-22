#pragma once
#include <Windows.h>

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define MkErrStr(file,line) "Problem in " file ":" STRINGIZE(line)

extern void ErrorBox(char const * messageText);

extern void ErrorBox();