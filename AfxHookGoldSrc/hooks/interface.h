#pragma once

#define CREATEINTERFACE_PROCNAME	"CreateInterface"

class CSysModule;

typedef void* (*CreateInterfaceFn)(const char *pName, int *pReturnCode);

CreateInterfaceFn Sys_GetFactory( CSysModule *pModule );
