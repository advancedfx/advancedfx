#pragma once

#include "SourceInterfaces.h"

#include "WrpConsole.h"

#include <list>

void CalcSmooth(double deltaT, double targetPos, double & lastPos, double & lastVel, double LimitVelocity, double LimitAcceleration);

class IMirvCalc abstract
{
public:
	virtual void AddRef(void) abstract = 0;
	virtual void Release(void) abstract = 0;;

	virtual int GetRefCount(void) abstract = 0;

	virtual  char const * GetName(void) abstract = 0;

	virtual void Console_PrintBegin(void) abstract = 0;
	virtual void Console_PrintEnd(void) abstract = 0;

	virtual void Console_Edit(IWrpCommandArgs * args) abstract = 0;
};

class IMirvHandleCalc abstract : public IMirvCalc
{
public:
	virtual bool CalcHandle(SOURCESDK::CSGO::CBaseHandle & outHandle) abstract = 0;
};

class IMirvVecAngCalc abstract : public IMirvCalc
{
public:
	virtual bool CalcVecAng(SOURCESDK::Vector & outVector, SOURCESDK::QAngle & outAngles) abstract = 0;
};

class IMirvCamCalc abstract : public IMirvCalc
{
public:
	virtual bool CalcCam(SOURCESDK::Vector & outVector, SOURCESDK::QAngle & outAngles, float & outFov) abstract = 0;
};

class IMirvFovCalc abstract : public IMirvCalc
{
public:
	virtual bool CalcFov(float & outFov) abstract = 0;
};

class IMirvBoolCalc abstract : public IMirvCalc
{
public:
	virtual bool CalcBool(bool & outResult) abstract = 0;
};

class IMirvIntCalc abstract : public IMirvCalc
{
public:
	virtual bool CalcInt(int & outResult) abstract = 0;
};

class IMirvFloatCalc abstract : public IMirvCalc
{
public:
	virtual bool CalcFloat(float& outResult) abstract = 0;
};

class CMirvHandleCalcs
{
public:
	~CMirvHandleCalcs();

	IMirvHandleCalc * GetByName(char const * name);

	IMirvHandleCalc * NewValueCalc(char const * name, int handle);
	IMirvHandleCalc * NewIndexCalc(char const * name, int entityIndex);
	IMirvHandleCalc * NewKeyCalc(char const * name, int slot);
	IMirvHandleCalc * NewActiveWeaponCalc(char const * name, IMirvHandleCalc * parent, bool world);
	IMirvHandleCalc * NewLocalPlayerCalc(char const * name);
	IMirvHandleCalc * NewObserverTargetCalc(char const * name, IMirvHandleCalc * parent);
	IMirvHandleCalc * NewOrCalc(char const * name, IMirvHandleCalc * a, IMirvHandleCalc * b);
	IMirvHandleCalc * NewOwnerEntityCalc(char const * name, IMirvHandleCalc * calc, unsigned int maxDepth = 1);

	bool Console_CheckName(char const * name);
	void Console_Remove(char const * name);
	void Console_Print(void);

private:
	std::list<IMirvHandleCalc *> m_Calcs;

	void GetIteratorByName(char const * name, std::list<IMirvHandleCalc *>::iterator & outIt);
};

extern CMirvHandleCalcs g_MirvHandleCalcs;


class CMirvVecAngCalcs
{
public:
	~CMirvVecAngCalcs();

	IMirvVecAngCalc * GetByName(char const * name);

	IMirvVecAngCalc * NewValueCalc(char const * name, float x, float y, float z, float rX, float rY, float rZ);
	IMirvVecAngCalc * NewAddCalc(char const * name, IMirvVecAngCalc * a, IMirvVecAngCalc * b);
	IMirvVecAngCalc * NewSubtractCalc(char const * name, IMirvVecAngCalc * a, IMirvVecAngCalc * b);
	IMirvVecAngCalc * NewOffsetCalc(char const * name, IMirvVecAngCalc * parent, IMirvVecAngCalc * offset, bool order);
	IMirvVecAngCalc * NewHandleCalc(char const * name, IMirvHandleCalc * handle);
	IMirvVecAngCalc * NewHandleEyeCalc(char const * name, IMirvHandleCalc * handle);
	IMirvVecAngCalc * NewHandleCalcEx(char const * name, IMirvHandleCalc * handle, bool eyeVec, bool eyeAng);
	IMirvVecAngCalc * NewHandleAttachmentCalc(char const * name, IMirvHandleCalc * handle, char const * attachmentName);
	IMirvVecAngCalc * NewIfCalc(char const * name, IMirvBoolCalc * condition, IMirvVecAngCalc * condTrue, IMirvVecAngCalc * condFalse);
	IMirvVecAngCalc * NewOrCalc(char const * name, IMirvVecAngCalc * a, IMirvVecAngCalc * b);
	IMirvVecAngCalc * NewCamCalc(char const * name, IMirvCamCalc * src);
	IMirvVecAngCalc * NewMotionProfile2Calc(char const * name, IMirvVecAngCalc * parent, IMirvHandleCalc * trackHandle);
	IMirvVecAngCalc * NewSwitchInterpCalc(char const * name, IMirvVecAngCalc * source, IMirvHandleCalc * switchHandle, IMirvHandleCalc * resetHandle, float holdTime, float interpTime);
	IMirvVecAngCalc * NewLocalToGlobalCalc(char const * name, IMirvVecAngCalc * source, IMirvHandleCalc * handle);
	IMirvVecAngCalc * NewGlobalToLocalCalc(char const * name, IMirvVecAngCalc * source, IMirvHandleCalc * handle);
	IMirvVecAngCalc * NewCollisionClipCalc(char const * name, IMirvVecAngCalc * source, IMirvVecAngCalc * clipTo, IMirvHandleCalc * ignoreHandle, float safeZoneDist);

	bool Console_CheckName(char const * name);
	void Console_Remove(char const * name);
	void Console_Print(void);

private:
	std::list<IMirvVecAngCalc *> m_Calcs;

	void GetIteratorByName(char const * name, std::list<IMirvVecAngCalc *>::iterator & outIt);
};

extern CMirvVecAngCalcs g_MirvVecAngCalcs;


class CMirvCamCalcs
{
public:
	~CMirvCamCalcs();

	IMirvCamCalc * GetByName(char const * name);

	IMirvCamCalc * NewCamCalc(char const * name, const char * camFileName, const char * startClientTime);
	IMirvCamCalc * NewGameCalc(char const * name);
	IMirvCamCalc * NewCurrentCalc(char const * name);
	IMirvCamCalc * NewPlayerCalc(char const* name, IMirvHandleCalc* handleCalc);
	IMirvCamCalc * NewSmoothCalc(char const * name, IMirvCamCalc * parent, IMirvHandleCalc * trackHandle);
	IMirvCamCalc * NewVecAngFovCalc(char const * name, IMirvVecAngCalc * vecAng, IMirvFovCalc * fov);

	bool Console_CheckName(char const * name);
	void Console_Remove(char const * name);
	void Console_Print(void);

private:
	std::list<IMirvCamCalc *> m_Calcs;

	void GetIteratorByName(char const * name, std::list<IMirvCamCalc *>::iterator & outIt);
};

extern CMirvCamCalcs g_MirvCamCalcs;


class CMirvFovCalcs
{
public:
	~CMirvFovCalcs();

	IMirvFovCalc * GetByName(char const * name);

	IMirvFovCalc * NewCamCalc(char const * name, IMirvCamCalc * src);

	bool Console_CheckName(char const * name);
	void Console_Remove(char const * name);
	void Console_Print(void);

private:
	std::list<IMirvFovCalc *> m_Calcs;

	void GetIteratorByName(char const * name, std::list<IMirvFovCalc *>::iterator & outIt);
};

extern CMirvFovCalcs g_MirvFovCalcs;



class CMirvBoolCalcs
{
public:
	~CMirvBoolCalcs();

	IMirvBoolCalc * GetByName(char const * name);

	IMirvBoolCalc * NewHandleCalc(char const * name, IMirvHandleCalc * handle);
	IMirvBoolCalc * NewVecAngCalc(char const * name, IMirvVecAngCalc * vecAng);
	IMirvBoolCalc * NewAliveCalc(char const * name, IMirvHandleCalc * vecAng);
	IMirvBoolCalc * NewAndCalc(char const * name, IMirvBoolCalc * a, IMirvBoolCalc * b);
	IMirvBoolCalc * NewOrCalc(char const * name, IMirvBoolCalc * a, IMirvBoolCalc * b);
	IMirvBoolCalc * NewNotCalc(char const * name, IMirvBoolCalc * a);
	IMirvBoolCalc * NewHandlesEqualCalc(char const * name, IMirvHandleCalc * a, IMirvHandleCalc * b);
	IMirvBoolCalc * NewIntsEqualCalc(char const * name, IMirvIntCalc * a, IMirvIntCalc * b);
	IMirvBoolCalc * NewClassnameMatchesCalc(char const * name, IMirvHandleCalc * handle, const char * wildCardString);

	bool Console_CheckName(char const * name);
	void Console_Remove(char const * name);
	void Console_Print(void);

private:
	std::list<IMirvBoolCalc *> m_Calcs;

	void GetIteratorByName(char const * name, std::list<IMirvBoolCalc *>::iterator & outIt);
};

extern CMirvBoolCalcs g_MirvBoolCalcs;


class CMirvIntCalcs
{
public:
	~CMirvIntCalcs();

	IMirvIntCalc * GetByName(char const * name);

	IMirvIntCalc * NewValueCalc(const char * name, int value);
	IMirvIntCalc * NewTeamNumberCalc(char const * name, IMirvHandleCalc * handle);

	bool Console_CheckName(char const * name);
	void Console_Remove(char const * name);
	void Console_Print(void);

private:
	std::list<IMirvIntCalc *> m_Calcs;

	void GetIteratorByName(char const * name, std::list<IMirvIntCalc *>::iterator & outIt);
};

extern CMirvIntCalcs g_MirvIntCalcs;


class CMirvFloatCalcs
{
public:
	~CMirvFloatCalcs();

	IMirvFloatCalc* GetByName(char const* name);

	IMirvFloatCalc* NewValueCalc(const char* name, float value);
	IMirvFloatCalc* NewIfCalc(char const* name, IMirvBoolCalc* condition, IMirvFloatCalc* condTrue, IMirvFloatCalc* condFalse);

	bool Console_CheckName(char const* name);
	void Console_Remove(char const* name);
	void Console_Print(void);

private:
	std::list<IMirvFloatCalc*> m_Calcs;

	void GetIteratorByName(char const* name, std::list<IMirvFloatCalc*>::iterator& outIt);
};

extern CMirvFloatCalcs g_MirvFloatCalcs;

void MirvCalcs_LevelInitPreEntity();

