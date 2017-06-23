#pragma once

// Copyright (c) advancedfx.org
//
// Last changes:
// 2016-11-06 dominik.matrixstorm.com
//
// First changes:
// 2016-11-06 dominik.matrixstorm.com

#include "SourceInterfaces.h"

class CAfxCsgoPrediction : public SOURCESDK::CSGO::IPrediction
{
public:
	CAfxCsgoPrediction(SOURCESDK::CSGO::IPrediction * parent);

	virtual ~CAfxCsgoPrediction();

	//
	// IPrediction:

	//virtual			~IPrediction(void) {};

	virtual void Init(void);
	virtual void Shutdown(void);
	virtual void Update(int startframe, bool validframe, int incoming_acknowledged,	 int outgoing_command);
	virtual void PreEntityPacketReceived(int commands_acknowledged, int current_world_update_packet);
	virtual void PostEntityPacketReceived(void);
	virtual void PostNetworkDataReceived(int commands_acknowledged);

	virtual void OnReceivedUncompressedPacket(void);

	virtual void GetViewOrigin(SOURCESDK::Vector& org);
	virtual void SetViewOrigin(SOURCESDK::Vector& org);
	virtual void GetViewAngles(SOURCESDK::QAngle& ang);
	virtual void SetViewAngles(SOURCESDK::QAngle& ang);
	virtual void GetLocalViewAngles(SOURCESDK::QAngle& ang);
	virtual void SetLocalViewAngles(SOURCESDK::QAngle& ang);
	
private:
	SOURCESDK::CSGO::IPrediction * m_Parent;
};

extern CAfxCsgoPrediction * g_AfxCsgoPrediction;

extern bool g_csgo_DuckFix;

bool csgo_CanDuckFix(void);