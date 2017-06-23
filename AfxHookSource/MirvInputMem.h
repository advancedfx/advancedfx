#pragma once

#include "WrpConsole.h"

#include <string>
#include <map>

class MirvInputMem
{
public:
	void Console(IWrpCommandArgs * args);

private:
	struct CData
	{
		double Origin[3];
		double Angles[3];
		double Fov;

		CData()
		{

		}

		CData(double x, double y, double z, double  yPitch, double xRoll, double zYaw, double fov)
		{
			Origin[0] = x;
			Origin[1] = y;
			Origin[2] = z;
			Angles[0] = yPitch;
			Angles[1] = xRoll;
			Angles[2] = zYaw;
			Fov = fov;
		}
	};

	// this is not unicode aware, but whatever:
	struct ci_less : std::binary_function<std::string, std::string, bool>
	{
		struct ci_less_char : public std::binary_function<unsigned char, unsigned char, bool>
		{
			bool operator() (const unsigned char& c1, const unsigned char& c2) const {
				return tolower(c1) < tolower(c2);
			}
		};
		bool operator() (const std::string & s1, const std::string & s2) const {
			return std::lexicographical_compare(
				s1.begin(), s1.end(),
				s2.begin(), s2.end(),
				ci_less_char());
		}
	};

	std::map<std::string, CData, ci_less> m_Map;

	bool Save(wchar_t const * fileName);

	bool Load(wchar_t const * fileName);

};

extern MirvInputMem g_MirvInputMem;
