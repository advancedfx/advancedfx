#define NOMINMAX
#include "Globals.h"
#include <sstream>
#include <iomanip> 
#include <algorithm>
#include <bitset>

void ErrorBox(char const * messageText) {
	MessageBoxA(0, messageText, "Error - AfxHookSource2", MB_OK|MB_ICONERROR);
}

void ErrorBox() {
	ErrorBox("Something went wrong.");
}

size_t getAddress(HMODULE dll, char const* pattern)
{
	Afx::BinUtils::ImageSectionsReader sections((HMODULE)dll);
	Afx::BinUtils::MemRange textRange = sections.GetMemRange();
	Afx::BinUtils::MemRange result = FindPatternString(textRange, pattern);
	if (result.IsEmpty()) {
		advancedfx::Warning("Could not find address for pattern: %s\n", pattern);
		return 0;
	} else {
		return result.Start;
	}
};

// feel free to move it in more appropriate place
namespace afxUtils {
	std::string createTable(std::vector<std::vector<std::string>>& rows, char* delimiter, char* emptyRowDelimiter) {
		std::vector<size_t> columnWidths(rows[0].size(), 0);
		
		int totalWidth = 0;
		int delimiterLength = delimiter ? strlen(delimiter) : 0;

		if(!emptyRowDelimiter) emptyRowDelimiter = " ";

		for (auto& row : rows) {
			if (row.empty()) continue;
			for (size_t i = 0; i < row.size(); ++i) {
				if (i >= columnWidths.size()) {
					columnWidths.resize(i + 1, 0);
				}
                size_t oldWidth = columnWidths[i];
                columnWidths[i] = std::max(columnWidths[i], row[i].size());
                totalWidth += columnWidths[i] - oldWidth;
			}
		}

		totalWidth += delimiterLength * (columnWidths.size() - 1);

		std::ostringstream stream;

		for (auto& row : rows) {
			if (row.empty()){
				stream << std::setw(totalWidth) << std::setfill('=') << "" << std::endl;
				continue;
			}
			for (size_t i = 0; i < row.size(); ++i) {
				stream << std::left << std::setw(columnWidths[i]) << std::setfill(' ') << row[i];
				if (i < row.size() - 1) {
					stream << delimiter;
				}
			}
			stream << std::endl;
		}

		return stream.str();
	};

	std::vector<std::string> splitString (std::string& str, std::string& delimiter)
	{
		std::vector<std::string> result;
		size_t pos = 0;
		std::string token;

		while ((pos = str.find(delimiter)) != std::string::npos) {
			token = str.substr(0, pos);
			result.push_back(token);
			str.erase(0, pos + delimiter.length());
		}

		result.push_back(str);

		return result;
	};

	std::string rgbaToHex(std::string str, advancedfx::Con_Printf_t &conMessage)
	{
		std::string result = "";
		std::string delim = ",";
		auto array = splitString(str, delim);

		if (array.size() != 4) 
		{
			std::string arrayValue;
			for (int i = 0; i < array.size(); ++i)
			{
				arrayValue.append(array[i]); 
				if (i < array.size() - 1) arrayValue.append(delim);
			};
			conMessage(
				"Error: cannot parse %s\n"
				"Expected 4 values.\n"
				, arrayValue.c_str()
			);

			return result; 
		};
		
		for (std::string item : array)
		{
			int val = -1;
			try { val = std::stoi(item); } catch (...) { };

			if (val < 0 || val > 255) 
			{
				conMessage(
					"Error: cannot parse %s\n"
					"Expected a number between 0 and 255 (inclusive).\n"
					, item.c_str()
				);

				result = "";
				return result; 
			};

			std::stringstream stream;
			stream << std::setfill('0') << std::setw(2) << std::hex << val;
			result.append(stream.str());
		};
		
		return result;
	};

	// string length must be 8
	uint32_t hexStrToInt(std::string str)
	{
		if (str.length() != 8) return 0;

		uint32_t result = 0;
		std::stringstream stream;
		// backwards due to endianness
		stream << std::hex << str.substr(6, 2) << str.substr(4, 2) << str.substr(2, 2) << str.substr(0, 2);
		stream >> result;

		return result;
	}
};