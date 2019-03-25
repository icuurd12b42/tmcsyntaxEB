#pragma once
wstring StringToWString(const std::string& s);
wstring LPCSTRtoWString(LPCSTR s);

std::string WStringToString(const wstring& s);
bool IsInStr_i(wstring data, wstring toSearch);
bool IsInStr(wstring data, wstring toSearch);
bool StartsWith(wstring data, wstring toSearch);
wstring &ReplaceAll(wstring &str, const wstring& from, const wstring& to);




