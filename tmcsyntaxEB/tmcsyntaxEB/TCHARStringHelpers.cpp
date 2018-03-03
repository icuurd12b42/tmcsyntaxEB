#include "stdafx.h"

TCHARString stringToTCHARString(const std::string& s)
{
	TCHARString temp(s.length(), L' ');
	std::copy(s.begin(), s.end(), temp.begin());
	return temp;
}
TCHARString LPCSTRtoTCHARString(LPCSTR s)
{
	stringstream ss;
	ss << s;
	return stringToTCHARString(ss.str());
}
std::string TCHARStringToString(const TCHARString& s)
{
	std::string temp(s.length(), ' ');
	std::copy(s.begin(), s.end(), temp.begin());
	return temp;
}


