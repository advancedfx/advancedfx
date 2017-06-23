#pragma once

typedef unsigned long AfxAddr;

//
// Macros:

// Use these primarily for effeciently and quickly accessing the address system:

// set address value macro (fast):
#define AFXADDR_GET(name) \
	(g_AfxAddr_##name)

// get address value macro (fast):
#define AFXADDR_SET(name,value) \
	(g_AfxAddr_##name = value)

// address decleration macro (.h):
#define AFXADDR_DECL(name) \
	extern AfxAddr g_AfxAddr_##name;

// address definition macro (.cpp):
#define AFXADDR_DEF(name) \
	AfxAddr g_AfxAddr_##name; \
	_AfxAddrInfo _g_AfxAddrInfo##name (&g_AfxAddr_##name, #name);

//
// Run-time access functions:

AfxAddr * AfxAddr_GetByName(char const * name);

//
// Debug functions:

bool AfxAddr_Debug_GetAt(unsigned int index, AfxAddr & outAddr, char const * & outName);
unsigned int AfxAddr_Debug_GetCount();


/// <summary>Infrastructure support class, not intended to be used directly.</summary>
class _AfxAddrInfo
{
public:
	AfxAddr * m_Addr;
	char const * m_Name;

	/// <remarks>Don't use this class directly, use the AFXADDR_* macros
	///   instead, so you don't have to worry about object life-time and stuff.
	///   (Since only the pointers are stored.)
	/// </remarks>
	_AfxAddrInfo(AfxAddr * addr, char const * name);
};
