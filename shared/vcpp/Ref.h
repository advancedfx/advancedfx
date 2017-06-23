#pragma once

#ifdef _DEBUG
#define AFX_DEBUG_REF
#endif


namespace Afx {

/// <summary>
/// Object which's life-time is managed using
/// Reference Counting.
/// </summary>
struct __declspec(novtable) IRef abstract
{
	virtual void AddRef (void) abstract = 0;
	virtual void Release (void) abstract = 0;
};

/// <summary>
/// Base class implementation of IRef.
/// </summary>
class Ref :
	public IRef
{
public:
	Ref();

	virtual void AddRef (void);

#ifdef AFX_DEBUG_REF
	/// <remarks>Results might inaccurate due to threading.</remarks>
	static unsigned int DEBUG_GetGlobalRefCount (void);
#endif

	virtual void Release (void);

	/// <summary>AddRef, Release</summary>
	void TouchRef (void);

	/// <summary>AddRef, Release</summary>
	static void TouchRef (IRef * ref);

protected:
	unsigned int m_RefCount;

	virtual ~Ref();

private:

#ifdef AFX_DEBUG_REF
	static unsigned int m_GlobalRefCount;
#endif

};


template <class T>
/// <summary>
/// Reference counting implementation helper.<br />
/// This class will AddRef upon construction and Release upon
/// destruction. This way it can be used to automate reference
/// counting within a specific scope. (Scoping oddities are
/// beyond this summary.)
/// </summary>
/// <remarks>
/// To be used with classes that derieve from Ref.
/// </remarks>
class RefPtr
{
public:
	RefPtr(T * ref)
	: m_Ref(ref)
	{
		ref->AddRef();
	}

	~RefPtr()
	{
		m_Ref->Release();
	}

	T * get (void) const {
		return m_Ref;
	}

protected:
	T * m_Ref;
};


template <class T>
/// <summary>
/// Reference counting implementation helper.<br />
/// This class will AddRef upon construction and Release upon
/// destruction. This way it can be used to automate reference
/// counting within a specific scope. (Scoping oddities are
/// beyond this summary.)
/// </summary>
/// <remarks>To be used with some interface that supplies [ virtual IRef * Ref (void); ].</remarks>
class RefIPtr
{
public:
	RefIPtr(T * ref)
	: m_Ref(ref)
	{
		ref->Ref()->AddRef();
	}

	~RefIPtr()
	{
		m_Ref->Ref()->Release();
	}

	T * get (void) const {
		return m_Ref;
	}

protected:
	T * m_Ref;
};



} // namespace Afx {
