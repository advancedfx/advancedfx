#include "stdafx.h"

// Description:
// Wrapper(s) for Source's IVEngineClient Interface

#include "WrpVEngineClient.h"

#include <stdarg.h>

// WrpVEngineClient_012 ///////////////////////////////////////////////////////////

WrpVEngineClient_012::WrpVEngineClient_012(SOURCESDK::IVEngineClient_012 * iface) {
	m_VEngineClient_012 = iface;
}

void WrpVEngineClient_012::GetScreenSize( int& width, int& height ) {
	m_VEngineClient_012->GetScreenSize(width, height);
}

void WrpVEngineClient_012::ServerCmd( const char *szCmdString, bool bReliable) {
	m_VEngineClient_012->ServerCmd(szCmdString, bReliable);
}

void WrpVEngineClient_012::ClientCmd( const char *szCmdString ) {
	m_VEngineClient_012->ClientCmd(szCmdString);
}

bool WrpVEngineClient_012::Con_IsVisible( void ) {
	return m_VEngineClient_012->Con_IsVisible();
}

int	WrpVEngineClient_012::GetLocalPlayer(void) {
	return m_VEngineClient_012->GetLocalPlayer();
}

void WrpVEngineClient_012::GetViewAngles(SOURCESDK::QAngle& va ) {
	m_VEngineClient_012->GetViewAngles(va);
}

void WrpVEngineClient_012::SetViewAngles(SOURCESDK::QAngle& va ) {
	m_VEngineClient_012->SetViewAngles(va);
}

int WrpVEngineClient_012::GetMaxClients( void ) {
	return m_VEngineClient_012->GetMaxClients();
}

bool WrpVEngineClient_012::IsInGame( void ) {
	return m_VEngineClient_012->IsInGame();
}

bool WrpVEngineClient_012::IsConnected( void ) {
	return m_VEngineClient_012->IsConnected();
}

bool WrpVEngineClient_012::IsDrawingLoadingImage( void ) {
	return m_VEngineClient_012->IsDrawingLoadingImage();
}

void WrpVEngineClient_012::Con_NPrintf( int pos, const char *fmt, ... ) {
	va_list argptr;
	va_start(argptr, fmt);
	m_VEngineClient_012->Con_NPrintf(pos, fmt, argptr);
}

const char *  WrpVEngineClient_012::GetGameDirectory( void ) {
	return m_VEngineClient_012->GetGameDirectory();
}

const SOURCESDK::VMatrix& WrpVEngineClient_012::WorldToScreenMatrix()
{
	return m_VEngineClient_012->WorldToScreenMatrix();
}

const SOURCESDK::VMatrix& WrpVEngineClient_012::WorldToViewMatrix()
{
	return m_VEngineClient_012->WorldToViewMatrix();
}

char const *  WrpVEngineClient_012::GetLevelName( void ) {
	return m_VEngineClient_012->GetLevelName();
}

void WrpVEngineClient_012::EngineStats_BeginFrame( void ) {
	return m_VEngineClient_012->EngineStats_BeginFrame();
}

void WrpVEngineClient_012::EngineStats_EndFrame( void ) {
	return m_VEngineClient_012->EngineStats_EndFrame();
}

bool WrpVEngineClient_012::IsPlayingDemo( void ) {
	return m_VEngineClient_012->IsPlayingDemo();
}

bool WrpVEngineClient_012::IsRecordingDemo( void ) {
	return m_VEngineClient_012->IsRecordingDemo();
}

bool WrpVEngineClient_012::IsPlayingTimeDemo( void ) {
	return m_VEngineClient_012->IsPlayingTimeDemo();
}

bool WrpVEngineClient_012::IsPaused( void ) {
	return m_VEngineClient_012->IsPaused();
}

bool WrpVEngineClient_012::IsTakingScreenshot( void ) {
	return m_VEngineClient_012->IsTakingScreenshot();
}
bool WrpVEngineClient_012::IsHLTV( void ) {
	return m_VEngineClient_012->IsHLTV();
}

bool WrpVEngineClient_012::IsLevelMainMenuBackground( void ) {
	return m_VEngineClient_012->IsLevelMainMenuBackground();
}

void WrpVEngineClient_012::GetMainMenuBackgroundName( char *dest, int destlen ) {
	return m_VEngineClient_012->GetMainMenuBackgroundName(dest, destlen);
}

bool WrpVEngineClient_012::IsInEditMode( void ) {
	return m_VEngineClient_012->IsInEditMode();
}

unsigned int WrpVEngineClient_012::GetEngineBuildNumber() {
	return m_VEngineClient_012->GetEngineBuildNumber();
}

const char * WrpVEngineClient_012::GetProductVersionString() {
	return m_VEngineClient_012->GetProductVersionString();
}

bool WrpVEngineClient_012::IsHammerRunning( ) const {
	return m_VEngineClient_012->IsHammerRunning();
}

void WrpVEngineClient_012::ExecuteClientCmd( const char *szCmdString ) {
	m_VEngineClient_012->ExecuteClientCmd(szCmdString);
}

void WrpVEngineClient_012::ClientCmd_Unrestricted( const char *szCmdString )
{
	// In 012 this is unrestricted:
	ClientCmd(szCmdString);
}

WrpVEngineClientDemoInfoEx * WrpVEngineClient_012::GetDemoInfoEx(void)
{
	return 0;
}

// WrpVEngineClient_013 ///////////////////////////////////////////////////////////

WrpVEngineClient_013::WrpVEngineClient_013(SOURCESDK::IVEngineClient_013 * iface) {
	m_VEngineClient_013 = iface;
}

void WrpVEngineClient_013::GetScreenSize( int& width, int& height ) {
	m_VEngineClient_013->GetScreenSize(width, height);
}

void WrpVEngineClient_013::ServerCmd( const char *szCmdString, bool bReliable) {
	m_VEngineClient_013->ServerCmd(szCmdString, bReliable);
}

void WrpVEngineClient_013::ClientCmd( const char *szCmdString ) {
	m_VEngineClient_013->ClientCmd(szCmdString);
}

bool WrpVEngineClient_013::Con_IsVisible( void ) {
	return m_VEngineClient_013->Con_IsVisible();
}

int	WrpVEngineClient_013::GetLocalPlayer(void) {
	return m_VEngineClient_013->GetLocalPlayer();
}

void WrpVEngineClient_013::GetViewAngles(SOURCESDK::QAngle& va ) {
	m_VEngineClient_013->GetViewAngles(va);
}

void WrpVEngineClient_013::SetViewAngles(SOURCESDK::QAngle& va ) {
	m_VEngineClient_013->SetViewAngles(va);
}

int WrpVEngineClient_013::GetMaxClients( void ) {
	return m_VEngineClient_013->GetMaxClients();
}

bool WrpVEngineClient_013::IsInGame( void ) {
	return m_VEngineClient_013->IsInGame();
}

bool WrpVEngineClient_013::IsConnected( void ) {
	return m_VEngineClient_013->IsConnected();
}

bool WrpVEngineClient_013::IsDrawingLoadingImage( void ) {
	return m_VEngineClient_013->IsDrawingLoadingImage();
}

void WrpVEngineClient_013::Con_NPrintf( int pos, const char *fmt, ... ) {
	va_list argptr;
	va_start(argptr, fmt);
	m_VEngineClient_013->Con_NPrintf(pos, fmt, argptr);
}

const char *  WrpVEngineClient_013::GetGameDirectory( void ) {
	return m_VEngineClient_013->GetGameDirectory();
}

const SOURCESDK::VMatrix& WrpVEngineClient_013::WorldToScreenMatrix()
{
	return m_VEngineClient_013->WorldToScreenMatrix();
}

const SOURCESDK::VMatrix& WrpVEngineClient_013::WorldToViewMatrix()
{
	return m_VEngineClient_013->WorldToViewMatrix();
}


char const *  WrpVEngineClient_013::GetLevelName( void ) {
	return m_VEngineClient_013->GetLevelName();
}

void WrpVEngineClient_013::EngineStats_BeginFrame( void ) {
	return m_VEngineClient_013->EngineStats_BeginFrame();
}

void WrpVEngineClient_013::EngineStats_EndFrame( void ) {
	return m_VEngineClient_013->EngineStats_EndFrame();
}

bool WrpVEngineClient_013::IsPlayingDemo( void ) {
	return m_VEngineClient_013->IsPlayingDemo();
}

bool WrpVEngineClient_013::IsRecordingDemo( void ) {
	return m_VEngineClient_013->IsRecordingDemo();
}

bool WrpVEngineClient_013::IsPlayingTimeDemo( void ) {
	return m_VEngineClient_013->IsPlayingTimeDemo();
}

int WrpVEngineClient_013::GetDemoRecordingTick( void )
{
	return m_VEngineClient_013->GetDemoRecordingTick();
}

int	WrpVEngineClient_013::GetDemoPlaybackTick( void )
{
	return m_VEngineClient_013->GetDemoPlaybackTick();
}

int	WrpVEngineClient_013::GetDemoPlaybackStartTick( void )
{
	return m_VEngineClient_013->GetDemoPlaybackStartTick();
}

float WrpVEngineClient_013::GetDemoPlaybackTimeScale( void )
{
	return m_VEngineClient_013->GetDemoPlaybackTimeScale();
}

int WrpVEngineClient_013::GetDemoPlaybackTotalTicks( void )
{
	return m_VEngineClient_013->GetDemoPlaybackTotalTicks();
}

bool WrpVEngineClient_013::IsPaused( void ) {
	return m_VEngineClient_013->IsPaused();
}

bool WrpVEngineClient_013::IsTakingScreenshot( void ) {
	return m_VEngineClient_013->IsTakingScreenshot();
}
bool WrpVEngineClient_013::IsHLTV( void ) {
	return m_VEngineClient_013->IsHLTV();
}

bool WrpVEngineClient_013::IsLevelMainMenuBackground( void ) {
	return m_VEngineClient_013->IsLevelMainMenuBackground();
}

void WrpVEngineClient_013::GetMainMenuBackgroundName( char *dest, int destlen ) {
	return m_VEngineClient_013->GetMainMenuBackgroundName(dest, destlen);
}

bool WrpVEngineClient_013::IsInEditMode( void ) {
	return m_VEngineClient_013->IsInEditMode();
}

unsigned int WrpVEngineClient_013::GetEngineBuildNumber() {
	return m_VEngineClient_013->GetEngineBuildNumber();
}

const char * WrpVEngineClient_013::GetProductVersionString() {
	return m_VEngineClient_013->GetProductVersionString();
}

bool WrpVEngineClient_013::IsHammerRunning( ) const {
	return m_VEngineClient_013->IsHammerRunning();
}

void WrpVEngineClient_013::ExecuteClientCmd( const char *szCmdString ) {
	m_VEngineClient_013->ExecuteClientCmd(szCmdString);
}

void WrpVEngineClient_013::ClientCmd_Unrestricted( const char *szCmdString ) {
	m_VEngineClient_013->ClientCmd_Unrestricted(szCmdString);
}

WrpVEngineClientDemoInfoEx * WrpVEngineClient_013::GetDemoInfoEx(void)
{
	return this;
}

// WrpVEngineClient_014_csgo ///////////////////////////////////////////////////////////

WrpVEngineClient_014_csgo::WrpVEngineClient_014_csgo(SOURCESDK::IVEngineClient_014_csgo * iface) {
	m_VEngineClient = iface;
}

void WrpVEngineClient_014_csgo::GetScreenSize( int& width, int& height ) {
	m_VEngineClient->GetScreenSize(width, height);
}

void WrpVEngineClient_014_csgo::ServerCmd( const char *szCmdString, bool bReliable) {
	m_VEngineClient->ServerCmd(szCmdString, bReliable);
}

void WrpVEngineClient_014_csgo::ClientCmd( const char *szCmdString ) {
	m_VEngineClient->ClientCmd(szCmdString);
}

bool WrpVEngineClient_014_csgo::Con_IsVisible( void ) {
	return m_VEngineClient->Con_IsVisible();
}

int	WrpVEngineClient_014_csgo::GetLocalPlayer(void) {
	return m_VEngineClient->GetLocalPlayer();
}

void WrpVEngineClient_014_csgo::GetViewAngles(SOURCESDK::QAngle& va ) {
	m_VEngineClient->GetViewAngles(va);
}

void WrpVEngineClient_014_csgo::SetViewAngles(SOURCESDK::QAngle& va ) {
	m_VEngineClient->SetViewAngles(va);
}

int WrpVEngineClient_014_csgo::GetMaxClients( void ) {
	return m_VEngineClient->GetMaxClients();
}

bool WrpVEngineClient_014_csgo::IsInGame( void ) {
	return m_VEngineClient->IsInGame();
}

bool WrpVEngineClient_014_csgo::IsConnected( void ) {
	return m_VEngineClient->IsConnected();
}

bool WrpVEngineClient_014_csgo::IsDrawingLoadingImage( void ) {
	return m_VEngineClient->IsDrawingLoadingImage();
}

void WrpVEngineClient_014_csgo::Con_NPrintf( int pos, const char *fmt, ... ) {
	va_list argptr;
	va_start(argptr, fmt);
	m_VEngineClient->Con_NPrintf(pos, fmt, argptr);
}

const char *  WrpVEngineClient_014_csgo::GetGameDirectory( void ) {
	return m_VEngineClient->GetGameDirectory();
}

const SOURCESDK::VMatrix& WrpVEngineClient_014_csgo::WorldToScreenMatrix()
{
	return m_VEngineClient->WorldToScreenMatrix();
}

const SOURCESDK::VMatrix& WrpVEngineClient_014_csgo::WorldToViewMatrix()
{
	return m_VEngineClient->WorldToViewMatrix();
}


char const *  WrpVEngineClient_014_csgo::GetLevelName( void ) {
	return m_VEngineClient->GetLevelName();
}

void WrpVEngineClient_014_csgo::EngineStats_BeginFrame( void ) {
	return m_VEngineClient->EngineStats_BeginFrame();
}

void WrpVEngineClient_014_csgo::EngineStats_EndFrame( void ) {
	return m_VEngineClient->EngineStats_EndFrame();
}

bool WrpVEngineClient_014_csgo::IsPlayingDemo( void ) {
	return m_VEngineClient->IsPlayingDemo();
}

bool WrpVEngineClient_014_csgo::IsRecordingDemo( void ) {
	return m_VEngineClient->IsRecordingDemo();
}

bool WrpVEngineClient_014_csgo::IsPlayingTimeDemo( void ) {
	return m_VEngineClient->IsPlayingTimeDemo();
}

int WrpVEngineClient_014_csgo::GetDemoRecordingTick( void )
{
	return m_VEngineClient->GetDemoRecordingTick();
}

int	WrpVEngineClient_014_csgo::GetDemoPlaybackTick( void )
{
	return m_VEngineClient->GetDemoPlaybackTick();
}

int	WrpVEngineClient_014_csgo::GetDemoPlaybackStartTick( void )
{
	return m_VEngineClient->GetDemoPlaybackStartTick();
}

float WrpVEngineClient_014_csgo::GetDemoPlaybackTimeScale( void )
{
	return m_VEngineClient->GetDemoPlaybackTimeScale();
}

int WrpVEngineClient_014_csgo::GetDemoPlaybackTotalTicks( void )
{
	return m_VEngineClient->GetDemoPlaybackTotalTicks();
}

bool WrpVEngineClient_014_csgo::IsPaused( void ) {
	return m_VEngineClient->IsPaused();
}

bool WrpVEngineClient_014_csgo::IsTakingScreenshot( void ) {
	return m_VEngineClient->IsTakingScreenshot();
}
bool WrpVEngineClient_014_csgo::IsHLTV( void ) {
	return m_VEngineClient->IsHLTV();
}

bool WrpVEngineClient_014_csgo::IsLevelMainMenuBackground( void ) {
	return m_VEngineClient->IsLevelMainMenuBackground();
}

void WrpVEngineClient_014_csgo::GetMainMenuBackgroundName( char *dest, int destlen ) {
	return m_VEngineClient->GetMainMenuBackgroundName(dest, destlen);
}

bool WrpVEngineClient_014_csgo::IsInEditMode( void ) {
	return m_VEngineClient->IsInEditMode();
}

unsigned int WrpVEngineClient_014_csgo::GetEngineBuildNumber() {
	return m_VEngineClient->GetEngineBuildNumber();
}

const char * WrpVEngineClient_014_csgo::GetProductVersionString() {
	return m_VEngineClient->GetProductVersionString();
}

bool WrpVEngineClient_014_csgo::IsHammerRunning( ) const {
	return m_VEngineClient->IsHammerRunning();
}

void WrpVEngineClient_014_csgo::ExecuteClientCmd( const char *szCmdString ) {
	m_VEngineClient->ExecuteClientCmd(szCmdString);
}

void WrpVEngineClient_014_csgo::ClientCmd_Unrestricted( const char *szCmdString ) {
	m_VEngineClient->ClientCmd_Unrestricted(szCmdString);
}

WrpVEngineClientDemoInfoEx * WrpVEngineClient_014_csgo::GetDemoInfoEx(void)
{
	return this;
}


// WrpVEngineClient_Insurgency2 ///////////////////////////////////////////////////////////

WrpVEngineClient_Insurgency2::WrpVEngineClient_Insurgency2(SOURCESDK::INSURGENCY2::IVEngineClient * iface) {
	m_VEngineClient = iface;
}

void WrpVEngineClient_Insurgency2::GetScreenSize(int& width, int& height) {
	m_VEngineClient->GetScreenSize(width, height);
}

void WrpVEngineClient_Insurgency2::ServerCmd(const char *szCmdString, bool bReliable) {
	m_VEngineClient->ServerCmd(szCmdString, bReliable);
}

void WrpVEngineClient_Insurgency2::ClientCmd(const char *szCmdString) {
	m_VEngineClient->ClientCmd(szCmdString);
}

bool WrpVEngineClient_Insurgency2::Con_IsVisible(void) {
	return m_VEngineClient->Con_IsVisible();
}

int	WrpVEngineClient_Insurgency2::GetLocalPlayer(void) {
	return m_VEngineClient->GetLocalPlayer();
}

void WrpVEngineClient_Insurgency2::GetViewAngles(SOURCESDK::QAngle& va) {
	m_VEngineClient->GetViewAngles(va);
}

void WrpVEngineClient_Insurgency2::SetViewAngles(SOURCESDK::QAngle& va) {
	m_VEngineClient->SetViewAngles(va);
}

int WrpVEngineClient_Insurgency2::GetMaxClients(void) {
	return m_VEngineClient->GetMaxClients();
}

bool WrpVEngineClient_Insurgency2::IsInGame(void) {
	return m_VEngineClient->IsInGame();
}

bool WrpVEngineClient_Insurgency2::IsConnected(void) {
	return m_VEngineClient->IsConnected();
}

bool WrpVEngineClient_Insurgency2::IsDrawingLoadingImage(void) {
	return m_VEngineClient->IsDrawingLoadingImage();
}

void WrpVEngineClient_Insurgency2::Con_NPrintf(int pos, const char *fmt, ...) {
	va_list argptr;
	va_start(argptr, fmt);
	m_VEngineClient->Con_NPrintf(pos, fmt, argptr);
}

const char *  WrpVEngineClient_Insurgency2::GetGameDirectory(void) {
	return m_VEngineClient->GetGameDirectory();
}

const SOURCESDK::VMatrix& WrpVEngineClient_Insurgency2::WorldToScreenMatrix()
{
	return m_VEngineClient->WorldToScreenMatrix();
}

const SOURCESDK::VMatrix& WrpVEngineClient_Insurgency2::WorldToViewMatrix()
{
	return m_VEngineClient->WorldToViewMatrix();
}


char const *  WrpVEngineClient_Insurgency2::GetLevelName(void) {
	return m_VEngineClient->GetLevelName();
}

void WrpVEngineClient_Insurgency2::EngineStats_BeginFrame(void) {
	return m_VEngineClient->EngineStats_BeginFrame();
}

void WrpVEngineClient_Insurgency2::EngineStats_EndFrame(void) {
	return m_VEngineClient->EngineStats_EndFrame();
}

bool WrpVEngineClient_Insurgency2::IsPlayingDemo(void) {
	return m_VEngineClient->IsPlayingDemo();
}

bool WrpVEngineClient_Insurgency2::IsRecordingDemo(void) {
	return m_VEngineClient->IsRecordingDemo();
}

bool WrpVEngineClient_Insurgency2::IsPlayingTimeDemo(void) {
	return m_VEngineClient->IsPlayingTimeDemo();
}

int WrpVEngineClient_Insurgency2::GetDemoRecordingTick(void)
{
	return m_VEngineClient->GetDemoRecordingTick();
}

int	WrpVEngineClient_Insurgency2::GetDemoPlaybackTick(void)
{
	return m_VEngineClient->GetDemoPlaybackTick();
}

int	WrpVEngineClient_Insurgency2::GetDemoPlaybackStartTick(void)
{
	return m_VEngineClient->GetDemoPlaybackStartTick();
}

float WrpVEngineClient_Insurgency2::GetDemoPlaybackTimeScale(void)
{
	return m_VEngineClient->GetDemoPlaybackTimeScale();
}

int WrpVEngineClient_Insurgency2::GetDemoPlaybackTotalTicks(void)
{
	return m_VEngineClient->GetDemoPlaybackTotalTicks();
}

bool WrpVEngineClient_Insurgency2::IsPaused(void) {
	return m_VEngineClient->IsPaused();
}

bool WrpVEngineClient_Insurgency2::IsTakingScreenshot(void) {
	return m_VEngineClient->IsTakingScreenshot();
}
bool WrpVEngineClient_Insurgency2::IsHLTV(void) {
	return m_VEngineClient->IsHLTV();
}

bool WrpVEngineClient_Insurgency2::IsLevelMainMenuBackground(void) {
	return m_VEngineClient->IsLevelMainMenuBackground();
}

void WrpVEngineClient_Insurgency2::GetMainMenuBackgroundName(char *dest, int destlen) {
	return m_VEngineClient->GetMainMenuBackgroundName(dest, destlen);
}

bool WrpVEngineClient_Insurgency2::IsInEditMode(void) {
	return m_VEngineClient->IsInEditMode();
}

unsigned int WrpVEngineClient_Insurgency2::GetEngineBuildNumber() {
	return m_VEngineClient->GetEngineBuildNumber();
}

const char * WrpVEngineClient_Insurgency2::GetProductVersionString() {
	return m_VEngineClient->GetProductVersionString();
}

bool WrpVEngineClient_Insurgency2::IsHammerRunning() const {
	return m_VEngineClient->IsHammerRunning();
}

void WrpVEngineClient_Insurgency2::ExecuteClientCmd(const char *szCmdString) {
	m_VEngineClient->ExecuteClientCmd(szCmdString);
}

void WrpVEngineClient_Insurgency2::ClientCmd_Unrestricted(const char *szCmdString) {
	m_VEngineClient->ClientCmd_Unrestricted(szCmdString);
}

WrpVEngineClientDemoInfoEx * WrpVEngineClient_Insurgency2::GetDemoInfoEx(void)
{
	return this;
}


// WrpVEngineClient_bm ///////////////////////////////////////////////////////////

WrpVEngineClient_bm::WrpVEngineClient_bm(SOURCESDK::BM::IVEngineClient * iface)
{
	m_VEngineClient = iface;
}

void WrpVEngineClient_bm::GetScreenSize(int& width, int& height)
{
	m_VEngineClient->GetScreenSize(width, height);
}

void WrpVEngineClient_bm::ServerCmd(const char *szCmdString, bool bReliable)
{
	m_VEngineClient->ServerCmd(szCmdString, bReliable);
}

void WrpVEngineClient_bm::ClientCmd(const char *szCmdString)
{
	m_VEngineClient->ClientCmd(szCmdString);
}

bool WrpVEngineClient_bm::Con_IsVisible(void)
{
	return m_VEngineClient->Con_IsVisible();
}

int	WrpVEngineClient_bm::GetLocalPlayer(void)
{
	return m_VEngineClient->GetLocalPlayer();
}

void WrpVEngineClient_bm::GetViewAngles(SOURCESDK::QAngle& va)
{
	m_VEngineClient->GetViewAngles(va);
}

void WrpVEngineClient_bm::SetViewAngles(SOURCESDK::QAngle& va)
{
	m_VEngineClient->SetViewAngles(va);
}

int WrpVEngineClient_bm::GetMaxClients(void)
{
	return m_VEngineClient->GetMaxClients();
}

bool WrpVEngineClient_bm::IsInGame(void)
{
	return m_VEngineClient->IsInGame();
}

bool WrpVEngineClient_bm::IsConnected(void)
{
	return m_VEngineClient->IsConnected();
}

bool WrpVEngineClient_bm::IsDrawingLoadingImage(void)
{
	return m_VEngineClient->IsDrawingLoadingImage();
}

void WrpVEngineClient_bm::Con_NPrintf(int pos, const char *fmt, ...)
{
	va_list argptr;
	va_start(argptr, fmt);
	m_VEngineClient->Con_NPrintf(pos, fmt, argptr);
}

const char *  WrpVEngineClient_bm::GetGameDirectory(void)
{
	return m_VEngineClient->GetGameDirectory();
}

const SOURCESDK::VMatrix& WrpVEngineClient_bm::WorldToScreenMatrix()
{
	return m_VEngineClient->WorldToScreenMatrix();
}

const SOURCESDK::VMatrix& WrpVEngineClient_bm::WorldToViewMatrix()
{
	return m_VEngineClient->WorldToViewMatrix();
}


char const *  WrpVEngineClient_bm::GetLevelName(void)
{
	return m_VEngineClient->GetLevelName();
}

void WrpVEngineClient_bm::EngineStats_BeginFrame(void)
{
//	return m_VEngineClient->EngineStats_BeginFrame();
}

void WrpVEngineClient_bm::EngineStats_EndFrame(void)
{
//	return m_VEngineClient->EngineStats_EndFrame();
}

bool WrpVEngineClient_bm::IsPlayingDemo(void)
{
	return m_VEngineClient->IsPlayingDemo();
}

bool WrpVEngineClient_bm::IsRecordingDemo(void)
{
	return m_VEngineClient->IsRecordingDemo();
}

bool WrpVEngineClient_bm::IsPlayingTimeDemo(void)
{
	return m_VEngineClient->IsPlayingTimeDemo();
}

int WrpVEngineClient_bm::GetDemoRecordingTick(void)
{
	return m_VEngineClient->GetDemoRecordingTick();
}

int	WrpVEngineClient_bm::GetDemoPlaybackTick(void)
{
	return m_VEngineClient->GetDemoPlaybackTick();
}

int	WrpVEngineClient_bm::GetDemoPlaybackStartTick(void)
{
	return m_VEngineClient->GetDemoPlaybackStartTick();
}

float WrpVEngineClient_bm::GetDemoPlaybackTimeScale(void)
{
	return m_VEngineClient->GetDemoPlaybackTimeScale();
}

int WrpVEngineClient_bm::GetDemoPlaybackTotalTicks(void)
{
	return m_VEngineClient->GetDemoPlaybackTotalTicks();
}

bool WrpVEngineClient_bm::IsPaused(void)
{
	return m_VEngineClient->IsPaused();
}

bool WrpVEngineClient_bm::IsTakingScreenshot(void)
{
	return m_VEngineClient->IsTakingScreenshot();
}
bool WrpVEngineClient_bm::IsHLTV(void)
{
	return m_VEngineClient->IsHLTV();
}

bool WrpVEngineClient_bm::IsLevelMainMenuBackground(void)
{
	return m_VEngineClient->IsLevelMainMenuBackground();
}

void WrpVEngineClient_bm::GetMainMenuBackgroundName(char *dest, int destlen)
{
	return m_VEngineClient->GetMainMenuBackgroundName(dest, destlen);
}

bool WrpVEngineClient_bm::IsInEditMode(void)
{
	return m_VEngineClient->IsInEditMode();
}

unsigned int WrpVEngineClient_bm::GetEngineBuildNumber()
{
	return m_VEngineClient->GetEngineBuildNumber();
}

const char * WrpVEngineClient_bm::GetProductVersionString()
{
	return m_VEngineClient->GetProductVersionString();
}

bool WrpVEngineClient_bm::IsHammerRunning() const
{
	return m_VEngineClient->IsHammerRunning();
}

void WrpVEngineClient_bm::ExecuteClientCmd(const char *szCmdString)
{
	m_VEngineClient->ExecuteClientCmd(szCmdString);
}

void WrpVEngineClient_bm::ClientCmd_Unrestricted(const char *szCmdString)
{
	m_VEngineClient->ClientCmd_Unrestricted(szCmdString);
}

WrpVEngineClientDemoInfoEx * WrpVEngineClient_bm::GetDemoInfoEx(void)
{
	return this;
}
