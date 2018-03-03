// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#define STRICT

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <Windowsx.h>

//classes included
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stack> 
using namespace std;

//Defines and macros
#ifndef UNICODE  
typedef std::string TCHARString;
#else
typedef std::wstring TCHARString;
#endif

#define repeat(n) for (int __i = n; __i > 0; __i--)

class HookedCtrl;



// My Program Headers Here
#include "TCHARStringHelpers.h"
#include "Language.h"
#include "debug.h"
#include "Parser.h"
#include "HookedCtrl.h"
typedef unordered_map<TCHARString, Language*> ZLoadedMap;


