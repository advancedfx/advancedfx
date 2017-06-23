#include "stdafx.h"

#include "AfxAddr.h"

//#include <windows.h>

#include <list>


class AfxAddresses {
public:
	void Register(_AfxAddrInfo * info);
	AfxAddr * GetByName(char const * name);

	_AfxAddrInfo * Debug_GetAt(unsigned int index);
	unsigned int Debug_GetCount();
	
private:
	std::list<_AfxAddrInfo *> m_AddrInfos;

};

AfxAddresses & GetAfxAddresses()
{
	static AfxAddresses afxAddresses;
	return afxAddresses;
}


_AfxAddrInfo::_AfxAddrInfo(AfxAddr * addr, char const * name)
{
	m_Addr = addr;
	m_Name = name;

	GetAfxAddresses().Register(this);
}


void AfxAddresses::Register(_AfxAddrInfo * info)
{
	m_AddrInfos.push_back(info);
}

AfxAddr * AfxAddresses::GetByName(char const * name)
{
	std::list<_AfxAddrInfo *>::iterator it;
	for (it = m_AddrInfos.begin(); it != m_AddrInfos.end(); it++)
	{
		if(strcmp((*it)->m_Name, name)==0)
			return (*it)->m_Addr;
	}

	return 0;
}

_AfxAddrInfo * AfxAddresses::Debug_GetAt(unsigned int index)
{
	if(index < m_AddrInfos.size())
	{
		std::list<_AfxAddrInfo *>::iterator it = m_AddrInfos.begin();
		while(0 < index)
		{
			it++;
			index--;
		}

		return (*it);
	}

	return 0;
}

unsigned int AfxAddresses::Debug_GetCount()
{
	return m_AddrInfos.size();
}


AfxAddr * AfxAddr_GetByName(char const * name)
{
	return GetAfxAddresses().GetByName(name);
}


bool AfxAddr_Debug_GetAt(unsigned int index, AfxAddr & outAddr, char const * & outName)
{
	_AfxAddrInfo * info = GetAfxAddresses().Debug_GetAt(index);

	if(info)
	{
		outAddr = *(info->m_Addr);
		outName = info->m_Name;
	}

	return 0 != info;
}

unsigned int AfxAddr_Debug_GetCount()
{
	return GetAfxAddresses().Debug_GetCount();
}

