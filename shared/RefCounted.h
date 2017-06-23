#pragma once

namespace Afx {

class IRefCounted abstract
{
public:
	virtual void AddRef(void) = 0;
	virtual void Release(void) = 0;
};

class CRefCounted
: public IRefCounted
{
public:
	CRefCounted();

	virtual void AddRef(void);
	virtual void Release(void);

protected:
	virtual ~CRefCounted();

private:
	int m_RefCount;
};

} // namespace Afx {
