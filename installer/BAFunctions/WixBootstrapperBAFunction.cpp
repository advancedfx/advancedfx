// Copyright (c) .NET Foundation and contributors. All rights reserved. Licensed under the Microsoft Reciprocal License. See LICENSE.TXT file in the project root for full license information.

#include "precomp.h"

class CWixBootstrapperBAFunction : IBootstrapperBAFunction
{
public:
	STDMETHODIMP OnDetect()
	{
		HRESULT hr = S_OK;

		BalLog(BOOTSTRAPPER_LOG_LEVEL_STANDARD, "Running detect BA function");

		//-------------------------------------------------------------------------------------------------
		LPWSTR sczValue = NULL;

		BalGetStringVariable(L"UserUILanguageID", &sczValue);
		BalExitOnFailure(hr, "Failed to get variable.");

		if (0 == wcscmp(sczValue, L"1031"))
		{
			hr = m_pEngine->SetVariableString(L"HlaeCoreTransform", L"de-de.mst");
		}		
		else if (0 == wcscmp(sczValue, L"1035"))
		{
			hr = m_pEngine->SetVariableString(L"HlaeCoreTransform", L"fi-fi.mst");
		}
		else if (0 == wcscmp(sczValue, L"1041"))
		{
			hr = m_pEngine->SetVariableString(L"HlaeCoreTransform", L"ja-jp.mst");
		}
		else if (0 == wcscmp(sczValue, L"1043"))
		{
			hr = m_pEngine->SetVariableString(L"HlaeCoreTransform", L"nl-nl.mst");
		}
		else if (0 == wcscmp(sczValue, L"2070"))
		{
			hr = m_pEngine->SetVariableString(L"HlaeCoreTransform", L"pt-pt.mst");
		}
		else if (0 == wcscmp(sczValue, L"1049"))
		{
			hr = m_pEngine->SetVariableString(L"HlaeCoreTransform", L"ru-ru.mst");
		}
		else if (0 == wcscmp(sczValue, L"2052"))
		{
			hr = m_pEngine->SetVariableString(L"HlaeCoreTransform", L"zh-cn.mst");
		}
		BalExitOnFailure(hr, "Failed to set variable.");
		//-------------------------------------------------------------------------------------------------

	LExit:
		return hr;
	}


	STDMETHODIMP OnDetectComplete() { return S_OK; }
	STDMETHODIMP OnPlan() { return S_OK; }
	STDMETHODIMP OnPlanComplete() { return S_OK; }

	/*
		STDMETHODIMP OnDetectComplete()
		{
			HRESULT hr = S_OK;
			BalLog(BOOTSTRAPPER_LOG_LEVEL_STANDARD, "Running detect complete BA function");
			//-------------------------------------------------------------------------------------------------
			// YOUR CODE GOES HERE
			BalExitOnFailure(hr, "Change this message to represent real error handling.");
			//-------------------------------------------------------------------------------------------------
		LExit:
			return hr;
		}
		STDMETHODIMP OnPlan()
		{
			HRESULT hr = S_OK;
			BalLog(BOOTSTRAPPER_LOG_LEVEL_STANDARD, "Running plan BA function");
			//-------------------------------------------------------------------------------------------------
			// YOUR CODE GOES HERE
			BalExitOnFailure(hr, "Change this message to represent real error handling.");
			//-------------------------------------------------------------------------------------------------
		LExit:
			return hr;
		}

		STDMETHODIMP OnPlanComplete()
		{
			HRESULT hr = S_OK;
			BalLog(BOOTSTRAPPER_LOG_LEVEL_STANDARD, "Running plan complete BA function");
			//-------------------------------------------------------------------------------------------------
			// YOUR CODE GOES HERE
			BalExitOnFailure(hr, "Change this message to represent real error handling.");
			//-------------------------------------------------------------------------------------------------
		LExit:
			return hr;
		}
	*/


private:
	HMODULE m_hModule;
	IBootstrapperEngine* m_pEngine;


public:
	//
	// Constructor - initialize member variables.
	//
	CWixBootstrapperBAFunction(
		__in IBootstrapperEngine* pEngine,
		__in HMODULE hModule
	)
	{
		m_hModule = hModule;
		m_pEngine = pEngine;
	}

	//
	// Destructor - release member variables.
	//
	~CWixBootstrapperBAFunction()
	{
	}
};


extern "C" HRESULT WINAPI CreateBootstrapperBAFunction(
	__in IBootstrapperEngine* pEngine,
	__in HMODULE hModule,
	__out CWixBootstrapperBAFunction** ppBAFunction
)
{
	HRESULT hr = S_OK;
	CWixBootstrapperBAFunction* pBAFunction = NULL;

	// This is required to enable logging functions
	BalInitialize(pEngine);

	pBAFunction = new CWixBootstrapperBAFunction(pEngine, hModule);
	ExitOnNull(pBAFunction, hr, E_OUTOFMEMORY, "Failed to create new bootstrapper BA function object.");

	*ppBAFunction = pBAFunction;
	pBAFunction = NULL;

LExit:
	delete pBAFunction;
	return hr;
}