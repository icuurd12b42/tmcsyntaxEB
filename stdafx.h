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
#include <list>
#include <thread>
#include <algorithm>

using namespace std;


//Defines and macros


#define repeat(n) for (size_t __i = max(0,n); __i > 0; __i--)
#define SIGN(x) (x > 0) - (x < 0)
#define MAKEPOINTRESULT(point,result) {point.x = LOWORD(result); point.y = HIWORD(result);} 
#define KEY_STATE_DOWN(n) ((GetKeyState(n) & 0x8000) == 0x8000)
class HookedCtrl;


#define UM_INTERNAL_GETEBHOOK (WM_USER+1000)
#define UM_INTERNAL_FORCEREFRESH (WM_USER +1001)
#define UM_INTERNAL_DOSEARCH (WM_USER +1002)
#define UM_INTERNAL_WORDSELECTED (WM_USER +1003)
#define UM_INTERNAL_CANCELED (WM_USER +1004)
#define UM_INTERNAL_REFOCUS (WM_USER +1005)

// My Program Headers Here
#include "wstringex.h"
typedef std::list<wstringex> WORDLIST;
#include "LateBindingTmcSyntaxEB.h"
#include "DrawingHelpers.h"
#include "TCHARStringHelpers.h"
#include "Language.h"
#include "ColorSpec.h"
#include "debug.h"
#include "Parser.h"
#include "WordListDialog.h"
#include "SearchDialog.h"
#include "HookedCtrl.h"
typedef unordered_map<wstring, Language*> LOADEDLANGUAGESMAP;
typedef unordered_map<wstring, ColorSpec*> LOADEDCOLORSPECSMAP;

