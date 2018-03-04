#include "stdafx.h"
#include "LateBindingTmcSyntaxEB.h"
#include <string>  

using namespace std;


extern  "C"     {
HMODULE tmcSyntaxhModule = NULL;
typedef void  (*TMCREMOVEHIHILITE) (HWND hWnd);
TMCREMOVEHIHILITE tmcRemoveHilite = NULL;

typedef void (*TMCSETHILITE) (HWND hWnd, const TCHAR* LanguageName, COLORREF LineNumberBackColor,  COLORREF LineNumberColor);
TMCSETHILITE tmcSetHilite = NULL;

typedef void (*TMCSETHILITEA) (HWND hWnd, LPCSTR LanguageName, COLORREF LineNumberBackColor, COLORREF LineNumberColor);
TMCSETHILITEA tmcSetHiliteA = NULL;

typedef void (*TMCLOADLANGUAGE) (const TCHAR* LanguageName, const TCHAR* LanguageFile);
TMCLOADLANGUAGE tmcLoadLanguage = NULL;

typedef void(*TMCLOADLANGUAGEA) (LPCSTR LanguageName, LPCSTR LanguageFile);
TMCLOADLANGUAGEA tmcLoadLanguageA = NULL;

typedef void (*TMCCLEANUP) ();
TMCCLEANUP tmcCleanup = NULL;

typedef LPCSTR (*TMCGETSELECTEDLANGUAGE) ();
TMCGETSELECTEDLANGUAGE tmcGetSelectedLanguage = NULL;



void TMC_SEB_InitSyntaxModule()
{
	tmcSyntaxhModule = LoadLibrary(L"tmcsyntaxEB.dll");

	if (tmcSyntaxhModule)
	{
		tmcRemoveHilite =  (TMCREMOVEHIHILITE) GetProcAddress(tmcSyntaxhModule, "TMC_SEB_RemoveHilite");
		tmcSetHilite = (TMCSETHILITE) GetProcAddress(tmcSyntaxhModule,  "TMC_SEB_SetHilite");
		tmcSetHiliteA = (TMCSETHILITEA)GetProcAddress(tmcSyntaxhModule, "TMC_SEB_SetHiliteA");
		tmcLoadLanguage = (TMCLOADLANGUAGE) GetProcAddress(tmcSyntaxhModule, "TMC_SEB_LoadLanguage");
		tmcLoadLanguageA = (TMCLOADLANGUAGEA)GetProcAddress(tmcSyntaxhModule, "TMC_SEB_LoadLanguageA");
		tmcCleanup = (TMCCLEANUP) GetProcAddress(tmcSyntaxhModule, "TMC_SEB_CleanUp");
		tmcGetSelectedLanguage = (TMCGETSELECTEDLANGUAGE) GetProcAddress(tmcSyntaxhModule, "TMC_SEB_GetSelectedLanguage");
	}
}
void TMC_SEB_SetHilite(HWND hWnd, const TCHAR* LanguageName, COLORREF LineNumberBackColor, COLORREF LineNumberColor)
{
	if (tmcSetHilite != NULL)
	{
		tmcSetHilite(hWnd, LanguageName, LineNumberBackColor, LineNumberColor);
	}
}
void TMC_SEB_SetHiliteA(HWND hWnd, LPCSTR LanguageName, COLORREF LineNumberBackColor, COLORREF LineNumberColor)
{
	if (tmcSetHiliteA != NULL)
	{
		tmcSetHiliteA(hWnd, LanguageName, LineNumberBackColor, LineNumberColor);
	}
}
void TMC_SEB_RemoveHilite(HWND hWnd)
{
	if (tmcRemoveHilite != NULL)
	{
		tmcRemoveHilite(hWnd);
	}
}

void TMC_SEB_CleanUp()
{
	if (tmcCleanup != NULL)
	{
		tmcCleanup();
	}
}

LPCSTR TMC_SEB_GetSelectedLanguage()
{
    static LPCSTR nothing = "\0\0";
	if (tmcGetSelectedLanguage != 0)
	{
		return tmcGetSelectedLanguage();

	}
	return nothing;
}
void TMC_SEB_LoadLanguage(const TCHAR* LanguageName, const TCHAR* LanguageFile)
{
	if (tmcLoadLanguage != 0)
	{
		tmcLoadLanguage(LanguageName, LanguageFile);
	}
}

void TMC_SEB_LoadLanguageA(LPCSTR LanguageName, LPCSTR LanguageFile)
{
	if (tmcLoadLanguageA != 0)
	{
		tmcLoadLanguageA(LanguageName, LanguageFile);
	}
}

//assumes the SyntaxEBLangFiles folder is in the same location as the dll
void TMC_SEB_LoadAllLanguageFiles()
{
	TCHAR wcFilename[MAX_PATH * 4];
	GetModuleFileName(tmcSyntaxhModule, wcFilename, MAX_PATH * 4);
	wstring Path = wcFilename;
	//- len of "tmcsyntaxEB.dll"
	Path = Path.substr(0, Path.length() - 15);
	Path += L"SyntaxEBLangFiles\\";
	wstring SearchPth = Path;
	SearchPth += L"*.lng";
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	hFind = FindFirstFile(SearchPth.c_str(), &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do {
			wstring Filename = FindFileData.cFileName;
			wstring LangFile = Path + Filename;
			wstring LangName = Filename.substr(0, Filename.length() - 4);
			const TCHAR* ln = LangName.c_str();
			const TCHAR* lf = LangFile.c_str();
			TMC_SEB_LoadLanguage(ln, lf);
		} while (FindNextFile(hFind, &FindFileData));

		FindClose(hFind);
	}
}

//assumes the SyntaxEBLangFiles folder is in the same location as the dll
void TMC_SEB_LoadAllLanguageFilesA()
{
	char wcFilename[MAX_PATH * 4];
	GetModuleFileNameA(tmcSyntaxhModule, wcFilename, MAX_PATH * 4);
	string Path = wcFilename;
	//- len of "tmcsyntaxEB.dll"
	Path = Path.substr(0, Path.length() - 15);
	Path += "SyntaxEBLangFiles\\";
	string SearchPth = Path;
	SearchPth += "*.lng";
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind;
	hFind = FindFirstFileA(SearchPth.c_str(), &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do {
			string Filename = FindFileData.cFileName;
			string LangFile = Path + Filename;
			string LangName = Filename.substr(0, Filename.length() - 4);
			LPCSTR ln = LangName.c_str();
			LPCSTR lf = LangFile.c_str();
			TMC_SEB_LoadLanguageA(ln, lf);
		} while (FindNextFileA(hFind, &FindFileData));

		FindClose(hFind);
	}
}
}