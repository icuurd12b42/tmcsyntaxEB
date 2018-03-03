#include "stdafx.h"
#include "debug.h"

void debug(const char * t)
{
	string ss = "\r\n";
	ss += t;
	OutputDebugStringA(ss.c_str());
	/*
	stringstream ss;
	ss << "\r\nRect {" << rect.left << "," << rect.top << "," << rect.right << "," << rect.bottom << "}";
	ss << "\r\nPos {" << x << "," << y << "}";
	ss << "\r\nCharPos {" << charPos << "}";
	debug(ss.str().c_str());
	//MessageBoxA(GetActiveWindow(), ss.str().c_str(), "SetHilite", MB_ICONINFORMATION | MB_OK);
	*/
}
void debug(string &t)
{
	string ss = "\r\n";
	ss += t;
	OutputDebugStringA(ss.c_str());
	/*
	stringstream ss;
	ss << "\r\nRect {" << rect.left << "," << rect.top << "," << rect.right << "," << rect.bottom << "}";
	ss << "\r\nPos {" << x << "," << y << "}";
	ss << "\r\nCharPos {" << charPos << "}";
	debug(ss.str());
	//MessageBoxA(GetActiveWindow(), ss.str().c_str(), "SetHilite", MB_ICONINFORMATION | MB_OK);
	*/
}
void debug(TCHARString &t)
{
	TCHARString ss = L"\r\n";
	ss += t;
	OutputDebugString(ss.c_str());
	/*
	stringstream ss;
	ss << "\r\nRect {" << rect.left << "," << rect.top << "," << rect.right << "," << rect.bottom << "}";
	ss << "\r\nPos {" << x << "," << y << "}";
	ss << "\r\nCharPos {" << charPos << "}";
	debug(ss.str());
	//MessageBoxA(GetActiveWindow(), ss.str().c_str(), "SetHilite", MB_ICONINFORMATION | MB_OK);
	*/
}
void debug(stringstream &t)
{
	OutputDebugStringA(t.str().c_str());
	/*
	stringstream ss;
	ss << "\r\nRect {" << rect.left << "," << rect.top << "," << rect.right << "," << rect.bottom << "}";
	ss << "\r\nPos {" << x << "," << y << "}";
	ss << "\r\nCharPos {" << charPos << "}";
	debug(ss);
	//MessageBoxA(GetActiveWindow(), ss.str().c_str(), "SetHilite", MB_ICONINFORMATION | MB_OK);
	*/
}
void debug(const int t)
{
	stringstream ss;
	ss << "\r\n" << t;
	OutputDebugStringA(ss.str().c_str());
}
void debug(const TCHAR t)
{
	TCHAR s[] = { t,0 };
	TCHARString TCHStr = L"\r\n";
	TCHStr+=s;
	
	OutputDebugString(TCHStr.c_str());
}
