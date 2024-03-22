#pragma once

// Description:
// Wrapper(s) for Source's IVEngineClient Interface

#include "SourceInterfaces.h"
#include "insurgency2/public/cdll_int.h"
#include "bm/sdk_src/public/cdll_int.h"
#include "l4d2/sdk_src/public/cdll_int.h"


// WrpVEngineClientDemoInfo ///////////////////////////////////////////////////

class WrpVEngineClientDemoInfoEx abstract
{
public:
	virtual int GetDemoRecordingTick( void ) abstract = 0; 
	virtual int	GetDemoPlaybackTick( void ) abstract = 0; 
	virtual int	GetDemoPlaybackStartTick( void ) abstract = 0; 
	virtual float GetDemoPlaybackTimeScale( void ) abstract= 0; 
	virtual int GetDemoPlaybackTotalTicks( void ) abstract = 0; 
};


// WrpVEngineClient ///////////////////////////////////////////////////////////

class WrpVEngineClient abstract {
public:
	virtual void GetScreenSize( int& width, int& height ) abstract = 0;
	virtual void ServerCmd( const char *szCmdString, bool bReliable = true ) abstract = 0;
	virtual void ClientCmd( const char *szCmdString ) abstract = 0;
	virtual bool Con_IsVisible( void ) abstract = 0;
	virtual int	GetLocalPlayer(void) abstract = 0;
	virtual void GetViewAngles( SOURCESDK::QAngle& va ) abstract = 0;
	virtual void SetViewAngles( SOURCESDK::QAngle& va ) abstract = 0;
	virtual int GetMaxClients( void ) abstract = 0;
	virtual bool IsInGame( void ) abstract = 0;
	virtual bool IsConnected( void ) abstract = 0;
	virtual bool IsDrawingLoadingImage( void ) abstract = 0;
	virtual void Con_NPrintf( int pos, const char *fmt, ... ) abstract = 0;
	virtual const char *  GetGameDirectory( void ) abstract = 0;
	virtual const SOURCESDK::VMatrix& WorldToScreenMatrix() abstract = 0;
	virtual const SOURCESDK::VMatrix& WorldToViewMatrix() abstract = 0;
	virtual char const *  GetLevelName( void ) abstract = 0;
	virtual void EngineStats_BeginFrame( void ) abstract = 0;
	virtual void EngineStats_EndFrame( void ) abstract = 0;
	virtual bool IsPlayingDemo( void ) abstract = 0;
	virtual bool IsRecordingDemo( void ) abstract = 0;
	virtual bool IsPlayingTimeDemo( void ) abstract = 0;
	virtual bool IsPaused( void ) abstract = 0;
	virtual bool IsTakingScreenshot( void ) abstract = 0;
	virtual bool IsHLTV( void ) abstract = 0;
	virtual bool IsLevelMainMenuBackground( void ) abstract = 0;
	virtual void GetMainMenuBackgroundName( char *dest, int destlen ) abstract = 0;
	virtual bool IsInEditMode( void ) abstract = 0;
	virtual unsigned int GetEngineBuildNumber() abstract = 0;
	virtual const char * GetProductVersionString() abstract = 0;
	virtual bool IsHammerRunning( ) const abstract = 0;
	virtual void ExecuteClientCmd( const char *szCmdString ) abstract = 0;
	virtual void ClientCmd_Unrestricted( const char *szCmdString ) abstract = 0;

	/// <returns>0 if not available WrpVEngineClientDemoInfoEx interface otherwise</returns>
	virtual WrpVEngineClientDemoInfoEx * GetDemoInfoEx(void) abstract = 0;

	virtual SOURCESDK::IVEngineClient_014_csgo * GetVEngineClient_csgo(void)
	{
		return 0;
	}

	virtual SOURCESDK::IVEngineClient_013 * GetVEngineClient_css(void)
	{
		return 0;
	}
};


// WrpVEngineClient_012 ////////////////////////////////////////////////////////

class WrpVEngineClient_012 : public WrpVEngineClient
{
public:
	WrpVEngineClient_012(SOURCESDK::IVEngineClient_012 * iface);

	virtual void GetScreenSize( int& width, int& height );
	virtual void ServerCmd( const char *szCmdString, bool bReliable = true );
	virtual void ClientCmd( const char *szCmdString );
	virtual bool Con_IsVisible( void );
	virtual int	GetLocalPlayer(void);
	virtual void GetViewAngles(SOURCESDK::QAngle& va );
	virtual void SetViewAngles(SOURCESDK::QAngle& va );
	virtual int GetMaxClients( void );
	virtual bool IsInGame( void );
	virtual bool IsConnected( void );
	virtual bool IsDrawingLoadingImage( void );
	virtual void Con_NPrintf( int pos, const char *fmt, ... );
	virtual const char *  GetGameDirectory( void );
	virtual const SOURCESDK::VMatrix& WorldToScreenMatrix();
	virtual const SOURCESDK::VMatrix& WorldToViewMatrix();
	virtual char const *  GetLevelName( void );
	virtual void EngineStats_BeginFrame( void );
	virtual void EngineStats_EndFrame( void );
	virtual bool IsPlayingDemo( void );
	virtual bool IsRecordingDemo( void );
	virtual bool IsPlayingTimeDemo( void );
	virtual bool IsPaused( void );
	virtual bool IsTakingScreenshot( void );
	virtual bool IsHLTV( void );
	virtual bool IsLevelMainMenuBackground( void );
	virtual void GetMainMenuBackgroundName( char *dest, int destlen );
	virtual bool IsInEditMode( void );
	virtual unsigned int GetEngineBuildNumber();
	virtual const char * GetProductVersionString();
	virtual bool IsHammerRunning( ) const;
	virtual void ExecuteClientCmd( const char *szCmdString );
	virtual void ClientCmd_Unrestricted( const char *szCmdString );

	virtual WrpVEngineClientDemoInfoEx * GetDemoInfoEx(void);

private:
	SOURCESDK::IVEngineClient_012 * m_VEngineClient_012;
};

// WrpVEngineClient_013 ////////////////////////////////////////////////////////

class WrpVEngineClient_013
: public WrpVEngineClient
, public WrpVEngineClientDemoInfoEx
{
public:
	WrpVEngineClient_013(SOURCESDK::IVEngineClient_013 * iface);

	virtual void GetScreenSize( int& width, int& height );
	virtual void ServerCmd( const char *szCmdString, bool bReliable = true );
	virtual void ClientCmd( const char *szCmdString );
	virtual bool Con_IsVisible( void );
	virtual int	GetLocalPlayer(void);
	virtual void GetViewAngles(SOURCESDK::QAngle& va );
	virtual void SetViewAngles(SOURCESDK::QAngle& va );
	virtual int GetMaxClients( void );
	virtual bool IsInGame( void );
	virtual bool IsConnected( void );
	virtual bool IsDrawingLoadingImage( void );
	virtual void Con_NPrintf( int pos, const char *fmt, ... );
	virtual const char *  GetGameDirectory( void );
	virtual const SOURCESDK::VMatrix& WorldToScreenMatrix();
	virtual const SOURCESDK::VMatrix& WorldToViewMatrix();
	virtual char const *  GetLevelName( void );
	virtual void EngineStats_BeginFrame( void );
	virtual void EngineStats_EndFrame( void );
	virtual bool IsPlayingDemo( void );
	virtual bool IsRecordingDemo( void );
	virtual bool IsPlayingTimeDemo( void );

	virtual int GetDemoRecordingTick( void ); 
	virtual int	GetDemoPlaybackTick( void ); 
	virtual int	GetDemoPlaybackStartTick( void ); 
	virtual float GetDemoPlaybackTimeScale( void ); 
	virtual int GetDemoPlaybackTotalTicks( void ); 

	virtual bool IsPaused( void );
	virtual bool IsTakingScreenshot( void );
	virtual bool IsHLTV( void );
	virtual bool IsLevelMainMenuBackground( void );
	virtual void GetMainMenuBackgroundName( char *dest, int destlen );
	virtual bool IsInEditMode( void );
	virtual unsigned int GetEngineBuildNumber();
	virtual const char * GetProductVersionString();
	virtual bool IsHammerRunning( ) const;
	virtual void ExecuteClientCmd( const char *szCmdString );
	virtual void ClientCmd_Unrestricted( const char *szCmdString );

	virtual WrpVEngineClientDemoInfoEx * GetDemoInfoEx(void);

	
	virtual SOURCESDK::IVEngineClient_013 * GetVEngineClient_css(void)
	{
		return m_VEngineClient_013;
	}

private:
	SOURCESDK::IVEngineClient_013 * m_VEngineClient_013;
};

// WrpVEngineClient_014_csgo ///////////////////////////////////////////////////

class WrpVEngineClient_014_csgo
: public WrpVEngineClient
, public WrpVEngineClientDemoInfoEx
{
public:
	WrpVEngineClient_014_csgo(SOURCESDK::IVEngineClient_014_csgo * iface);

	virtual void GetScreenSize( int& width, int& height );
	virtual void ServerCmd( const char *szCmdString, bool bReliable = true );
	virtual void ClientCmd( const char *szCmdString );
	virtual bool Con_IsVisible( void );
	virtual int	GetLocalPlayer(void);
	virtual void GetViewAngles(SOURCESDK::QAngle& va );
	virtual void SetViewAngles(SOURCESDK::QAngle& va );
	virtual int GetMaxClients( void );
	virtual bool IsInGame( void );
	virtual bool IsConnected( void );
	virtual bool IsDrawingLoadingImage( void );
	virtual void Con_NPrintf( int pos, const char *fmt, ... );
	virtual const char *  GetGameDirectory( void );
	virtual const SOURCESDK::VMatrix& WorldToScreenMatrix();
	virtual const SOURCESDK::VMatrix& WorldToViewMatrix();
	virtual char const *  GetLevelName( void );
	virtual void EngineStats_BeginFrame( void );
	virtual void EngineStats_EndFrame( void );
	virtual bool IsPlayingDemo( void );
	virtual bool IsRecordingDemo( void );
	virtual bool IsPlayingTimeDemo( void );

	virtual int GetDemoRecordingTick( void ); 
	virtual int	GetDemoPlaybackTick( void ); 
	virtual int	GetDemoPlaybackStartTick( void ); 
	virtual float GetDemoPlaybackTimeScale( void ); 
	virtual int GetDemoPlaybackTotalTicks( void ); 

	virtual bool IsPaused( void );
	virtual bool IsTakingScreenshot( void );
	virtual bool IsHLTV( void );
	virtual bool IsLevelMainMenuBackground( void );
	virtual void GetMainMenuBackgroundName( char *dest, int destlen );
	virtual bool IsInEditMode( void );
	virtual unsigned int GetEngineBuildNumber();
	virtual const char * GetProductVersionString();
	virtual bool IsHammerRunning( ) const;
	virtual void ExecuteClientCmd( const char *szCmdString );
	virtual void ClientCmd_Unrestricted( const char *szCmdString );

	virtual WrpVEngineClientDemoInfoEx * GetDemoInfoEx(void);

	virtual SOURCESDK::IVEngineClient_014_csgo * GetVEngineClient_csgo(void)
	{
		return m_VEngineClient;
	}

private:
	SOURCESDK::IVEngineClient_014_csgo * m_VEngineClient;
};

// WrpVEngineClient_Insurgency2 ////////////////////////////////////////////////

class WrpVEngineClient_Insurgency2
	: public WrpVEngineClient
	, public WrpVEngineClientDemoInfoEx
{
public:
	WrpVEngineClient_Insurgency2(SOURCESDK::INSURGENCY2::IVEngineClient * iface);

	virtual void GetScreenSize(int& width, int& height);
	virtual void ServerCmd(const char *szCmdString, bool bReliable = true);
	virtual void ClientCmd(const char *szCmdString);
	virtual bool Con_IsVisible(void);
	virtual int	GetLocalPlayer(void);
	virtual void GetViewAngles(SOURCESDK::QAngle& va);
	virtual void SetViewAngles(SOURCESDK::QAngle& va);
	virtual int GetMaxClients(void);
	virtual bool IsInGame(void);
	virtual bool IsConnected(void);
	virtual bool IsDrawingLoadingImage(void);
	virtual void Con_NPrintf(int pos, const char *fmt, ...);
	virtual const char *  GetGameDirectory(void);
	virtual const SOURCESDK::VMatrix& WorldToScreenMatrix();
	virtual const SOURCESDK::VMatrix& WorldToViewMatrix();
	virtual char const *  GetLevelName(void);
	virtual void EngineStats_BeginFrame(void);
	virtual void EngineStats_EndFrame(void);
	virtual bool IsPlayingDemo(void);
	virtual bool IsRecordingDemo(void);
	virtual bool IsPlayingTimeDemo(void);

	virtual int GetDemoRecordingTick(void);
	virtual int	GetDemoPlaybackTick(void);
	virtual int	GetDemoPlaybackStartTick(void);
	virtual float GetDemoPlaybackTimeScale(void);
	virtual int GetDemoPlaybackTotalTicks(void);

	virtual bool IsPaused(void);
	virtual bool IsTakingScreenshot(void);
	virtual bool IsHLTV(void);
	virtual bool IsLevelMainMenuBackground(void);
	virtual void GetMainMenuBackgroundName(char *dest, int destlen);
	virtual bool IsInEditMode(void);
	virtual unsigned int GetEngineBuildNumber();
	virtual const char * GetProductVersionString();
	virtual bool IsHammerRunning() const;
	virtual void ExecuteClientCmd(const char *szCmdString);
	virtual void ClientCmd_Unrestricted(const char *szCmdString);

	virtual WrpVEngineClientDemoInfoEx * GetDemoInfoEx(void);

	virtual SOURCESDK::IVEngineClient_014_csgo * GetVEngineClient_csgo(void)
	{
		return 0;
	}

private:
	SOURCESDK::INSURGENCY2::IVEngineClient * m_VEngineClient;
};

// WrpVEngineClient_bm ///////////////////////////////////////////////////

class WrpVEngineClient_bm
	: public WrpVEngineClient
	, public WrpVEngineClientDemoInfoEx
{
public:
	WrpVEngineClient_bm(SOURCESDK::BM::IVEngineClient * iface);

	virtual void GetScreenSize(int& width, int& height);
	virtual void ServerCmd(const char *szCmdString, bool bReliable = true);
	virtual void ClientCmd(const char *szCmdString);
	virtual bool Con_IsVisible(void);
	virtual int	GetLocalPlayer(void);
	virtual void GetViewAngles(SOURCESDK::QAngle& va);
	virtual void SetViewAngles(SOURCESDK::QAngle& va);
	virtual int GetMaxClients(void);
	virtual bool IsInGame(void);
	virtual bool IsConnected(void);
	virtual bool IsDrawingLoadingImage(void);
	virtual void Con_NPrintf(int pos, const char *fmt, ...);
	virtual const char *  GetGameDirectory(void);
	virtual const SOURCESDK::VMatrix& WorldToScreenMatrix();
	virtual const SOURCESDK::VMatrix& WorldToViewMatrix();
	virtual char const *  GetLevelName(void);
	virtual void EngineStats_BeginFrame(void);
	virtual void EngineStats_EndFrame(void);
	virtual bool IsPlayingDemo(void);
	virtual bool IsRecordingDemo(void);
	virtual bool IsPlayingTimeDemo(void);

	virtual int GetDemoRecordingTick(void);
	virtual int	GetDemoPlaybackTick(void);
	virtual int	GetDemoPlaybackStartTick(void);
	virtual float GetDemoPlaybackTimeScale(void);
	virtual int GetDemoPlaybackTotalTicks(void);

	virtual bool IsPaused(void);
	virtual bool IsTakingScreenshot(void);
	virtual bool IsHLTV(void);
	virtual bool IsLevelMainMenuBackground(void);
	virtual void GetMainMenuBackgroundName(char *dest, int destlen);
	virtual bool IsInEditMode(void);
	virtual unsigned int GetEngineBuildNumber();
	virtual const char * GetProductVersionString();
	virtual bool IsHammerRunning() const;
	virtual void ExecuteClientCmd(const char *szCmdString);
	virtual void ClientCmd_Unrestricted(const char *szCmdString);

	virtual WrpVEngineClientDemoInfoEx * GetDemoInfoEx(void);

	virtual SOURCESDK::IVEngineClient_014_csgo * GetVEngineClient_csgo(void)
	{
		return 0;
	}

private:
	SOURCESDK::BM::IVEngineClient * m_VEngineClient;
};

// WrpVEngineClient_L4D2 ////////////////////////////////////////////////////////

class WrpVEngineClient_L4D2
: public WrpVEngineClient
, public WrpVEngineClientDemoInfoEx
{
public:
	WrpVEngineClient_L4D2(SOURCESDK::L4D2::IVEngineClient * iface);

	virtual void GetScreenSize( int& width, int& height );
	virtual void ServerCmd( const char *szCmdString, bool bReliable = true );
	virtual void ClientCmd( const char *szCmdString );
	virtual bool Con_IsVisible( void );
	virtual int	GetLocalPlayer(void);
	virtual void GetViewAngles(SOURCESDK::QAngle& va );
	virtual void SetViewAngles(SOURCESDK::QAngle& va );
	virtual int GetMaxClients( void );
	virtual bool IsInGame( void );
	virtual bool IsConnected( void );
	virtual bool IsDrawingLoadingImage( void );
	virtual void Con_NPrintf( int pos, const char *fmt, ... );
	virtual const char *  GetGameDirectory( void );
	virtual const SOURCESDK::VMatrix& WorldToScreenMatrix();
	virtual const SOURCESDK::VMatrix& WorldToViewMatrix();
	virtual char const *  GetLevelName( void );
	virtual void EngineStats_BeginFrame( void );
	virtual void EngineStats_EndFrame( void );
	virtual bool IsPlayingDemo( void );
	virtual bool IsRecordingDemo( void );
	virtual bool IsPlayingTimeDemo( void );

	virtual int GetDemoRecordingTick( void ); 
	virtual int	GetDemoPlaybackTick( void ); 
	virtual int	GetDemoPlaybackStartTick( void ); 
	virtual float GetDemoPlaybackTimeScale( void ); 
	virtual int GetDemoPlaybackTotalTicks( void ); 

	virtual bool IsPaused( void );
	virtual bool IsTakingScreenshot( void );
	virtual bool IsHLTV( void );
	virtual bool IsLevelMainMenuBackground( void );
	virtual void GetMainMenuBackgroundName( char *dest, int destlen );
	virtual bool IsInEditMode( void );
	virtual unsigned int GetEngineBuildNumber();
	virtual const char * GetProductVersionString();
	virtual bool IsHammerRunning( ) const;
	virtual void ExecuteClientCmd( const char *szCmdString );
	virtual void ClientCmd_Unrestricted( const char *szCmdString );

	virtual WrpVEngineClientDemoInfoEx * GetDemoInfoEx(void);

private:
	SOURCESDK::L4D2::IVEngineClient * m_VEngineClient;
};