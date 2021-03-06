// Dll1.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"

LOADEDLANGUAGESMAP LoadedLanguages;
LOADEDCOLORSPECSMAP LoadedColorSpecs;
unordered_map<HWND, HookedCtrl*> HookedControls;

#define export extern "C" __declspec (dllexport)
WNDPROC fallBackProc = NULL;

wstring SelectedLanguage;
string SelectedLanguageA;
wstring SelectedColorSpec;
string SelectedColorSpecA;

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
				t->StoreWindowProcs(fallBackProc, WndEditProc);
				t->OnHooked();
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
	export void TMC_SEB_SetHilite(HWND hWnd, const TCHAR* LanguageName, const TCHAR* ColorSpecName)
	{
		SelectedLanguage = LanguageName;
		SelectedColorSpec = ColorSpecName;
		if (IsWindow(hWnd))
		{
			
			Language *Lang = NULL;
			if (LoadedLanguages.count(wstring(LanguageName)))
			{
				Lang = LoadedLanguages[wstring(LanguageName)];
			}
			else
			{
				MessageBox(GetActiveWindow(), L"Ooops!\r\nA request to hook to an Edit Box was made but the specified Syntax Highlighting Language was not loaded. Will try to default to a loaded language (if any).", L"TMCSyntaxEB.dll", MB_ICONINFORMATION | MB_OK);
				wstring ItemName;
				for (LOADEDLANGUAGESMAP::const_iterator it = LoadedLanguages.begin(); it != LoadedLanguages.end(); ++it)
				{
					ItemName = it->first;
					Lang = LoadedLanguages[ItemName];
					break;
				}
				
			}

			ColorSpec *ColSpec = NULL;
			if (LoadedColorSpecs.count(wstring(ColorSpecName)))
			{
				ColSpec = LoadedColorSpecs[wstring(ColorSpecName)];
			}
			else
			{
				MessageBox(GetActiveWindow(), L"Ooops!\r\nA request to hook to an Edit Box was made but the specified Highlighting Color Specification was not loaded. Will try to default to a loaded color specification (if any).", L"TMCSyntaxEB.dll", MB_ICONINFORMATION | MB_OK);
				wstring ItemName;
				for (LOADEDCOLORSPECSMAP::const_iterator it = LoadedColorSpecs.begin(); it != LoadedColorSpecs.end(); ++it)
				{
					ItemName = it->first;
					ColSpec = LoadedColorSpecs[ItemName];
					break;
				}

			}
			//failed entirely, create a default
			if (ColSpec == NULL) ColSpec = new ColorSpec();


			WNDPROC wpOrigEditProc = (WNDPROC)GetWindowLongPtr(hWnd, GWLP_WNDPROC);
			//WNDPROC wpOrigEditProc = (WNDPROC)GetWindowLong(hWnd, GWL_WNDPROC);
			if (wpOrigEditProc != WndEditProc)
			{
				if (fallBackProc == NULL) fallBackProc = wpOrigEditProc;
				SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)WndEditProc);
				//SetWindowLong(hWnd, GWL_WNDPROC, (LONG_PTR)WndEditProc);
				if (HookedControls.count(hWnd))
				{
					HookedCtrl* t = HookedControls[hWnd];
					t->StoreWindowProcs(wpOrigEditProc, WndEditProc);
					t->OnHooked();
					t->SetLanguage(Lang);
					t->SetColorSpec(ColSpec);
				}
				else
				{
					HookedCtrl* t = new HookedCtrl(hWnd);
					HookedControls[hWnd] = t;
					t->StoreWindowProcs(wpOrigEditProc, WndEditProc);
					t->OnHooked();
					t->SetLanguage(Lang);
					t->SetColorSpec(ColSpec);
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
						t->StoreWindowProcs(fallBackProc, WndEditProc);
						t->OnHooked();
						t->SetLanguage(Lang);
						t->SetColorSpec(ColSpec);
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
						t->SetColorSpec(ColSpec);
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
	export void TMC_SEB_SetHiliteA(HWND hWnd, LPCSTR LanguageName, LPSTR ColorSpecName)
	{
		string ln = LanguageName;
		wstring wln = StringToWString(ln);
		string cn = ColorSpecName;
		wstring wcn = StringToWString(cn);
		TMC_SEB_SetHilite(hWnd, wln.c_str(), wcn.c_str());
	}
	export void TMC_SEB_RemoveHilite(HWND hWnd)
	{
		if (HookedControls.count(hWnd))
		{
			HookedCtrl* t = HookedControls[hWnd];
			delete t;
			HookedControls.erase(hWnd);
		}
	}
	

	export void TMC_SEB_LoadLanguage(const TCHAR* LanguageName, const TCHAR* LanguageFile)
	{
		if (!LoadedLanguages.count(wstring(LanguageName)))
		{
			LoadedLanguages[wstring(LanguageName)] = new Language(LanguageName, LanguageFile);
		}
	}
	export void TMC_SEB_LoadLanguageA(LPCSTR LanguageName, LPCSTR LanguageFile)
	{
		string ln = LanguageName;
		string lf = LanguageFile;
		wstring wln = StringToWString(ln);
		wstring wlf = StringToWString(lf);
		TMC_SEB_LoadLanguage(wln.c_str(), wlf.c_str());
	}
	export CHAR*  TMC_SEB_GetSelectedLanguage()
	{
		return (CHAR*)SelectedLanguage.c_str();
	}
	export LPCSTR  TMC_SEB_GetSelectedLanguageA()
	{
		SelectedLanguageA = WStringToString(SelectedLanguage);
		return (LPCSTR)SelectedLanguageA.c_str();
	}

	export void TMC_SEB_LoadColorSpec(const TCHAR* ColorSpecName, const TCHAR* ColorSpecFile)
	{
		if (!LoadedColorSpecs.count(wstring(ColorSpecName)))
		{
			LoadedColorSpecs[wstring(ColorSpecName)] = new ColorSpec(ColorSpecName, ColorSpecFile);
		}
	}
	export void TMC_SEB_LoadColorSpecA(LPCSTR ColorSpecName, LPCSTR ColorSpecFile)
	{
		string cn = ColorSpecName;
		string cf = ColorSpecFile;
		wstring wcn = StringToWString(cn);
		wstring wcf = StringToWString(cf);
		TMC_SEB_LoadColorSpec(wcn.c_str(), wcf.c_str());
	}
	export CHAR*  TMC_SEB_GetSelectedColorSpec()
	{
		return (CHAR*)SelectedColorSpec.c_str();
	}
	export LPCSTR  TMC_SEB_GetSelectedColorSpecA()
	{
		SelectedColorSpecA = WStringToString(SelectedColorSpec);
		return (LPCSTR)SelectedColorSpecA.c_str();
	}

	export void TMC_SEB_CleanUp()
	{
		//the items are all removed the element are deleted... their destructor func is called apparently
		HookedControls.clear(); 
		LoadedLanguages.clear();
		LoadedColorSpecs.clear();
	}
}//extern c
