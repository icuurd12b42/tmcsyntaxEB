#include "stdafx.h"

wstring StringToWString(const std::string& s)
{
	wstring temp(s.length(), L' ');
	std::copy(s.begin(), s.end(), temp.begin());
	return temp;
}
wstring LPCSTRtoWString(LPCSTR s)
{
	stringstream ss;
	ss << s;
	return StringToWString(ss.str());
}
std::string WStringToString(const wstring& s)
{
	std::string temp(s.length(), ' ');
	std::copy(s.begin(), s.end(), temp.begin());
	return temp;
}

bool IsInStr_i(wstring data, wstring toSearch)
{
	if (data == L"") return false;
	if (toSearch == L"") return false;
	// Convert complete given String to lower case
	std::transform(data.begin(), data.end(), data.begin(), ::tolower);
	// Convert complete given Sub String to lower case
	std::transform(toSearch.begin(), toSearch.end(), toSearch.begin(), ::tolower);

	return data.find(toSearch) != -1;
}

bool IsInStr(wstring data, wstring toSearch)
{
	if (data == L"") return false;
	if (toSearch == L"") return false;
	
	return data.find(toSearch) != -1;
}
bool StartsWith(wstring data, wstring toSearch)
{
	if (data == L"") return false;
	if (toSearch == L"") return false;

	return data.find(toSearch) == 0;
}
wstring& ReplaceAll(wstring &str, const wstring& from, const wstring& to) {
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return str;
}