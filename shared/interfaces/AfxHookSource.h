#pragma once

#define ADVANCEDFX_DEF_INTERFACE(name) struct name
#define ADVANCEDFX_DECL_INTERFACE(name) __declspec(novtable) struct name abstract

namespace advancedfx {

	ADVANCEDFX_DECL_INTERFACE(IAfxString)
	{
		virtual void AddRef() abstract = 0;
		virtual void Release() abstract = 0;

		virtual const char * Get() abstract = 0;
	};

	ADVANCEDFX_DEF_INTERFACE(IPython);

	ADVANCEDFX_DECL_INTERFACE(IPythonEventHandler)
	{
		virtual void OnConnected(IPython * python) abstract = 0;
		virtual void OnDisconnected(IPython * python) abstract = 0;
	};

	ADVANCEDFX_DECL_INTERFACE(IPythonEvents)
	{
		virtual void Add(IPythonEventHandler * handler) abstract = 0;
		virtual void Remove(IPythonEventHandler * handler) abstract = 0;
	};

	ADVANCEDFX_DECL_INTERFACE(IAfxHookSourceEvents)
	{
		virtual IPythonEvents * Python() abstract = 0;
	};

	ADVANCEDFX_DECL_INTERFACE(IAfxHookSource)
	{
		virtual IAfxHookSourceEvents * Events()
	};

	ADVANCEDFX_DECL_INTERFACE(IAfxScriptEngine)
	{
		virtual void Connect(IAfxHookSource * afxHookSource) abstract = 0;
		virtual void Disconnect(IAfxHookSource * afxHookSource) abstract = 0;

		virtual bool Exec(const char * code, IAfxString * outErrorString = nullptr) abstract = 0;
	};

}