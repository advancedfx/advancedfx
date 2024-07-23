#include "stdafx.h"

#include "CamPath.h"

#include "../deps/release/rapidxml/rapidxml.hpp"
#include "../deps/release/rapidxml/rapidxml_print.hpp"
#include <iterator>
#include <stdio.h>
#include <fstream>
#include <algorithm>

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

bool CamPath::DoubleInterp_FromString(char const * value, DoubleInterp & outValue)
{
	if(!_stricmp(value,"default"))
	{
		outValue = DI_DEFAULT;
		return true;
	}
	else
	if(!_stricmp(value,"linear"))
	{
		outValue = DI_LINEAR;
		return true;
	}
	else
	if(!_stricmp(value,"cubic"))
	{
		outValue = DI_CUBIC;
		return true;
	}

	return false;
}

char const * CamPath::DoubleInterp_ToString(DoubleInterp value)
{
	switch(value)
	{
	case DI_DEFAULT:
		return "default";
	case DI_LINEAR:
		return "linear";
	case DI_CUBIC:
		return "cubic";
	}

	return "[unkown]";
}

bool CamPath::QuaternionInterp_FromString(char const * value, QuaternionInterp & outValue)
{
	if(!_stricmp(value,"default"))
	{
		outValue = QI_DEFAULT;
		return true;
	}
	else
	if(!_stricmp(value,"sLinear"))
	{
		outValue = QI_SLINEAR;
		return true;
	}
	else
	if(!_stricmp(value,"sCubic"))
	{
		outValue = QI_SCUBIC;
		return true;
	}

	return false;
}

char const * CamPath::QuaternionInterp_ToString(QuaternionInterp value)
{
	switch(value)
	{
	case QI_DEFAULT:
		return "default";
	case QI_SLINEAR:
		return "sLinear";
	case QI_SCUBIC:
		return "sCubic";
	}

	return "[unkown]";
}

CamPathValue::CamPathValue()
: X(0.0), Y(0.0), Z(0.0), R(), Fov(90.0), Selected(false)
{
}

CamPathValue::CamPathValue(double x, double y, double z, double pitch, double yaw, double roll, double fov)
: X(x)
, Y(y)
, Z(z)
, R(Quaternion::FromQREulerAngles(QREulerAngles::FromQEulerAngles(QEulerAngles(pitch,yaw,roll))))
, Fov(fov)
, Selected(false)
{
}

CamPathValue::CamPathValue(double x, double y, double z, double q_w, double q_x, double q_y, double q_z, double fov, bool selected)
: X(x), Y(y), Z(z), R(Quaternion(q_w,q_x,q_y,q_z)), Fov(fov), Selected(selected) {
}

CamPathIterator::CamPathIterator(CInterpolationMap<CamPathValue>::const_iterator & it) : wrapped(it)
{
}

double CamPathIterator::GetTime() const
{
	return wrapped->first;
}

CamPathValue CamPathIterator::GetValue() const
{
	return wrapped->second;
}

CamPathIterator& CamPathIterator::operator ++ ()
{
	wrapped++;
	return *this;
}

bool CamPathIterator::operator == (CamPathIterator const &it) const
{
	return wrapped == it.wrapped;
}

bool CamPathIterator::operator != (CamPathIterator const &it) const
{
	return !(*this == it);
}

// CamPath /////////////////////////////////////////////////////////////////////

CamPath::CamPath()
: m_OnChanged(0)
, m_Offset(0)
, m_Enabled(false)
, m_PositionInterpMethod(DI_DEFAULT)
, m_RotationInterpMethod(QI_DEFAULT)
, m_FovInterpMethod(DI_DEFAULT)
, m_XView(&m_Map, XSelector)
, m_YView(&m_Map, YSelector)
, m_ZView(&m_Map, ZSelector)
, m_RView(&m_Map, RSelector)
, m_FovView(&m_Map, FovSelector)
, m_SelectedView(&m_Map, SelectedSelector)
{
	m_XInterp = new CCubicDoubleInterpolation<CamPathValue>(&m_XView);
	m_YInterp = new CCubicDoubleInterpolation<CamPathValue>(&m_YView);
	m_ZInterp = new CCubicDoubleInterpolation<CamPathValue>(&m_ZView);
	m_RInterp = new CSCubicQuaternionInterpolation<CamPathValue>(&m_RView);
	m_FovInterp = new CCubicDoubleInterpolation<CamPathValue>(&m_FovView);
	m_SelectedInterp = new CBoolAndInterpolation<CamPathValue>(&m_SelectedView);

	Changed();
}

CamPath::~CamPath()
{
	m_Map.clear();

	Changed();

	delete m_SelectedInterp;
	delete m_FovInterp;
	delete m_RInterp;
	delete m_ZInterp;
	delete m_YInterp;
	delete m_XInterp;
}

void CamPath::DoInterpolationMapChangedAll(void)
{
	m_XInterp->InterpolationMapChanged();
	m_YInterp->InterpolationMapChanged();
	m_ZInterp->InterpolationMapChanged();
	m_RInterp->InterpolationMapChanged();
	m_FovInterp->InterpolationMapChanged();
	m_SelectedInterp->InterpolationMapChanged();
}

void CamPath::Enabled_set(bool enable)
{
	m_Enabled = enable;
}

bool CamPath::Enabled_get(void) const
{
	return m_Enabled;
}

bool CamPath::GetHold(void) const
{
	return m_Hold;
}

void CamPath::SetHold(bool value)
{
	m_Hold = value;
}

void CamPath::PositionInterpMethod_set(DoubleInterp value)
{
	delete m_XInterp;
	delete m_YInterp;
	delete m_ZInterp;

	m_PositionInterpMethod = value;

	switch(value)
	{
	case DI_LINEAR:
		m_XInterp = new CLinearDoubleInterpolation<CamPathValue>(&m_XView);
		m_YInterp = new CLinearDoubleInterpolation<CamPathValue>(&m_YView);
		m_ZInterp = new CLinearDoubleInterpolation<CamPathValue>(&m_ZView);
		break;
	default:
		m_XInterp = new CCubicDoubleInterpolation<CamPathValue>(&m_XView);
		m_YInterp = new CCubicDoubleInterpolation<CamPathValue>(&m_YView);
		m_ZInterp = new CCubicDoubleInterpolation<CamPathValue>(&m_ZView);
		break;
	}

	Changed();
}

CamPath::DoubleInterp CamPath::PositionInterpMethod_get(void) const
{
	return m_PositionInterpMethod;
}

void CamPath::RotationInterpMethod_set(QuaternionInterp value)
{
	delete m_RInterp;

	m_RotationInterpMethod = value;

	switch(value)
	{
	case QI_SLINEAR:
		m_RInterp = new CSLinearQuaternionInterpolation<CamPathValue>(&m_RView);
		break;
	default:
		m_RInterp = new CSCubicQuaternionInterpolation<CamPathValue>(&m_RView);
		break;
	}

	Changed();
}

CamPath::QuaternionInterp CamPath::RotationInterpMethod_get(void) const
{
	return m_RotationInterpMethod;
}

void CamPath::FovInterpMethod_set(DoubleInterp value)
{
	delete m_FovInterp;

	m_FovInterpMethod = value;

	switch(value)
	{
	case DI_LINEAR:
		m_FovInterp = new CLinearDoubleInterpolation<CamPathValue>(&m_FovView);
		break;
	default:
		m_FovInterp = new CCubicDoubleInterpolation<CamPathValue>(&m_FovView);
		break;
	}

	Changed();
}

CamPath::DoubleInterp CamPath::FovInterpMethod_get(void) const
{
	return m_FovInterpMethod;
}

void CamPath::Add(double time, const CamPathValue & value)
{
	m_Map[time] = value;
	DoInterpolationMapChangedAll();
	Changed();
}

void CamPath::Changed()
{
	if(m_OnChanged) m_OnChanged->CamPathChanged(this);
}

void CamPath::Remove(double time)
{
	m_Map.erase(time);
	DoInterpolationMapChangedAll();
	Changed();
}

void CamPath::Clear()
{
	bool selectAll = true;

	CInterpolationMap<CamPathValue>::iterator last = m_Map.end();
	for(CInterpolationMap<CamPathValue>::iterator it = m_Map.begin(); it != m_Map.end();)
	{
		CInterpolationMap<CamPathValue>::iterator itNext = it;
		++itNext;

		if(it->second.Selected)
		{
			selectAll = false;
			m_Map.erase(it);
		}

		it = itNext;
	}

	if(selectAll) m_Map.clear();

	m_Offset = 0;

	DoInterpolationMapChangedAll();
	Changed();
}

size_t CamPath::GetSize() const
{
	return m_Map.size();
}

CamPathIterator CamPath::GetBegin()
{
	return CamPathIterator(m_Map.begin());
}

CamPathIterator CamPath::GetEnd()
{
	return CamPathIterator(m_Map.end());
}

double CamPath::GetLowerBound() const
{
	return m_Map.cbegin()->first;
}

double CamPath::GetUpperBound() const
{
	return (--m_Map.cend())->first;
}

bool CamPath::CanEval(void) const
{
	return
		m_XInterp->CanEval()
		&& m_YInterp->CanEval()
		&& m_ZInterp->CanEval()
		&& m_RInterp->CanEval()
		&& m_FovInterp->CanEval()
		&& m_SelectedInterp->CanEval();
}

CamPathValue CamPath::Eval(double t)
{
	CamPathValue val;
	
	val.X = m_XInterp->Eval(t);
	val.Y = m_YInterp->Eval(t);
	val.Z = m_ZInterp->Eval(t);
	val.R = m_RInterp->Eval(t);
	val.Fov = m_FovInterp->Eval(t);
	val.Selected = m_SelectedInterp->Eval(t);

	return val;
}

void CamPath::OnChanged_set(ICamPathChanged * value)
{
	m_OnChanged = value;
}

char * double2xml(rapidxml::xml_document<> & doc, double value)
{
	char szTmp[196];
	_snprintf_s(szTmp, _TRUNCATE,"%f", value);
	return doc.allocate_string(szTmp);
}

bool CamPath::Save(wchar_t const * fileName)
{
	rapidxml::xml_document<> doc;

	rapidxml::xml_node<> * decl = doc.allocate_node(rapidxml::node_declaration);
	decl->append_attribute(doc.allocate_attribute("version", "1.0"));
	decl->append_attribute(doc.allocate_attribute("encoding", "utf-8"));
	doc.append_node(decl);

	rapidxml::xml_node<> * cam = doc.allocate_node(rapidxml::node_element, "campath");
	if(DI_DEFAULT != m_PositionInterpMethod)
		cam->append_attribute(doc.allocate_attribute("positionInterp", DoubleInterp_ToString(m_PositionInterpMethod)));
	if(QI_DEFAULT != m_RotationInterpMethod)
		cam->append_attribute(doc.allocate_attribute("rotationInterp", QuaternionInterp_ToString(m_RotationInterpMethod)));
	if(DI_DEFAULT != m_FovInterpMethod)
		cam->append_attribute(doc.allocate_attribute("fovInterp", DoubleInterp_ToString(m_FovInterpMethod)));
	if (m_Offset)
		cam->append_attribute(doc.allocate_attribute("offset", double2xml(doc, m_Offset)));
	if (m_Hold)
		cam->append_attribute(doc.allocate_attribute("hold"));
	doc.append_node(cam);

	rapidxml::xml_node<> * pts = doc.allocate_node(rapidxml::node_element, "points");
	cam->append_node(pts);

	rapidxml::xml_node<> * cmt = doc.allocate_node(rapidxml::node_comment,0,
		"Points are in Quake coordinates, meaning x=forward, y=left, z=up and rotation order is first rx, then ry and lastly rz.\n"
		"Rotation direction follows the right-hand grip rule.\n"
		"rx (roll), ry (pitch), rz(yaw) are the Euler angles in degrees.\n"
		"qw, qx, qy, qz are the quaternion values.\n"
		"When read it is sufficient that either rx, ry, rz OR qw, qx, qy, qz are present.\n"
		"If both are pesent then qw, qx, qy, qz take precedence."
	);
	pts->append_node(cmt);

	for(CamPathIterator it = GetBegin(); it != GetEnd(); ++it)
	{
		double time = it.GetTime();
		CamPathValue val = it.GetValue();
		QEulerAngles ang = val.R.ToQREulerAngles().ToQEulerAngles();

		rapidxml::xml_node<> * pt = doc.allocate_node(rapidxml::node_element, "p");
		pt->append_attribute(doc.allocate_attribute("t", double2xml(doc,time)));
		pt->append_attribute(doc.allocate_attribute("x", double2xml(doc,val.X)));
		pt->append_attribute(doc.allocate_attribute("y", double2xml(doc,val.Y)));
		pt->append_attribute(doc.allocate_attribute("z", double2xml(doc,val.Z)));
		pt->append_attribute(doc.allocate_attribute("fov", double2xml(doc,val.Fov)));
		pt->append_attribute(doc.allocate_attribute("rx", double2xml(doc,ang.Roll)));
		pt->append_attribute(doc.allocate_attribute("ry", double2xml(doc,ang.Pitch)));
		pt->append_attribute(doc.allocate_attribute("rz", double2xml(doc,ang.Yaw)));
		pt->append_attribute(doc.allocate_attribute("qw", double2xml(doc,it.wrapped->second.R.W)));
		pt->append_attribute(doc.allocate_attribute("qx", double2xml(doc,it.wrapped->second.R.X)));
		pt->append_attribute(doc.allocate_attribute("qy", double2xml(doc,it.wrapped->second.R.Y)));
		pt->append_attribute(doc.allocate_attribute("qz", double2xml(doc,it.wrapped->second.R.Z)));

		if(val.Selected)
			pt->append_attribute(doc.allocate_attribute("selected"));

		pts->append_node(pt);
	}

	std::string xmlString;
	rapidxml::print(std::back_inserter(xmlString), doc);

	std::ofstream ofs(fileName, std::ios_base::binary);

	bool bOk = !ofs.fail();

	if (bOk)
	{
		ofs << doc;
	}

	if (ofs.fail())
		bOk = false;

	ofs.close();
	
	return bOk;
}

bool CamPath::Load(wchar_t const * fileName)
{
	bool bOk = false;

	FILE * pFile = 0;

	_wfopen_s(&pFile, fileName, L"rb");

	if(!pFile)
		return false;
	
	fseek(pFile, 0, SEEK_END);
	size_t fileSize = ftell(pFile);
	rewind(pFile);

	char * pData = new char[fileSize+1];
	pData[fileSize] = 0;

	size_t readSize = fread(pData, sizeof(char), fileSize, pFile);
	bOk = readSize == fileSize;
	if(bOk)
	{
		try
		{
			do
			{
				rapidxml::xml_document<> doc;
				doc.parse<0>(pData);

				rapidxml::xml_node<> * cur_node = doc.first_node("campath");
				if(!cur_node) break;

				// Clear current Campath:
				SelectNone();
				Clear();

				rapidxml::xml_attribute<> * positionInterpA = cur_node->first_attribute("positionInterp");
				DoubleInterp positionInterp = DI_DEFAULT;
				if(positionInterpA) DoubleInterp_FromString(positionInterpA->value(), positionInterp);
				PositionInterpMethod_set(positionInterp);

				rapidxml::xml_attribute<> * rotationInterpA = cur_node->first_attribute("rotationInterp");
				QuaternionInterp rotationInterp = QI_DEFAULT;
				if(rotationInterpA) QuaternionInterp_FromString(rotationInterpA->value(), rotationInterp);
				RotationInterpMethod_set(rotationInterp);

				rapidxml::xml_attribute<> * fovInterpA = cur_node->first_attribute("fovInterp");
				DoubleInterp fovInterp = DI_DEFAULT;
				if(fovInterpA) DoubleInterp_FromString(fovInterpA->value(), fovInterp);
				FovInterpMethod_set(fovInterp);

				rapidxml::xml_attribute<>* offsetA = cur_node->first_attribute("offset");
				double offset = offsetA ? atof(offsetA->value()) : 0.0;
				SetOffset(offset);

				rapidxml::xml_attribute<> * holdA = cur_node->first_attribute("hold");
				bool bHold = nullptr != holdA;
				SetHold(bHold);

				cur_node = cur_node->first_node("points");
				if(!cur_node) break;

				for(cur_node = cur_node->first_node("p"); cur_node; cur_node = cur_node->next_sibling("p"))
				{
					rapidxml::xml_attribute<> * timeAttr = cur_node->first_attribute("t");
					if(!timeAttr) continue;

					rapidxml::xml_attribute<> * xA = cur_node->first_attribute("x");
					rapidxml::xml_attribute<> * yA = cur_node->first_attribute("y");
					rapidxml::xml_attribute<> * zA = cur_node->first_attribute("z");
					rapidxml::xml_attribute<> * fovA = cur_node->first_attribute("fov");
					rapidxml::xml_attribute<> * rxA = cur_node->first_attribute("rx");
					rapidxml::xml_attribute<> * ryA = cur_node->first_attribute("ry");
					rapidxml::xml_attribute<> * rzA = cur_node->first_attribute("rz");
					rapidxml::xml_attribute<> * qwA = cur_node->first_attribute("qw");
					rapidxml::xml_attribute<> * qxA = cur_node->first_attribute("qx");
					rapidxml::xml_attribute<> * qyA = cur_node->first_attribute("qy");
					rapidxml::xml_attribute<> * qzA = cur_node->first_attribute("qz");
					rapidxml::xml_attribute<> * selectedA = cur_node->first_attribute("selected");

					double dT = atof(timeAttr->value());
					double dX = xA ? atof(xA->value()) : 0.0;
					double dY = yA ? atof(yA->value()) : 0.0;
					double dZ = zA ? atof(zA->value()) : 0.0;
					double dFov = fovA ? atof(fovA->value()) : 90.0;

					if(qwA && qxA && qyA && qzA)
					{
						CamPathValue r;
						r.X = dX;
						r.Y = dY;
						r.Z = dZ;
						r.R.W = atof(qwA->value());
						r.R.X = atof(qxA->value());
						r.R.Y = atof(qyA->value());
						r.R.Z = atof(qzA->value());
						r.Fov = dFov;
						r.Selected = 0 != selectedA;

						// Add point:
						m_Map[dT] = r;
					}
					else
					{
						double dRXroll = rxA ? atof(rxA->value()) : 0.0;
						double dRYpitch = ryA ? atof(ryA->value()) : 0.0;
						double dRZyaw = rzA ? atof(rzA->value()) : 0.0;

						CamPathValue r;
						r.X = dX;
						r.Y = dY;
						r.Z = dZ;
						r.R = Quaternion::FromQREulerAngles(QREulerAngles::FromQEulerAngles(QEulerAngles(dRYpitch, dRZyaw, dRXroll)));
						r.Fov = dFov;
						r.Selected = 0 != selectedA;

						// Add point:
						m_Map[dT] = r;
					}
				}
			}
			while (false);
		}
		catch(rapidxml::parse_error &)
		{
			bOk=false;
		}
	}

	delete pData;

	fclose(pFile);

	DoInterpolationMapChangedAll();
	Changed();

	return bOk;
}

size_t CamPath::SelectAll()
{
	for(CInterpolationMap<CamPathValue>::iterator it = m_Map.begin(); it != m_Map.end(); ++it)
	{
		it->second.Selected = true;
	}

	m_SelectedInterp->InterpolationMapChanged();
	Changed();

	return m_Map.size();
}

void CamPath::SelectNone()
{
	for(CInterpolationMap<CamPathValue>::iterator it = m_Map.begin(); it != m_Map.end(); ++it)
	{
		it->second.Selected = false;
	}

	m_SelectedInterp->InterpolationMapChanged();
	Changed();
}

size_t CamPath::SelectInvert()
{
	size_t selected = 0;

	for(CInterpolationMap<CamPathValue>::iterator it = m_Map.begin(); it != m_Map.end(); ++it)
	{
		it->second.Selected = !it->second.Selected;

		if(it->second.Selected) ++selected;
	}

	m_SelectedInterp->InterpolationMapChanged();
	Changed();

	return selected;
}

size_t CamPath::SelectAdd(size_t min, size_t max)
{
	size_t i = 0;
	size_t selected = 0;

	for(CInterpolationMap<CamPathValue>::iterator it = m_Map.begin(); it != m_Map.end(); ++it)
	{
		it->second.Selected = it->second.Selected || min <= i && i <= max;

		if(it->second.Selected) ++selected;

		++i;
	}

	m_SelectedInterp->InterpolationMapChanged();
	Changed();

	return selected;
}

size_t CamPath::SelectAdd(double min, size_t count)
{
	size_t selected = 0;

	for(CInterpolationMap<CamPathValue>::iterator it = m_Map.begin(); it != m_Map.end(); ++it)
	{
		it->second.Selected = it->second.Selected || min <= it->first && selected < count;

		if(it->second.Selected) ++selected;
	}

	m_SelectedInterp->InterpolationMapChanged();
	Changed();

	return selected;
}

size_t CamPath::SelectAdd(double min, double max)
{
	size_t selected = 0;

	for(CInterpolationMap<CamPathValue>::iterator it = m_Map.begin(); it != m_Map.end(); ++it)
	{
		it->second.Selected = it->second.Selected || min <= it->first && it->first <= max;

		if(it->second.Selected) ++selected;
	}

	m_SelectedInterp->InterpolationMapChanged();
	Changed();

	return selected;
}

void CamPath::SetStart(double t, bool relative)
{
	if(m_Map.size()<1) return;

	CInterpolationMap<CamPathValue> tempMap;

	bool selectAll = true;
	double first = 0;

	for(CInterpolationMap<CamPathValue>::iterator it = m_Map.begin(); it != m_Map.end(); ++it)
	{
		if(it->second.Selected)
		{
			if(selectAll)
			{
				selectAll = false;
				first = it->first;
				break;
			}
		}
	}

	double deltaT = relative ? t : (selectAll ? t -m_Map.begin()->first : t -first);

	for(CInterpolationMap<CamPathValue>::iterator it = m_Map.begin(); it != m_Map.end(); ++it)
	{
		double curT = it->first;
		CamPathValue curValue = it->second;

		if(selectAll || curValue.Selected)
		{
			tempMap[deltaT+curT] = curValue;
		}
		else
		{
			tempMap[curT] = curValue;
		}
	}

	CopyMap(m_Map, tempMap);

	DoInterpolationMapChangedAll();

	Changed();
}
	
void CamPath::SetDuration(double t)
{
	if(m_Map.size()<2) return;

	CInterpolationMap<CamPathValue> tempMap;

	CopyMap(tempMap, m_Map);

	bool selectAll = true;
	double first = 0, last = 0;

	for(CInterpolationMap<CamPathValue>::iterator it = m_Map.begin(); it != m_Map.end(); ++it)
	{
		if(it->second.Selected)
		{
			if(selectAll)
			{
				selectAll = false;
				first = it->first;
				last = first;
			}
			else
			{
				last = it->first;
			}
		}
	}

	double oldDuration = selectAll ? GetDuration() : last -first;

	m_Map.clear();

	double scale = oldDuration ? t / oldDuration : 0.0;
	bool isFirst = true;
	double firstT = 0;

	for(CInterpolationMap<CamPathValue>::const_iterator it = tempMap.begin(); it != tempMap.end(); ++it)
	{
		double curT = it->first;
		CamPathValue curValue = it->second;

		if(selectAll || curValue.Selected)
		{
			if(isFirst)
			{
				m_Map[curT] = curValue;
				firstT = curT;
				isFirst = false;
			}
			else
				m_Map[firstT+scale*(curT-firstT)] = curValue;
		}
		else
			m_Map[curT] = curValue;
	}

	DoInterpolationMapChangedAll();

	Changed();
}

void CamPath::SetPosition(double x, double y, double z, bool setX, bool setY, bool setZ)
{
	if(m_Map.size()<1) return;

	bool selectAll = true;

	for(CInterpolationMap<CamPathValue>::const_iterator it = m_Map.begin(); it != m_Map.end(); ++it)
	{
		if(it->second.Selected)
		{
			if(selectAll)
			{
				selectAll = false;
				break;
			}
		}
	}

	// calcualte mid:

	double minX = 0, maxX = 0, minY = 0, maxY = 0, minZ = 0, maxZ = 0;
	bool first = true;

	for(CInterpolationMap<CamPathValue>::const_iterator it = m_Map.begin(); it != m_Map.end(); ++it)
	{
		if(selectAll || it->second.Selected)
		{
			CamPathValue curValue = it->second;

			if(first)
			{
				minX = curValue.X;
				minY = curValue.Y;
				minZ = curValue.Z;
				maxX = curValue.X;
				maxY = curValue.Y;
				maxZ = curValue.Z;
				first = false;
			}
			else
			{
				minX = std::min(minX, curValue.X);
				minY = std::min(minY, curValue.Y);
				minZ = std::min(minZ, curValue.Z);
				maxX = std::max(maxX, curValue.X);
				maxY = std::max(maxY, curValue.Y);
				maxZ = std::max(maxZ, curValue.Z);
			}
		}
	}

	double x0 = (maxX +minX) / 2;
	double y0 = (maxY +minY) / 2;
	double z0 = (maxZ +minZ) / 2;

	for(CInterpolationMap<CamPathValue>::iterator it = m_Map.begin(); it != m_Map.end(); ++it)
	{
		double curT = it->first;
		CamPathValue curValue = it->second;

		if(selectAll || curValue.Selected)
		{
			if(setX) curValue.X = x +(curValue.X -x0);
			if(setY) curValue.Y = y +(curValue.Y -y0);
			if(setZ) curValue.Z = z +(curValue.Z -z0);

			it->second = curValue;
		}
	}

	m_XInterp->InterpolationMapChanged();
	m_YInterp->InterpolationMapChanged();
	m_ZInterp->InterpolationMapChanged();

	Changed();
}

void CamPath::SetAngles(double yPitch, double zYaw, double xRoll, bool setY, bool setZ, bool setX)
{
	if(m_Map.size()<1) return;

	bool selectAll = true;

	for(CInterpolationMap<CamPathValue>::const_iterator it = m_Map.begin(); it != m_Map.end(); ++it)
	{
		if(it->second.Selected)
		{
			if(selectAll)
			{
				selectAll = false;
				break;
			}
		}
	}

	for(CInterpolationMap<CamPathValue>::iterator it = m_Map.begin(); it != m_Map.end(); ++it)
	{
		double curT = it->first;
		CamPathValue curValue = it->second;

		if(selectAll || curValue.Selected)
		{
			if(setY && setZ && setX) {
				curValue.R = Quaternion::FromQREulerAngles(QREulerAngles::FromQEulerAngles(QEulerAngles(yPitch, zYaw, xRoll)));
			} else {
				QEulerAngles angles = curValue.R.ToQREulerAngles().ToQEulerAngles();
				curValue.R = Quaternion::FromQREulerAngles(QREulerAngles::FromQEulerAngles(QEulerAngles(
					(setY ? yPitch : angles.Pitch),
					(setZ ? zYaw : angles.Yaw),
					(setX ? xRoll : angles.Roll)
				)));
			}

			it->second = curValue;
		}

	}

	m_RInterp->InterpolationMapChanged();

	Changed();
}

void CamPath::SetFov(double fov)
{
	if(m_Map.size()<1) return;

	bool selectAll = true;

	for(CInterpolationMap<CamPathValue>::const_iterator it = m_Map.begin(); it != m_Map.end(); ++it)
	{
		if(it->second.Selected)
		{
			if(selectAll)
			{
				selectAll = false;
				break;
			}
		}
	}

	for(CInterpolationMap<CamPathValue>::iterator it = m_Map.begin(); it != m_Map.end(); ++it)
	{
		double curT = it->first;
		CamPathValue curValue = it->second;

		if(selectAll || curValue.Selected)
		{
			curValue.Fov = fov;

			it->second = curValue;
		}

	}

	m_FovInterp->InterpolationMapChanged();

	Changed();
}

void CamPath::Rotate(double yPitch, double zYaw, double xRoll)
{
	if(m_Map.size()<1) return;

	bool selectAll = true;

	for(CInterpolationMap<CamPathValue>::const_iterator it = m_Map.begin(); it != m_Map.end(); ++it)
	{
		if(it->second.Selected)
		{
			if(selectAll)
			{
				selectAll = false;
				break;
			}
		}
	}

	// calcualte mid:

	double minX = 0, maxX = 0, minY = 0, maxY = 0, minZ = 0, maxZ = 0;
	bool first = true;

	for(CInterpolationMap<CamPathValue>::const_iterator it = m_Map.begin(); it != m_Map.end(); ++it)
	{
		if(selectAll || it->second.Selected)
		{
			CamPathValue curValue = it->second;

			if(first)
			{
				minX = curValue.X;
				minY = curValue.Y;
				minZ = curValue.Z;
				maxX = curValue.X;
				maxY = curValue.Y;
				maxZ = curValue.Z;
				first = false;
			}
			else
			{
				minX = std::min(minX, curValue.X);
				minY = std::min(minY, curValue.Y);
				minZ = std::min(minZ, curValue.Z);
				maxX = std::max(maxX, curValue.X);
				maxY = std::max(maxY, curValue.Y);
				maxZ = std::max(maxZ, curValue.Z);
			}
		}
	}

	double x0 = (maxX +minX) / 2;
	double y0 = (maxY +minY) / 2;
	double z0 = (maxZ +minZ) / 2;

	// build rotation matrix:
	double R[3][3];
	{
		double angle;
		double sr, sp, sy, cr, cp, cy;

		angle = zYaw * (M_PI*2 / 360);
		sy = sin(angle);
		cy = cos(angle);
		angle = yPitch * (M_PI*2 / 360);
		sp = sin(angle);
		cp = cos(angle);
		angle = xRoll * (M_PI*2 / 360);
		sr = sin(angle);
		cr = cos(angle);

		// R = YAW * (PITCH * ROLL)
		R[0][0] = cy*cp;
		R[0][1] = cy*sp*sr -sy*cr;
		R[0][2] = cy*sp*cr +sy*sr;
		R[1][0] = sy*cp;
		R[1][1] = sy*sp*sr +cy*cr;
		R[1][2] = sy*sp*cr +cy*-sr;
		R[2][0] = -sp;
		R[2][1] = cp*sr;
		R[2][2] = cp*cr;
	}
	Quaternion quatR = Quaternion::FromQREulerAngles(QREulerAngles::FromQEulerAngles(QEulerAngles(yPitch, zYaw, xRoll)));

	// rotate:

	for(CInterpolationMap<CamPathValue>::iterator it = m_Map.begin(); it != m_Map.end(); ++it)
	{
		double curT = it->first;
		CamPathValue curValue = it->second;

		if(selectAll || curValue.Selected)
		{
			// update position:
			{
				// translate into origin:
				double x = curValue.X -x0;
				double y = curValue.Y -y0;
				double z = curValue.Z -z0;

				// rotate:
				double Rx = R[0][0]*x +R[0][1]*y +R[0][2]*z;
				double Ry = R[1][0]*x +R[1][1]*y +R[1][2]*z;
				double Rz = R[2][0]*x +R[2][1]*y +R[2][2]*z;

				// translate back:
				curValue.X = Rx +x0;
				curValue.Y = Ry +y0;
				curValue.Z = Rz +z0;
			}

			// update rotation:
			{
				Quaternion quatQ = curValue.R;

				curValue.R = quatR * quatQ;
			}

			// update:
			it->second = curValue;
		}

	}

	m_XInterp->InterpolationMapChanged();
	m_YInterp->InterpolationMapChanged();
	m_ZInterp->InterpolationMapChanged();
	m_RInterp->InterpolationMapChanged();

	Changed();
}

void CamPath::AnchorTransform(double anchorX, double anchorY, double anchorZ, double anchorYPitch, double anchorZYaw, double anchorXRoll, double destX, double destY, double destZ, double destYPitch, double destZYaw, double destXRoll)
{
	if(m_Map.size()<1) return;

	bool selectAll = true;

	for(CInterpolationMap<CamPathValue>::const_iterator it = m_Map.begin(); it != m_Map.end(); ++it)
	{
		if(it->second.Selected)
		{
			if(selectAll)
			{
				selectAll = false;
				break;
			}
		}
	}

	Quaternion quatAnchor = Quaternion::FromQREulerAngles(QREulerAngles::FromQEulerAngles(QEulerAngles(anchorYPitch, anchorZYaw, anchorXRoll)));
	Quaternion quatDest = Quaternion::FromQREulerAngles(QREulerAngles::FromQEulerAngles(QEulerAngles(destYPitch, destZYaw, destXRoll)));

	// Make sure we take the shortest path:
	double dotProduct = DotProduct(quatDest, quatAnchor);
	if (dotProduct<0.0)
	{
		quatAnchor = -1.0 * quatAnchor;
	}

	Quaternion quatR = quatDest * (/*(1.0 / quatAnchor.Norm()) * */quatAnchor.Conjugate());

	QEulerAngles angles = quatR.ToQREulerAngles().ToQEulerAngles();

	double yPitch = angles.Pitch;
	double zYaw = angles.Yaw;
	double xRoll = angles.Roll;

	// build rotation matrix:
	double R[3][3];
	{
		double angle;
		double sr, sp, sy, cr, cp, cy;

		angle = zYaw * (M_PI*2 / 360);
		sy = sin(angle);
		cy = cos(angle);
		angle = yPitch * (M_PI*2 / 360);
		sp = sin(angle);
		cp = cos(angle);
		angle = xRoll * (M_PI*2 / 360);
		sr = sin(angle);
		cr = cos(angle);

		// R = YAW * (PITCH * ROLL)
		R[0][0] = cy*cp;
		R[0][1] = cy*sp*sr -sy*cr;
		R[0][2] = cy*sp*cr +sy*sr;
		R[1][0] = sy*cp;
		R[1][1] = sy*sp*sr +cy*cr;
		R[1][2] = sy*sp*cr +cy*-sr;
		R[2][0] = -sp;
		R[2][1] = cp*sr;
		R[2][2] = cp*cr;
	}

	// rotate:

	for(CInterpolationMap<CamPathValue>::iterator it = m_Map.begin(); it != m_Map.end(); ++it)
	{
		double curT = it->first;
		CamPathValue curValue = it->second;

		if(selectAll || curValue.Selected)
		{
			// update position:
			{
				// translate into anchor:
				double x = curValue.X -anchorX;
				double y = curValue.Y -anchorY;
				double z = curValue.Z -anchorZ;

				// rotate:
				double Rx = R[0][0]*x +R[0][1]*y +R[0][2]*z;
				double Ry = R[1][0]*x +R[1][1]*y +R[1][2]*z;
				double Rz = R[2][0]*x +R[2][1]*y +R[2][2]*z;

				// translate into destination:
				curValue.X = Rx +destX;
				curValue.Y = Ry +destY;
				curValue.Z = Rz +destZ;
			}

			// update rotation:
			{
				Quaternion quatQ = curValue.R;

				curValue.R = quatR * quatQ;
			}

			// update:
			it->second = curValue;
		}

	}

	m_XInterp->InterpolationMapChanged();
	m_YInterp->InterpolationMapChanged();
	m_ZInterp->InterpolationMapChanged();
	m_RInterp->InterpolationMapChanged();

	Changed();
}

void CamPath::CopyMap(CInterpolationMap<CamPathValue> & dst, CInterpolationMap<CamPathValue> & src)
{
	dst.clear();

	for(CInterpolationMap<CamPathValue>::const_iterator it = src.begin(); it != src.end(); ++it)
	{
		dst[it->first] = it->second;
	}
}

double CamPath::GetDuration() const
{
	if(m_Map.size()<2) return 0.0;

	return (--m_Map.cend())->first - m_Map.cbegin()->first;
}

void CamPath::SetOffset(double value)
{
	m_Offset = value;

	Changed();
}

double CamPath::GetOffset() const
{
	return m_Offset;
}