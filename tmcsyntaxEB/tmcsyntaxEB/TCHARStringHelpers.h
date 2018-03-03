#pragma once
TCHARString stringToTCHARString(const std::string& s);
TCHARString LPCSTRtoTCHARString(LPCSTR s);
void SetSelectedLanguage(LPCSTR l);
std::string TCHARStringToString(const TCHARString& s);

