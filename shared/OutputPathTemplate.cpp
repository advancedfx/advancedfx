#include "stdafx.h"

#include "OutputPathTemplate.h"

#include "StringTools.h"

#include <Windows.h>

#include <algorithm>
#include <regex>
#include <vector>

namespace advancedfx {

namespace {

struct VariableInfo {
	OutputPathVariable Variable;
	const wchar_t * Name;
	int DefaultWidth; // -1 if not numeric.
};

const VariableInfo g_Variables[] = {
	{OutputPathVariable_RecordPath, L"RECORD_PATH", -1},
	{OutputPathVariable_Take, L"TAKE", -1},
	{OutputPathVariable_TakeNumber, L"TAKE_NUMBER", 4},
	{OutputPathVariable_Tick, L"TICK", 0},
	{OutputPathVariable_Time, L"TIME", -1},
	{OutputPathVariable_StreamName, L"STREAM_NAME", -1},
	{OutputPathVariable_StreamPath, L"STREAM_PATH", -1},
	{OutputPathVariable_SettingName, L"SETTING_NAME", -1},
	{OutputPathVariable_SequenceNr, L"SEQUENCE_NR", 5},
	{OutputPathVariable_Ext, L"EXT", -1},
};

const size_t MaxWidthDigits = 2;
const int MaxWidth = 20;

const VariableInfo * FindVariable(const std::wstring & name) {
	for (auto & info : g_Variables) {
		if (0 == _wcsicmp(info.Name, name.c_str())) return &info;
	}
	return nullptr;
}

struct Token {
	const VariableInfo * Variable; // nullptr for literal text.
	std::wstring Text; // Unescaped literal text.
	int Width; // -1 if not given.
};

bool Parse(const std::wstring & value, std::vector<Token> & outTokens, std::wstring & outError) {
	std::wstring literal;
	auto flush = [&]() {
		if (!literal.empty()) {
			outTokens.push_back(Token{nullptr, literal, -1});
			literal.clear();
		}
	};

	size_t i = 0;
	const size_t n = value.size();
	while (i < n) {
		wchar_t c = value[i];
		if (L'{' == c) {
			if (i + 1 < n && L'{' == value[i + 1]) {
				literal += L'{';
				i += 2;
				continue;
			}
			size_t end = value.find(L'}', i + 1);
			if (std::wstring::npos == end) {
				outError = L"Missing } for { at position " + std::to_wstring(i) + L", use {{ for a literal {.";
				return false;
			}
			std::wstring content = value.substr(i + 1, end - i - 1);
			std::wstring name = content;
			int width = -1;
			size_t colon = content.find(L':');
			if (std::wstring::npos != colon) {
				name = content.substr(0, colon);
				std::wstring widthText = content.substr(colon + 1);
				bool widthOk = !widthText.empty() && widthText.size() <= MaxWidthDigits;
				for (wchar_t d : widthText) if (d < L'0' || L'9' < d) widthOk = false;
				if (widthOk) {
					width = _wtoi(widthText.c_str());
					widthOk = width <= MaxWidth;
				}
				if (!widthOk) {
					outError = L"Invalid width in {" + content + L"}, must be 0 to " + std::to_wstring(MaxWidth) + L".";
					return false;
				}
			}
			const VariableInfo * info = FindVariable(name);
			if (nullptr == info) {
				outError = L"Unknown variable {" + content + L"}.";
				return false;
			}
			if (-1 != width && info->DefaultWidth < 0) {
				outError = std::wstring(L"Variable {") + info->Name + L"} does not support a width.";
				return false;
			}
			flush();
			outTokens.push_back(Token{info, std::wstring(), width});
			i = end + 1;
		}
		else if (L'}' == c) {
			if (i + 1 < n && L'}' == value[i + 1]) {
				literal += L'}';
				i += 2;
				continue;
			}
			outError = L"Unexpected } at position " + std::to_wstring(i) + L", use }} for a literal }.";
			return false;
		}
		else {
			literal += c;
			++i;
		}
	}
	flush();

	return true;
}

void AppendEscaped(std::wstring & outResult, const std::wstring & value) {
	for (wchar_t c : value) {
		if (L'{' == c || L'}' == c) outResult += c;
		outResult += c;
	}
}

void AppendVariable(std::wstring & outResult, const Token & token) {
	outResult += L'{';
	outResult += token.Variable->Name;
	if (0 <= token.Width) {
		outResult += L':';
		outResult += std::to_wstring(token.Width);
	}
	outResult += L'}';
}

std::wstring FormatNumber(long long value, int width) {
	bool negative = value < 0;
	std::wstring digits = std::to_wstring(negative ? 0ull - (unsigned long long)value : (unsigned long long)value);
	if (digits.size() < (size_t)width) digits.insert(0, width - digits.size(), L'0');
	if (negative) digits.insert(0, 1, L'-');
	return digits;
}

bool IsPathSeparator(wchar_t c) {
	return L'\\' == c || L'/' == c;
}

} // namespace {

// COutputPathValues ///////////////////////////////////////////////////////////

void COutputPathValues::SetString(OutputPathVariable variable, const std::wstring & value) {
	m_Values[variable] = Value{ValueType::String, value, 0};
}

void COutputPathValues::SetNumber(OutputPathVariable variable, long long value) {
	m_Values[variable] = Value{ValueType::Number, std::wstring(), value};
}

void COutputPathValues::SetTemplate(OutputPathVariable variable, const std::wstring & value) {
	m_Values[variable] = Value{ValueType::Template, value, 0};
}

std::wstring COutputPathValues::Expand(const std::wstring & value, unsigned keepUnset, unsigned * outUnset) const {
	std::wstring result;
	unsigned unset = 0;
	Expand(value, keepUnset, 0, unset, result);
	if (outUnset) *outUnset = unset;
	return result;
}

void COutputPathValues::Expand(const std::wstring & value, unsigned keepUnset, unsigned expanding, unsigned & outUnset, std::wstring & outResult) const {
	// If anything is kept, the result is a template again and literal text needs to be escaped.
	const bool escape = 0 != keepUnset;

	std::vector<Token> tokens;
	std::wstring error;
	if (!Parse(value, tokens, error)) {
		// Should not happen for validated templates, treat as literal text.
		if (escape) AppendEscaped(outResult, value);
		else outResult += value;
		return;
	}

	for (auto & token : tokens) {
		if (nullptr == token.Variable) {
			if (escape) AppendEscaped(outResult, token.Text);
			else outResult += token.Text;
			continue;
		}

		OutputPathVariable variable = token.Variable->Variable;
		auto it = m_Values.find(variable);
		if (it == m_Values.end() || (expanding & variable)) {
			outUnset |= variable;
			if (keepUnset & variable) AppendVariable(outResult, token);
			continue;
		}

		const Value & itValue = it->second;
		switch (itValue.Type) {
		case ValueType::String:
			if (escape) AppendEscaped(outResult, itValue.String);
			else outResult += itValue.String;
			break;
		case ValueType::Number:
			outResult += FormatNumber(itValue.Number, 0 <= token.Width ? token.Width : (std::max)(0, token.Variable->DefaultWidth));
			break;
		case ValueType::Template:
			Expand(itValue.String, keepUnset, expanding | variable, outUnset, outResult);
			break;
		}
	}
}

// Functions ///////////////////////////////////////////////////////////////////

bool OutputPathTemplate_Validate(const char * value, unsigned allowedVariables, unsigned requiredVariables, std::string & outError) {
	std::wstring wideValue;
	if (!UTF8StringToWideString(value, wideValue)) {
		outError = "Invalid UTF-8.";
		return false;
	}

	std::vector<Token> tokens;
	std::wstring error;
	if (!Parse(wideValue, tokens, error)) {
		if (!WideStringToUTF8String(error.c_str(), outError)) outError = "Syntax error.";
		return false;
	}

	unsigned used = 0;
	for (auto & token : tokens) {
		if (nullptr == token.Variable) continue;
		used |= token.Variable->Variable;
	}

	if (unsigned notAllowed = used & ~allowedVariables) {
		outError = "Variables not allowed here: " + OutputPathTemplate_GetVariableNames(notAllowed);
		return false;
	}

	if (unsigned missing = requiredVariables & ~used) {
		outError = "Missing required variables: " + OutputPathTemplate_GetVariableNames(missing);
		return false;
	}

	return true;
}

unsigned OutputPathTemplate_GetVariables(const std::wstring & value) {
	std::vector<Token> tokens;
	std::wstring error;
	if (!Parse(value, tokens, error)) return 0;

	unsigned result = 0;
	for (auto & token : tokens) {
		if (nullptr != token.Variable) result |= token.Variable->Variable;
	}
	return result;
}

std::string OutputPathTemplate_GetVariableNames(unsigned variables) {
	std::string result;
	for (auto & info : g_Variables) {
		if (0 == (variables & info.Variable)) continue;
		if (!result.empty()) result += " ";
		result += "{";
		for (const wchar_t * p = info.Name; *p; ++p) result += (char)*p; // names are ASCII.
		result += "}";
	}
	return result;
}

long long OutputPathTemplate_FindMaxTakeNumber(const std::wstring & value) {
	std::vector<Token> tokens;
	std::wstring error;
	if (!Parse(value, tokens, error)) return -1;

	struct Element {
		wchar_t Char;
		OutputPathVariable Variable; // OutputPathVariable_None for literal characters.
	};

	std::vector<Element> elements;
	for (auto & token : tokens) {
		if (nullptr == token.Variable) {
			for (wchar_t c : token.Text) elements.push_back(Element{c, OutputPathVariable_None});
		}
		else {
			elements.push_back(Element{L'\0', token.Variable->Variable});
		}
	}

	size_t takeIndex = 0;
	while (takeIndex < elements.size() && OutputPathVariable_TakeNumber != elements[takeIndex].Variable) ++takeIndex;
	if (elements.size() <= takeIndex) return -1;

	// The path component that contains the (first) take number:
	size_t componentBegin = 0;
	for (size_t i = 0; i < takeIndex; ++i) {
		if (OutputPathVariable_None == elements[i].Variable && IsPathSeparator(elements[i].Char)) componentBegin = i + 1;
	}
	size_t componentEnd = elements.size();
	for (size_t i = takeIndex + 1; i < elements.size(); ++i) {
		if (OutputPathVariable_None == elements[i].Variable && IsPathSeparator(elements[i].Char)) {
			componentEnd = i;
			break;
		}
	}

	std::wstring directory;
	for (size_t i = 0; i < componentBegin; ++i) {
		if (OutputPathVariable_None != elements[i].Variable) return -1; // Unknown folder, can not search.
		directory += elements[i].Char;
	}

	std::wstring pattern;
	bool firstTakeNumber = true;
	for (size_t i = componentBegin; i < componentEnd; ++i) {
		const Element & element = elements[i];
		if (OutputPathVariable_None == element.Variable) {
			if (wcschr(L"\\^$.|?*+()[]{}", element.Char)) pattern += L'\\';
			pattern += element.Char;
		}
		else if (OutputPathVariable_TakeNumber == element.Variable) {
			pattern += firstTakeNumber ? L"(\\d+)" : L"\\d+";
			firstTakeNumber = false;
		}
		else {
			pattern += L".*";
		}
	}

	std::wregex regex;
	try {
		regex.assign(pattern, std::regex_constants::ECMAScript | std::regex_constants::icase);
	}
	catch (const std::regex_error &) {
		return -1;
	}

	long long result = -1;

	WIN32_FIND_DATAW findData;
	HANDLE hFindFile = FindFirstFileW((directory + L"*").c_str(), &findData);
	if (INVALID_HANDLE_VALUE == hFindFile) return -1;
	do {
		std::wcmatch match;
		if (std::regex_match(findData.cFileName, match, regex) && 2 <= match.size()) {
			std::wstring digits = match[1].str();
			if (digits.size() <= 15) { // Ignore absurdly large numbers.
				long long number = _wcstoi64(digits.c_str(), nullptr, 10);
				if (result < number) result = number;
			}
		}
	} while (FindNextFileW(hFindFile, &findData));
	FindClose(hFindFile);

	return result;
}

// COutputPathSetting //////////////////////////////////////////////////////////

COutputPathSetting::COutputPathSetting(const char * defaultValue, unsigned allowedVariables, unsigned requiredVariables)
	: m_DefaultValue(defaultValue)
	, m_Value(defaultValue)
	, m_AllowedVariables(allowedVariables)
	, m_RequiredVariables(requiredVariables)
{
}

bool COutputPathSetting::GetWide(std::wstring & outValue) const {
	return UTF8StringToWideString(m_Value.c_str(), outValue);
}

bool COutputPathSetting::Set(const char * value) {
	std::string error;
	if (!OutputPathTemplate_Validate(value, m_AllowedVariables, m_RequiredVariables, error)) {
		advancedfx::Warning("AFXERROR: Invalid path template \"%s\": %s\n", value, error.c_str());
		return false;
	}
	m_Value = value;
	return true;
}

void COutputPathSetting::Console(ICommandArgs * args, const char * description) {
	int argC = args->ArgC();
	const char * arg0 = args->ArgV(0);

	if (2 == argC) {
		const char * arg1 = args->ArgV(1);
		if (0 == _stricmp("default", arg1)) m_Value = m_DefaultValue;
		else Set(arg1);
		return;
	}
	else if (2 < argC) {
		advancedfx::Warning("AFXERROR: Too many arguments, put the path template in quotes.\n");
		return;
	}

	advancedfx::Message(
		"%s default|\"<pathTemplate>\" - %s\n"
		"Variables: %s\n"
		, arg0, description
		, OutputPathTemplate_GetVariableNames(m_AllowedVariables).c_str()
	);
	if (m_RequiredVariables) {
		advancedfx::Message(
			"Required variables: %s\n"
			, OutputPathTemplate_GetVariableNames(m_RequiredVariables).c_str()
		);
	}
	advancedfx::Message(
		"Numeric variables take an optional zero-padding width, e.g. {TAKE_NUMBER:4}, use {{ for { and }} for }.\n"
		"Default value: \"%s\"\n"
		"Current value: \"%s\"\n"
		, m_DefaultValue.c_str()
		, m_Value.c_str()
	);
}

} // namespace advancedfx
