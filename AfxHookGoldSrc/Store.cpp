#include "stdafx.h"

#include "Store.h"

#include <stdexcept>

using namespace std;

////////////////////////////////////////////////////////////////////////////////

class FrequentStoreItem :
	public IStoreItem
{
public:
	FrequentStoreItem * NextFree;

	FrequentStoreItem(FrequentStoreManager * manager);
	~FrequentStoreItem();

	void Aquire();

	virtual StoreValue GetValue();

	virtual void Release();

private:
	bool m_Aquired;
	FrequentStoreManager * m_Manager;
	StoreValue m_Value;

	void Delist();
	void Enlist();
};

class FrequentStoreManager
{
public:
	FrequentStoreItem * FreeItem;

	FrequentStoreManager(IStoreFactory * factory);
	~FrequentStoreManager();

	IStoreItem * Aquire(void);

	IStoreFactory * GetFactory();

	void Pack(void);

private:
	IStoreFactory * m_Factory;

};


// FrequentStore /////////////////////////////////////////////////////////

FrequentStore::FrequentStore(IStoreFactory * factory)
{
	m_Manager = new FrequentStoreManager(factory);
}
FrequentStore::~FrequentStore()
{
	delete m_Manager;
}

IStoreItem * FrequentStore::Aquire(void)
{
	return m_Manager->Aquire();
}

void FrequentStore::Pack(void)
{
	m_Manager->Pack();
}


// FrequentStoreManager //////////////////////////////////////////////////

FrequentStoreManager::FrequentStoreManager(IStoreFactory * factory)
{
	FreeItem = 0;
	m_Factory = factory;
}

FrequentStoreManager::~FrequentStoreManager()
{
	while(FreeItem)
		delete FreeItem;
}

IStoreItem * FrequentStoreManager::Aquire(void)
{
	FrequentStoreItem * item = FreeItem;

	if(!item)
		item = new FrequentStoreItem(this);

	item->Aquire();

	return item;
}

IStoreFactory *  FrequentStoreManager::GetFactory()
{
	return m_Factory;
}

void FrequentStoreManager::Pack(void)
{
	while(FreeItem)
		delete FreeItem;
}



// FrequentStoreItem /////////////////////////////////////////////////////

FrequentStoreItem::FrequentStoreItem(FrequentStoreManager * manager)
{
	NextFree = 0;
	m_Aquired = false;
	m_Manager = manager;
	m_Value = manager->GetFactory()->ConstructValue();

	Enlist();
}

FrequentStoreItem::~FrequentStoreItem()
{
	//if(m_Aquired)
	//	throw logic_error("");

	Delist();

	m_Manager->GetFactory()->DestructValue(m_Value);
}

void FrequentStoreItem::Aquire()
{
	if(m_Aquired)
		throw logic_error("");

	Delist();
	m_Aquired = true;
}

void FrequentStoreItem::Delist()
{
	m_Manager->FreeItem = NextFree;
	NextFree = 0;
}

void FrequentStoreItem::Enlist()
{
	NextFree = m_Manager->FreeItem;
	m_Manager->FreeItem = this;
}

StoreValue FrequentStoreItem::GetValue()
{
	return m_Value;
}

void FrequentStoreItem::Release()
{
	if(!m_Aquired)
		throw logic_error("");

	m_Aquired = false;
	Enlist();
}


// Store ///////////////////////////////////////////////////////////////////////

Store::~Store()
{
}