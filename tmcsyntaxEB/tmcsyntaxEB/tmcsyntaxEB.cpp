// Dll1.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"

ZLoadedMap LoadedLanguage;
unordered_map<HWND, HookedCtrl*> HookedControls;

#define gmexport extern "C" __declspec (dllexport)
WNDPROC fallBackProc = NULL;
COLORREF Last_LineNumberBackColor = 0x272727;
COLORREF Last_LineNumberColor = 0x4A4A4A;
string SelectedLanguage;
extern "C" {
	LRESULT CALLBACK WndEditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		bool found = (HookedControls.count(hWnd) != 0);
		if(!found)
		{
			//Oops this is bad
			if (fallBackProc != NULL)
			{
				MessageBox(GetActiveWindow(), L"Ooops!\r\nThis Code Box's Syntax Highlighting Hook Reference was Lost. This should never have happened. I will try to recover but an imminent crash is possible.", L"TMCSyntaxEB.dll", MB_ICONWARNING | MB_OK);
				HookedCtrl* t = new HookedCtrl(hWnd);
				HookedControls[hWnd] = t;
				t->SetOrigProc(fallBackProc);
				t->OnHooked(Last_LineNumberBackColor, Last_LineNumberColor);
				found = true;
			}
			else
			{
				MessageBox(GetActiveWindow(), L"Ooops!\r\nThis Code Box's Syntax Highlighting Hook Reference was Lost. This should never have happened. Recovery is Not Possible, an imminent crash is likely.", L"TMCSyntaxEB.dll", MB_ICONWARNING | MB_OK);
				DestroyWindow(hWnd);
			}
		}
		else
		{
			HookedCtrl* t = HookedControls[hWnd];
			return t->DoWindowProc(hWnd, message, wParam, lParam);
		}
		return 0l;
	}
	gmexport void TMC_SEB_SetHilite(HWND hWnd, LPCSTR LanguageName, DWORD LineNumberBackColor, DWORD LineNumberColor)
	{
		if (IsWindow(hWnd))
		{
			Last_LineNumberBackColor = LineNumberBackColor;
			Last_LineNumberColor = LineNumberColor;
			Language *Lang = NULL;
			if (LoadedLanguage.count(stringToTCHARString(LanguageName)))
			{
				Lang = LoadedLanguage[stringToTCHARString(LanguageName)];
			}
			else
			{
				MessageBox(GetActiveWindow(), L"Ooops!\r\nA request to hook to an Edit Box was made but the specified Syntax Highlighting Language was not loaded. Will try to default to a loaded language (if any).", L"TMCSyntaxEB.dll", MB_ICONINFORMATION | MB_OK);
				TCHARString ItemName;
				for (ZLoadedMap::const_iterator it = LoadedLanguage.begin(); it != LoadedLanguage.end(); ++it)
				{
					ItemName = it->first;
					Lang = LoadedLanguage[ItemName];
					break;
				}
				
			}
			WNDPROC wpOrigEditProc = (WNDPROC)GetWindowLong(hWnd, GWL_WNDPROC);
			if (wpOrigEditProc != WndEditProc)
			{
				if (fallBackProc == NULL) fallBackProc = wpOrigEditProc;
				SetWindowLong(hWnd, GWL_WNDPROC, (LONG_PTR)WndEditProc);
				if (HookedControls.count(hWnd))
				{
					HookedCtrl* t = HookedControls[hWnd];
					t->SetOrigProc(wpOrigEditProc);
					t->OnHooked(LineNumberBackColor, LineNumberColor);
					t->SetLanguage(Lang);
				}
				else
				{
					HookedCtrl* t = new HookedCtrl(hWnd);
					HookedControls[hWnd] = t;
					t->SetOrigProc(wpOrigEditProc);
					t->OnHooked(LineNumberBackColor, LineNumberColor);
					t->SetLanguage(Lang);
				}

			}
			else
			{
				if (HookedControls.count(hWnd) == 0)
				{
					//Oops this is bad
					if (fallBackProc != NULL)
					{
						MessageBox(GetActiveWindow(), L"Ooops!\r\nThis Code Box's Syntax Highlighting Hook Reference was Lost. This should never have happened. I will try to recover but an imminent crash is possible.", L"TMCSyntaxEB.dll", MB_ICONWARNING | MB_OK);
						HookedCtrl* t = new HookedCtrl(hWnd);
						HookedControls[hWnd] = t;
						t->SetOrigProc(fallBackProc);
						t->OnHooked(LineNumberBackColor, LineNumberColor);
						t->SetLanguage(Lang);
					}
					else
					{
						MessageBox(GetActiveWindow(), L"Ooops!\r\nThis Code Box's Syntax Highlighting Hook Reference was Lost. This should never have happened. Recovery is Not Possible, an imminent crash is likely.", L"TMCSyntaxEB.dll", MB_ICONWARNING | MB_OK);
						DestroyWindow(hWnd);
					}
				}
				else
				{
					if (HookedControls.count(hWnd))
					{
						HookedCtrl* t = HookedControls[hWnd];
						t->SetLanguage(Lang);
					}
				}
			}
		}
		else
		{
			MessageBox(GetActiveWindow(), L"Ooops!\r\nA request to hook to a Non Existing Edit Box was made. This should never have happened. The request will be ignored.", L"TMCSyntaxEB.dll", MB_ICONINFORMATION | MB_OK);
			if (HookedControls.count(hWnd))
			{
				HookedCtrl* t = HookedControls[hWnd];
				delete t;
				HookedControls.erase(hWnd);
			}
		}
	}
	gmexport void TMC_SEB_RemoveHilite(HWND hWnd)
	{
		if (HookedControls.count(hWnd))
		{
			HookedCtrl* t = HookedControls[hWnd];
			delete t;
			HookedControls.erase(hWnd);
		}
	}
	
	gmexport void TMC_SEB_LoadLanguage(LPCSTR LanguageName, LPCSTR LanguageFile)
	{
		SelectedLanguage = LanguageName;
		if (!LoadedLanguage.count(stringToTCHARString(LanguageName)))
		{
			LoadedLanguage[stringToTCHARString(LanguageName)] = new Language(LanguageName, LanguageFile);
		}
		Language *Lang = NULL;
		if (LoadedLanguage.count(stringToTCHARString(LanguageName)))
		{
			Lang = LoadedLanguage[stringToTCHARString(LanguageName)];
		}
	}
	gmexport CHAR*  TMC_SEB_GetSelectedLanguage()
	{
		//ret = L"Hello";
		//ret2 = "Hello";
		return (CHAR*)SelectedLanguage.c_str();
	}
	gmexport void TMC_SEB_CleanUp(HWND hWnd)
	{
		//the items are all removed the element are deleted... their destructor func is called apparently
		HookedControls.clear(); 
		LoadedLanguage.clear();
	}
}//extern c
