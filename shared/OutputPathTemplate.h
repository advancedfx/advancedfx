#pragma once

#include "AfxConsole.h"

#include <string>
#include <map>

namespace advancedfx {

/// Variables that can be used in output path templates, e.g. "{RECORD_PATH}\{TAKE}{TAKE_NUMBER:4}\audio.wav".
/// Numeric variables accept an optional zero-padding width, e.g. {TAKE_NUMBER:4}.
/// Use {{ and }} for literal { and }.
enum OutputPathVariable : unsigned {
	OutputPathVariable_None = 0,
	OutputPathVariable_RecordPath = 1u << 0,  // {RECORD_PATH} - full path of mirv_streams record name.
	OutputPathVariable_Take = 1u << 1,        // {TAKE} - mirv_streams record take.
	OutputPathVariable_TakeNumber = 1u << 2,  // {TAKE_NUMBER} - numeric, default width 4.
	OutputPathVariable_Tick = 1u << 3,        // {TICK} - numeric, game tick at start of recording.
	OutputPathVariable_Time = 1u << 4,        // {TIME} - game client time at start of recording.
	OutputPathVariable_StreamName = 1u << 5,  // {STREAM_NAME}
	OutputPathVariable_StreamPath = 1u << 6,  // {STREAM_PATH} - output folder of the stream.
	OutputPathVariable_SettingName = 1u << 7, // {SETTING_NAME} - name of the child setting (multi settings only).
	OutputPathVariable_SequenceNr = 1u << 8,  // {SEQUENCE_NR} - numeric, default width 5, image number.
	OutputPathVariable_Ext = 1u << 9,         // {EXT} - image file extension (without dot).
	OutputPathVariable_All = ~0u
};

/// Variables that are known at start of recording.
const unsigned OutputPathVariables_Record =
	OutputPathVariable_RecordPath
	| OutputPathVariable_Take
	| OutputPathVariable_TakeNumber
	| OutputPathVariable_Tick
	| OutputPathVariable_Time;

/// Variables that are known at start of recording for outputs of a stream.
const unsigned OutputPathVariables_Stream =
	OutputPathVariables_Record
	| OutputPathVariable_StreamName
	| OutputPathVariable_StreamPath;

class COutputPathValues {
public:
	void SetString(OutputPathVariable variable, const std::wstring & value);
	void SetNumber(OutputPathVariable variable, long long value);

	/// Sets a value that is a template itself and gets expanded in place.
	void SetTemplate(OutputPathVariable variable, const std::wstring & value);

	/// <summary>Expands a template.</summary>
	/// <param name="keepUnset">Variables without value in this mask are kept as they are (the result is a template again then), others are replaced with an empty string.</param>
	/// <param name="outUnset">Optional, receives all variables in the template that had no value.</param>
	std::wstring Expand(const std::wstring & value, unsigned keepUnset, unsigned * outUnset = nullptr) const;

private:
	enum class ValueType {
		String,
		Number,
		Template
	};

	struct Value {
		ValueType Type;
		std::wstring String;
		long long Number;
	};

	std::map<OutputPathVariable, Value> m_Values;

	void Expand(const std::wstring & value, unsigned keepUnset, unsigned expanding, unsigned & outUnset, std::wstring & outResult) const;
};

/// Checks value (UTF-8) for syntax errors, disallowed and missing required variables.
bool OutputPathTemplate_Validate(const char * value, unsigned allowedVariables, unsigned requiredVariables, std::string & outError);

/// Returns the variables used in the template value.
unsigned OutputPathTemplate_GetVariables(const std::wstring & value);

/// Returns the variable names in the mask, e.g. "{RECORD_PATH} {TAKE}".
std::string OutputPathTemplate_GetVariableNames(unsigned variables);

/// <summary>Finds the highest take number already used on disk for a template.</summary>
/// <param name="value">Template where {TAKE_NUMBER} is still unexpanded. Other variables left are treated as wildcards.</param>
/// <returns>Highest take number found or -1 if none.</returns>
long long OutputPathTemplate_FindMaxTakeNumber(const std::wstring & value);

/// An output path template setting that can be edited from the console.
class COutputPathSetting {
public:
	COutputPathSetting(const char * defaultValue, unsigned allowedVariables, unsigned requiredVariables = OutputPathVariable_None);

	const std::string & Get() const {
		return m_Value;
	}

	bool GetWide(std::wstring & outValue) const;

	/// Validates and sets the value, prints a warning and returns false if invalid.
	bool Set(const char * value);

	/// ArgV(0) is the command prefix, optional ArgV(1) the new value or "default".
	void Console(ICommandArgs * args, const char * description);

private:
	std::string m_DefaultValue;
	std::string m_Value;
	unsigned m_AllowedVariables;
	unsigned m_RequiredVariables;
};

} // namespace advancedfx
