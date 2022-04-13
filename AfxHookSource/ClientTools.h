#pragma once

#include "SourceInterfaces.h"
#include "WrpConsole.h"

#include <shared/AfxGameRecord.h>

#include <string>
#include <set>
#include <map>

class CClientTools abstract
{
public:
	static inline CClientTools * Instance(void)
	{
		return m_Instance;
	}

	CClientTools();
	virtual ~CClientTools();

	virtual void OnPostToolMessage(void * hEntity, void * msg);

	virtual void OnBeforeFrameRenderStart(void);

	virtual void OnAfterSetupEngineView(void) {

	}

	virtual void OnAfterFrameRenderEnd(void);

	virtual bool SuppotsAutoEnableRecordingMode(void) {
		return false;
	}

	virtual void EnableRecordingMode_set(bool value) {

	}

	virtual bool EnableRecordingMode_get() {
		return false;
	}

	bool GetRecording(void);

	virtual int GetAgrVersion() {
		return 5;
	}

	virtual void StartRecording(wchar_t const * fileName);

	virtual void EndRecording();

	int Debug_get(void)
	{
		return m_Debug;
	}

	void Debug_set(int value)
	{
		m_Debug = value;
	}

	bool RecordWeapons_get(void)
	{
		return m_RecordWeapons;
	}

	void RecordWeapons_set(bool value)
	{
		m_RecordWeapons = value;
	}

	bool RecordProjectiles_get(void)
	{
		return m_RecordProjectiles;
	}

	void RecordProjectiles_set(bool value)
	{
		m_RecordProjectiles = value;
	}

	bool RecordPlayers_get(void)
	{
		return m_RecordPlayers;
	}

	void RecordPlayers_set(bool value)
	{
		m_RecordPlayers = value;
	}

	bool RecordCamera_get(void)
	{
		return m_RecordCamera;
	}

	void RecordCamera_set(bool value)
	{
		m_RecordCamera = value;
	}

	int RecordViewModels_get(void)
	{
		return m_RecordViewModels;
	}

	void RecordViewModels_set(int value)
	{
		m_RecordViewModels = value;
	}

	bool RecordInvisible_get(void)
	{
		return m_RecordInvisible;
	}

	void RecordInvisible_set(bool value)
	{
		m_RecordInvisible = value;
	}

	int RecordPlayerCameras_get(void)
	{
		return m_RecordPlayerCameras;
	}

	void RecordPlayerCameras_set(int value)
	{
		m_RecordPlayerCameras = value;
	}

	virtual bool SupportsRecordPlayerCameras() {
		return false;
	}

	virtual bool SupportsRecordViewModelMultiple() {
		return false;
	}

protected:
	virtual float ScaleFov(int width, int height, float fov) { return fov; }

	void WriteDictionary(char const * value) {
		m_AfxGameRecord.WriteDictionary(value);
	}

	void Write(bool value) { m_AfxGameRecord.Write(value); }
	void Write(int value) { m_AfxGameRecord.Write(value); }
	void Write(float value) { m_AfxGameRecord.Write(value); }
	void Write(double value) { m_AfxGameRecord.Write(value); }

	/// Consider using WriteDictionary instead (if string is long enough and likely to repeat often).
	void Write(char const * value) { m_AfxGameRecord.Write(value); } 

	void Write(SOURCESDK::Vector const & value);
	void Write(SOURCESDK::QAngle const & value);
	void Write(SOURCESDK::Quaternion const & value);
	void WriteMatrix3x4(SOURCESDK::matrix3x4_t const &value);
	void WriteBones(SOURCESDK::CStudioHdr * hdr, SOURCESDK::matrix3x4_t const *boneState, const SOURCESDK::matrix3x4_t & parentTransform);

	void MarkHidden(int value) {
		m_AfxGameRecord.MarkHidden(value);
	}

	static bool InvertMatrix(const SOURCESDK::matrix3x4_t &matrix, SOURCESDK::matrix3x4_t &out_matrix);

private:
	static CClientTools * m_Instance;

	advancedfx::CAfxGameRecord m_AfxGameRecord;

	bool m_EnableRecording = false;

	int m_Debug = 0;
	bool m_RecordCamera = true;
	bool m_RecordPlayers = true;
	int m_RecordPlayerCameras = -1;
	bool m_RecordWeapons = true;
	bool m_RecordProjectiles = true;
	int m_RecordViewModels = 0;
	bool m_RecordInvisible = false;

};

bool ClientTools_Console_Cfg(IWrpCommandArgs * args);
