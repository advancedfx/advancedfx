#pragma once

typedef void * StoreValue;


class __declspec(novtable) IStoreFactory abstract
{
public:
	virtual StoreValue ConstructValue() abstract = 0;
	virtual void DestructValue(StoreValue value) abstract = 0;
};


class __declspec(novtable) IStoreItem abstract
{
public:
	virtual StoreValue GetValue() abstract = 0;
	virtual void Release() abstract = 0;
};


class Store abstract
{
public:
	virtual ~Store();

	virtual IStoreItem * Aquire(void) abstract = 0;
};


class FrequentStoreManager;

class FrequentStore :
	public Store
{
public:
	FrequentStore(IStoreFactory * factory);
	virtual ~FrequentStore();

	virtual IStoreItem * Aquire(void);
	virtual void Destruct(void)
	{
		delete this;
	}

	void Pack(void);

private:
	FrequentStoreManager * m_Manager;
};
