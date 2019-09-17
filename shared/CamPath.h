#pragma once

#include "RefCounted.h"
#include <shared/AfxMath.h>

using namespace Afx;
using namespace Afx::Math;

struct CamPathValue
{
	double X;
	double Y;
	double Z;

	Quaternion R;

	double Fov;

	bool Selected;

	CamPathValue();

	CamPathValue(double x, double y, double z, double pitch, double yaw, double roll, double fov);

};

struct CamPathIterator
{
public:
	CInterpolationMap<CamPathValue>::const_iterator wrapped;

	CamPathIterator(CInterpolationMap<CamPathValue>::const_iterator & it);

	double GetTime();

	CamPathValue GetValue();

	CamPathIterator& operator ++ ();

	bool operator == (CamPathIterator const &it) const;

	bool operator != (CamPathIterator const &it) const;

};

class CamPath;

class ICamPathChanged abstract
{
public:
	virtual void CamPathChanged(CamPath * obj) = 0;
};

class CamPath
{
public:
	enum DoubleInterp {
		DI_DEFAULT = 0,
		DI_LINEAR = 1,
		DI_CUBIC = 2,
		_DI_COUNT = 3
	};

	enum QuaternionInterp {
		QI_DEFAULT = 0,
		QI_SLINEAR = 1,
		QI_SCUBIC = 2,
		_QI_COUNT = 3,
	};

	static bool DoubleInterp_FromString(char const * value, DoubleInterp & outValue);
	static char const * DoubleInterp_ToString(DoubleInterp value);

	static bool QuaternionInterp_FromString(char const * value, QuaternionInterp & outValue);
	static char const * QuaternionInterp_ToString(QuaternionInterp value);

	CamPath();
	
	~CamPath();

	void Enabled_set(bool enable);
	bool Enabled_get(void);

	void PositionInterpMethod_set(DoubleInterp value);
	DoubleInterp PositionInterpMethod_get(void);

	void RotationInterpMethod_set(QuaternionInterp value);
	QuaternionInterp RotationInterpMethod_get(void);

	void FovInterpMethod_set(DoubleInterp value);
	DoubleInterp FovInterpMethod_get(void);

	void Add(double time, CamPathValue value);

	void Remove(double time);
	void Clear();

	size_t GetSize();
	CamPathIterator GetBegin();
	CamPathIterator GetEnd();
	double GetDuration();

	/// <remarks>Must not be called if GetSize is less than 1!</remarks>
	double GetLowerBound();

	/// <remarks>Must not be called if GetSize is less than 1!</remarks>
	double GetUpperBound();

	bool CanEval(void);

	/// <remarks>
	/// Must not be called if CanEval() returns false!<br />
	/// </remarks>
	CamPathValue Eval(double t);

	bool Save(wchar_t const * fileName);
	bool Load(wchar_t const * fileName);
	
	/// <remarks>In the current implementation if points happen to fall on the same time value, then the last point's value will be used (no interpolation).</remarks>
	/// <param name="relative">If t is an relative offset (true), or absolute value (false).</param>
	void SetStart(double t, bool relative = false);
	
	/// <remarks>In the current implementation if points happen to fall on the same time value, then the last point's value will be used (no interpolation).
	/// Setting duration for a path with less than 2 points will do nothing.</remarks>
	void SetDuration(double t);

	void SetPosition(double x, double y, double z);

	void SetAngles(double yPitch, double zYaw, double xRoll);

	void SetFov(double fov);

	void Rotate(double yPitch, double zYaw, double xRoll);

	void AnchorTransform(double anchorX, double anchorY, double anchorZ, double anchorYPitch, double anchorZYaw, double anchorXRoll, double destX, double destY, double destZ, double destYPitch, double destZYaw, double destXRoll);

	size_t SelectAll();

	void SelectNone();

	size_t SelectInvert();

	/// <summary>Adds a range of key frames to the selection.</param>
	/// <param name="min">Index of first keyframe to add to selection.</param>
	/// <param name="max">Index of last keyframe to add to selection.</param>
	/// <returns>Number of selected keyframes.</returns>
	size_t SelectAdd(size_t min, size_t max);

	/// <summary>Adds a range of key frames to the selection.</param>
	/// <param name="min">Lower bound to start adding selection at.</param>
	/// <param name="count">Number of keyframes to select.</param>
	/// <returns>Number of selected keyframes.</returns>
	size_t SelectAdd(double min, size_t count);

	/// <summary>Adds a range of key frames to the selection.</param>
	/// <param name="min">Lower bound to start adding selection at.</param>
	/// <param name="count">Upper bound to end adding selection at.</param>
	/// <returns>Number of selected keyframes.</returns>
	size_t SelectAdd(double min, double max);

	void OnChanged_set(ICamPathChanged * value);

	void SetOffset(double value);

	double GetOffset();

private:
	static double XSelector(CamPathValue const & value)
	{
		return value.X;
	}

	static double YSelector(CamPathValue const & value)
	{
		return value.Y;
	}

	static double ZSelector(CamPathValue const & value)
	{
		return value.Z;
	}

	static Quaternion RSelector(CamPathValue const & value)
	{
		return value.R;
	}

	static double FovSelector(CamPathValue const & value)
	{
		return value.Fov;
	}

	static bool SelectedSelector(CamPathValue const & value)
	{
		return value.Selected;
	}

	bool m_Enabled;
	DoubleInterp m_PositionInterpMethod;
	QuaternionInterp m_RotationInterpMethod;
	DoubleInterp m_FovInterpMethod;
	ICamPathChanged * m_OnChanged;
	double m_Offset;
	
	CInterpolationMap<CamPathValue> m_Map;

	CInterpolationMapView<CamPathValue, double> m_XView;
	CInterpolationMapView<CamPathValue, double> m_YView;
	CInterpolationMapView<CamPathValue, double> m_ZView;
	CInterpolationMapView<CamPathValue, Quaternion> m_RView;
	CInterpolationMapView<CamPathValue, double> m_FovView;
	CInterpolationMapView<CamPathValue, bool> m_SelectedView;

	CInterpolation<double> * m_XInterp;
	CInterpolation<double> * m_YInterp;
	CInterpolation<double> * m_ZInterp;
	CInterpolation<Quaternion> * m_RInterp;
	CInterpolation<double> * m_FovInterp;
	CInterpolation<bool> * m_SelectedInterp;

	void Changed();
	void CopyMap(CInterpolationMap<CamPathValue> & dst, CInterpolationMap<CamPathValue> & src);

	void DoInterpolationMapChangedAll(void);
};
