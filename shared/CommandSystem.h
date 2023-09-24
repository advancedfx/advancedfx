#pragma once

#include "AfxConsole.h"
#include "AfxMath.h"

#include <string>
#include <map>
#include <vector>
#include <list>
#include <queue>

class IExecuteClientCmdForCommandSystem {
public:
	virtual void ExecuteClientCmd(const char * value) = 0;
};

class IGetTickForCommandSystem {
public:
	virtual float GetTick() = 0;
};

class IGetTimeForCommandSystem {
public:
	virtual float GetTime() = 0;
};

class CommandSystem
{
public:
	bool Enabled;

	/// <param name="pExecuteClientCmd">must not be nullptr</param>
	/// <param name="pGetTick">can be nullptr</param>
	/// <param name="pGetTime">can be nullptr</param>
	CommandSystem(class IExecuteClientCmdForCommandSystem * pExecuteClientCmd,
		class IGetTickForCommandSystem * pGetTick,
		class IGetTimeForCommandSystem * pGetTime
	);

	~CommandSystem() {
		Clear();
	}

	void OnExecuteCommands(void);

	void OnLevelInitPreEntity(void);

	void Console_Command(advancedfx::ICommandArgs* args);

private:
	class CDoubleInterp
	{
	public:
		enum Method_e {
			Method_Linear,
			Method_Cubic
		};

		CDoubleInterp()
			: m_View(&m_Map, Selector)
		{

		}

		CDoubleInterp(const CDoubleInterp& copyFrom)
			: m_Map(copyFrom.m_Map)
			, m_View(&m_Map, Selector)
			, m_Interp(nullptr)
			, m_Method(copyFrom.m_Method)
		{
		

		}

		~CDoubleInterp()
		{
			delete m_Interp;
		}

		Method_e GetMethod()
		{
			return m_Method;
		}

		void SetMethod(Method_e value)
		{
			if (m_Method != value)
			{
				if (nullptr != m_Interp)
				{
					delete m_Interp;
					m_Interp = nullptr;
				}
			}
			m_Method = value;
		}

		bool CanEval(void)
		{
			EnsureInterp();

			return m_Interp->CanEval();
		}

		/// <remarks>
		/// Must not be called if CanEval() returns false!<br />
		/// </remarks>
		double Eval(double t)
		{
			EnsureInterp();

			return m_Interp->Eval(t);
		}

		Afx::Math::CInterpolationMap<double>& GetMap()
		{
			return m_Map;
		}

		void TriggerMapChanged()
		{
			if (m_Interp) m_Interp->InterpolationMapChanged();
		}
		

	private:
		Afx::Math::CInterpolationMap<double> m_Map;
		Afx::Math::CInterpolationMapView<double, double> m_View;
		Afx::Math::CInterpolation<double>* m_Interp = nullptr;
		Method_e m_Method = Method_Linear;

		static double Selector(double const& value)
		{
			return value;
		}

		void EnsureInterp()
		{
			if (nullptr == m_Interp) m_Interp = m_Method == Method_Cubic ? static_cast<Afx::Math::CInterpolation<double>*>(new Afx::Math::CCubicDoubleInterpolation<double>(&m_View)) : static_cast<Afx::Math::CInterpolation<double>*>(new Afx::Math::CLinearDoubleInterpolation<double>(&m_View));
		}
	};

	class CCommand {
	public:
		CCommand()
		{
		}

		~CCommand()
		{
		}

		size_t GetSize() { return m_Interp.size(); }
		void SetSize(size_t value) { m_Interp.resize(value); }

		bool GetFormated() { return m_Formated; }
		void SetFormated(bool value) { m_Formated = value; }

		const char* GetCommand() { return m_Command.c_str(); }
		void SetCommand(const char* value) { m_Command = value; }

		CDoubleInterp::Method_e GetInterp(int idx) { return m_Interp[idx].GetMethod(); }
		void SetInterp(int idx, CDoubleInterp::Method_e value) { m_Interp[idx].SetMethod(value); }

		Afx::Math::CInterpolationMap<double>& GetMap(int idx)
		{
			return m_Interp[idx].GetMap();
		}
	
		void TriggerMapChanged(int idx)
		{
			m_Interp[idx].TriggerMapChanged();
		}

		void Remove(int idx)
		{
			auto it = m_Interp.begin(); std::advance(it, idx);
			m_Interp.erase(it);
		}

		void Add(int idx)
		{
			auto it = m_Interp.begin(); std::advance(it, idx);
			m_Interp.insert(it, CDoubleInterp());
		}

		void ClearCurves()
		{
			m_Interp.clear();
		}

		bool DoCommand(double t01, std::queue<std::string> & commandsToExecute);

	private:
		bool m_OnlyOnce = false;
		bool m_Formated = false;
		std::string m_Command;
		std::vector<CDoubleInterp> m_Interp;
	};

	struct Interval {

		Interval() {
		}

		Interval(double low, double high, bool epsilon)
			: Low(low)
			, High(high)
			, Epsilon(epsilon)
		{

		}

		double Low, High;
		bool Epsilon;

		bool operator<(const Interval& other) const {
			double cmp = Low - other.Low;
			if (cmp < 0) return true;
			return cmp == 0 && High < other.High;
		}
	};

	struct ITNode {
		CCommand* Command;
		Interval i;
		double Max;
		struct ITNode* Left, * Right;
	};

	 ITNode* NewITNode(Interval i, CCommand * command) {
		ITNode* result = new ITNode();
		result->Command = command;
		result->i = i;
		result->Max = i.High;
		result->Right = result->Left = nullptr;
		return result;
	}

	void DeleteNode(ITNode*root) {
		if (nullptr == root)
			return;

		DeleteNode(root->Left);
		DeleteNode(root->Right);
	}

	ITNode* Insert(ITNode* root, Interval i, CCommand * command)
	{
		if (nullptr == root)
			return NewITNode(i, command);

		double l = root->i.Low;

		if (i.Low < l)
			root->Left = Insert(root->Left, i, command);
		else
			root->Right = Insert(root->Right, i, command);

		if (root->Max < i.High)
			root->Max = i.High;

		return root;
	}

	bool DoOverlap(Interval i1, Interval i2)
	{
		if ((i2.Epsilon ? i1.Low <= i2.High : i1.Low < i2.High)
			&& (i1.Epsilon ? i2.Low <= i1.High: i2.Low < i1.High))
			return true;
		return false;
	}

	void OverlapExecute(ITNode* root, Interval i)
	{
		if (nullptr == root) return;

		if (DoOverlap(root->i, i))
		{
			if (root->Command)
			{
				double d = (double)root->i.High - (double)root->i.Low;
				double t01 = 0 != d ? ((double)i.High - (double)root->i.Low) / d : 1;

				if (t01 < 0) t01 = 0;
				else if (1 < t01) t01 = 1;

				root->Command->DoCommand(t01, m_CommandsToExecute);
			}
		}

		if (nullptr != root->Left && root->Left->Max >= i.Low)
			OverlapExecute(root->Left, i);
		else
			OverlapExecute(root->Right, i);
	}

	ITNode* m_TickTree = nullptr;
	ITNode* m_TimeTree = nullptr;

	std::multimap<Interval, CCommand*> m_TickMap;
	std::multimap<Interval, CCommand*> m_TimeMap;

	std::queue<std::string> m_CommandsToExecute;

	void DeleteTickTree()
	{
		DeleteNode(m_TickTree);
		m_TickTree = nullptr;
	}

	void DeleteTimeTree()
	{
		DeleteNode(m_TimeTree);
		m_TimeTree = nullptr;
	}

	void EnsureTickTree()
	{
		if (m_TickTree != nullptr) return;

		for (auto it = m_TickMap.begin(); it != m_TickMap.end(); ++it)
		{
			m_TickTree = Insert(m_TickTree, it->first, it->second); // TODO: This tree will degenerate really bad.
		}
	}

	void EnsureTimeTree()
	{
		if (m_TimeTree != nullptr) return;

		for (auto it = m_TimeMap.begin(); it != m_TimeMap.end(); ++it)
		{
			m_TimeTree = Insert(m_TimeTree, it->first, it->second); // TODO: This tree will degenerate really bad.
		}
	}

	double m_LastTime;
	double m_LastTick;

	bool m_GotCleared = false;

	bool IsSupportedByTime(void);
	bool IsSupportedByTick(void);

	void EditCommandCurves(Interval I, CCommand * c, advancedfx::ICommandArgs* args);

	class IExecuteClientCmdForCommandSystem * m_pExecuteClientCmd = nullptr;
	class IGetTickForCommandSystem * m_pGetTick = nullptr;
	class IGetTimeForCommandSystem * m_pGetTime = nullptr;

	void Add(char const * command);
	void AddTick(char const * command);

	void AddAtTime(char const* command, double time);
	void AddAtTick(char const* command, double tick);

	void AddCurves(advancedfx::ICommandArgs* args);

	void EditStart(double startTime);
	void EditStartTick(double startTick);

	void EditCommand(advancedfx::ICommandArgs* args);

	bool Remove(int index);

	void Clear(void);

	bool Save(wchar_t const * fileName);
	bool Load(wchar_t const * fileName);

	void Console_List(void);

	double GetLastTime(void) {
		return m_LastTime;
	}

	double GetLastTick(void) {
		return m_LastTick;
	}
};
