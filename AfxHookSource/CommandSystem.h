#pragma once

#include <WrpConsole.h>

#include <deps/release/prop/shared/AfxMath.h>

#include <string>
#include <map>
#include <vector>
#include <list>

class CommandSystem
{
public:
	bool Enabled;

	CommandSystem();

	~CommandSystem() {
		Clear();
	}

	void Add(char const * command);
	void AddTick(char const * command);

	void AddAtTime(char const* command, double time);
	void AddAtTick(char const* command, int tick);

	void AddCurves(IWrpCommandArgs* args);

	void EditStart(double startTime);
	void EditStartTick(int startTick);

	bool Remove(int index);

	void Clear(void);

	bool Save(wchar_t const * fileName);
	bool Load(wchar_t const * fileName);

	void Console_List(void);

	void Do_Commands(void);

	void OnLevelInitPreEntityAllTools(void);

	double GetLastTime(void) {
		return m_LastTime;
	}

	int GetLastTick(void) {
		return m_LastTick;
	}

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

		bool DoCommand(double t01);

		int LastTick = -1;
		double LastTime = -1;

	private:
		bool m_Animated = false;
		bool m_Formated = false;
		std::string m_Command;
		std::vector<CDoubleInterp> m_Interp;
	};

	template<typename T> struct Interval {

		Interval() {
		}

		Interval(T low, T high)
			: Low(low)
			, High(high) {

		}

		T Low, High;

		bool operator<(const Interval<T>& other) const {
			T cmp = Low - other.Low;
			if (cmp < 0) return true;
			return cmp == 0 && High < other.High;
		}
	};

	template<typename T> struct ITNode {
		CCommand* Command;
		Interval<T> i;
		T Max;
		struct ITNode<T>* Left, * Right;
	};

	template<typename T> ITNode<T>* NewITNode(Interval<T> i, CCommand * command) {
		ITNode<T>* result = new ITNode<T>();
		result->Command = command;
		result->i = i;
		result->Max = i.High;
		result->Right = result->Left = nullptr;
		return result;
	}

	template<typename T> void DeleteNode(ITNode<T>*root) {
		if (nullptr == root)
			return;

		DeleteNode(root->Left);
		DeleteNode(root->Right);
	}

	template<typename T> ITNode<T>* Insert(ITNode<T>* root, Interval<T> i, CCommand * command)
	{
		if (nullptr == root)
			return NewITNode<T>(i, command);

		T l = root->i.Low;

		if (i.Low < l)
			root->Left = Insert<T>(root->Left, i, command);
		else
			root->Right = Insert<T>(root->Right, i, command);

		if (root->Max < i.High)
			root->Max = i.High;

		return root;
	}

	template<typename T> bool DoOverlap(Interval<T> i1, Interval<T> i2)
	{
		if (i1.Low <= i2.High && i2.Low <= i1.High)
			return true;
		return false;
	}

	void OverlapExecute(ITNode<int>* root, Interval<int> i, float interpolationAmount)
	{
		if (nullptr == root) return;

		if (DoOverlap<int>(root->i, i))
		{
			if (root->Command && (root->Command->LastTick != i.Low || 0 <root->Command->GetSize()))
			{
				double d = 1.0 + (double)root->i.High - (double)root->i.Low;
				double t01 = 0 != d ? ((double)i.High - (double)root->i.Low + (double)interpolationAmount) / d : 1;

				if (t01 < 0) t01 = 0;
				else if (1 < t01) t01 = 1;

				root->Command->DoCommand(t01);
				root->Command->LastTick = i.High;
			}
		}

		if (nullptr != root->Left && root->Left->Max >= i.Low)
			OverlapExecute(root->Left, i, interpolationAmount);
		else
			OverlapExecute(root->Right, i, interpolationAmount);
	}

	void OverlapExecute(ITNode<double>* root, Interval<double> i)
	{
		if (nullptr == root) return;

		if (DoOverlap<double>(root->i, i))
		{
			if (root->Command && (root->Command->LastTime != i.Low || 0 < root->Command->GetSize()))
			{
				double d = (double)root->i.High - (double)root->i.Low;
				double t01 = 0 != d ? ((double)i.High - (double)root->i.Low) / d : 1;

				if (t01 < 0) t01 = 0;
				else if (1 < t01) t01 = 1;

				root->Command->DoCommand(t01);
				root->Command->LastTime = i.High;
			}
		}

		if (nullptr != root->Left && root->Left->Max >= i.Low)
			OverlapExecute(root->Left, i);
		else
			OverlapExecute(root->Right, i);
	}

	ITNode<int>* m_TickTree = nullptr;
	ITNode<double>* m_TimeTree = nullptr;

	std::multimap<Interval<int>, CCommand*> m_TickMap;
	std::multimap<Interval<double>, CCommand*> m_TimeMap;

	void DeleteTickTree()
	{
		DeleteNode<int>(m_TickTree);
		m_TickTree = nullptr;
	}

	void DeleteTimeTree()
	{
		DeleteNode<double>(m_TimeTree);
		m_TimeTree = nullptr;
	}

	void EnsureTickTree()
	{
		if (m_TickTree != nullptr) return;

		for (auto it = m_TickMap.begin(); it != m_TickMap.end(); ++it)
		{
			m_TickTree = Insert<int>(m_TickTree, it->first, it->second); // TODO: This tree will degenerate really bad.
		}
	}

	void EnsureTimeTree()
	{
		if (m_TimeTree != nullptr) return;

		for (auto it = m_TimeMap.begin(); it != m_TimeMap.end(); ++it)
		{
			m_TimeTree = Insert<double>(m_TimeTree, it->first, it->second); // TODO: This tree will degenerate really bad.
		}
	}

	double m_LastTime;
	int m_LastTick;

	bool m_GotCleared = false;

	bool IsSupportedByTime(void);
	bool IsSupportedByTick(void);
};

extern CommandSystem g_CommandSystem;
