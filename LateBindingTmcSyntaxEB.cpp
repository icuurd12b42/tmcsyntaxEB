#include "stdafx.h"
#include <Windows.h>
#include "LateBindingTmcSyntaxEB.h"
#include <string>  

using namespace std;



HMODULE tmcSyntaxhModule = NULL;
typedef void  (*TMCREMOVEHIHILITE) (HWND hWnd);
TMCREMOVEHIHILITE tmcRemoveHilite = NULL;

typedef void (*TMCSETHILITE) (HWND hWnd, const TCHAR* LanguageName, const TCHAR* ColorSpecName);
TMCSETHILITE tmcSetHilite = NULL;

typedef void (*TMCSETHILITEA) (HWND hWnd, LPCSTR LanguageName, LPCSTR ColorSpecName);
TMCSETHILITEA tmcSetHiliteA = NULL;

typedef void (*TMCLOADLANGUAGE) (const TCHAR* LanguageName, const TCHAR* LanguageFile);
TMCLOADLANGUAGE tmcLoadLanguage = NULL;

typedef void(*TMCLOADLANGUAGEA) (LPCSTR LanguageName, LPCSTR LanguageFile);
TMCLOADLANGUAGEA tmcLoadLanguageA = NULL;

typedef void(*TMCLOADCOLORSPEC) (const TCHAR* ColorSpecName, const TCHAR* ColorSpecFile);
TMCLOADCOLORSPEC tmcLoadColorSpec = NULL;

typedef void(*TMCLOADCOLORSPECA) (LPCSTR ColorSpecName, LPCSTR ColorSpecFile);
TMCLOADCOLORSPECA tmcLoadColorSpecA = NULL;

typedef void (*TMCCLEANUP) ();
TMCCLEANUP tmcCleanup = NULL;

typedef const TCHAR* (*TMCGETSELECTEDLANGUAGE) ();
TMCGETSELECTEDLANGUAGE tmcGetSelectedLanguage = NULL;

typedef LPCSTR (*TMCGETSELECTEDLANGUAGEA) ();
TMCGETSELECTEDLANGUAGEA tmcGetSelectedLanguageA = NULL;

typedef const TCHAR* (*TMCGETSELECTEDCOLORSPEC) ();
TMCGETSELECTEDCOLORSPEC tmcGetSelectedColorSpec = NULL;

typedef LPCSTR(*TMCGETSELECTEDCOLORSPECA) ();
TMCGETSELECTEDCOLORSPECA tmcGetSelectedColorSpecA = NULL;

std::string TMC_SEB_LB_GetLastErrorAsString()
{
	//Get the error message, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0)
		return std::string(); //No error message has been recorded

	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	std::string message(messageBuffer, size);

	//Free the buffer.
	LocalFree(messageBuffer);

	return message;
}
void TMC_SEB_LB_InitSyntaxModule()
{
#ifdef _WIN64
	tmcSyntaxhModule = LoadLibrary(L"tmcsyntaxEBx64.dll");
#else
	tmcSyntaxhModule = LoadLibrary(L"tmcsyntaxEBx86.dll");
#endif
	if (tmcSyntaxhModule)
	{
		tmcRemoveHilite =  (TMCREMOVEHIHILITE) GetProcAddress(tmcSyntaxhModule, "TMC_SEB_RemoveHilite");
		tmcSetHilite = (TMCSETHILITE) GetProcAddress(tmcSyntaxhModule,  "TMC_SEB_SetHilite");
		tmcSetHiliteA = (TMCSETHILITEA)GetProcAddress(tmcSyntaxhModule, "TMC_SEB_SetHiliteA");
		tmcLoadLanguage = (TMCLOADLANGUAGE) GetProcAddress(tmcSyntaxhModule, "TMC_SEB_LoadLanguage");
		tmcLoadLanguageA = (TMCLOADLANGUAGEA)GetProcAddress(tmcSyntaxhModule, "TMC_SEB_LoadLanguageA");
		tmcLoadColorSpec = (TMCLOADCOLORSPEC)GetProcAddress(tmcSyntaxhModule, "TMC_SEB_LoadColorSpec");
		tmcLoadColorSpecA = (TMCLOADCOLORSPECA)GetProcAddress(tmcSyntaxhModule, "TMC_SEB_LoadColorSpecA");
		tmcCleanup = (TMCCLEANUP) GetProcAddress(tmcSyntaxhModule, "TMC_SEB_CleanUp");
		tmcGetSelectedLanguage = (TMCGETSELECTEDLANGUAGE) GetProcAddress(tmcSyntaxhModule, "TMC_SEB_GetSelectedLanguage");
		tmcGetSelectedLanguageA = (TMCGETSELECTEDLANGUAGEA)GetProcAddress(tmcSyntaxhModule, "TMC_SEB_GetSelectedLanguageA");
		tmcGetSelectedColorSpec = (TMCGETSELECTEDCOLORSPEC)GetProcAddress(tmcSyntaxhModule, "TMC_SEB_GetSelectedColorSpec");
		tmcGetSelectedColorSpecA = (TMCGETSELECTEDCOLORSPECA)GetProcAddress(tmcSyntaxhModule, "TMC_SEB_GetSelectedColorSpecA");
	}
}
void TMC_SEB_LB_SetHilite(HWND hWnd, const TCHAR* LanguageName, const TCHAR* ColorSpecName)
{
	if (tmcSetHilite != NULL)
	{
		tmcSetHilite(hWnd, LanguageName, ColorSpecName);
	}
}
void TMC_SEB_LB_SetHiliteA(HWND hWnd, LPCSTR LanguageName, LPCSTR ColorSpecName)
{
	if (tmcSetHiliteA != NULL)
	{
		tmcSetHiliteA(hWnd, LanguageName, ColorSpecName);
	}
}
void TMC_SEB_LB_RemoveHilite(HWND hWnd)
{
	if (tmcRemoveHilite != NULL)
	{
		tmcRemoveHilite(hWnd);
	}
}

void TMC_SEB_LB_CleanUp()
{
	if (tmcCleanup != NULL)
	{
		tmcCleanup();
	}
}

const TCHAR* TMC_SEB_LB_GetSelectedLanguage()
{
    static TCHAR nothing[] = {0,0};
	if (tmcGetSelectedLanguage != 0)
	{
		return tmcGetSelectedLanguage();

	}
	return nothing;
}

LPCSTR TMC_SEB_LB_GetSelectedLanguageA()
{
	static char nothing[] = "\0";
	if (tmcGetSelectedLanguageA != 0)
	{
		return tmcGetSelectedLanguageA();

	}
	return nothing;
}

const TCHAR* TMC_SEB_LB_GetSelectedColorSpec()
{
	static TCHAR nothing[] = { 0,0 };
	if (tmcGetSelectedColorSpec != 0)
	{
		return tmcGetSelectedColorSpec();

	}
	return nothing;
}

LPCSTR TMC_SEB_LB_GetSelectedColorSpecA()
{
	static char nothing[] = "\0";
	if (tmcGetSelectedColorSpecA != 0)
	{
		return tmcGetSelectedColorSpecA();

	}
	return nothing;
}

void TMC_SEB_LB_LoadLanguage(const TCHAR* LanguageName, const TCHAR* LanguageFile)
{
	if (tmcLoadLanguage != 0)
	{
		tmcLoadLanguage(LanguageName, LanguageFile);
	}
}

void TMC_SEB_LB_LoadLanguageA(LPCSTR LanguageName, LPCSTR LanguageFile)
{
	if (tmcLoadLanguageA != 0)
	{
		tmcLoadLanguageA(LanguageName, LanguageFile);
	}
}

void TMC_SEB_LB_LoadColorSpec(const TCHAR* ColorSpecName, const TCHAR* ColorSpecFile)
{
	if (tmcLoadColorSpec != 0)
	{
		tmcLoadColorSpec(ColorSpecName, ColorSpecFile);
	}
}

void TMC_SEB_LB_LoadColorSpecA(LPCSTR ColorSpecName, LPCSTR ColorSpecFile)
{
	if (tmcLoadColorSpecA != 0)
	{
		tmcLoadColorSpecA(ColorSpecName, ColorSpecFile);
	}
}



//assumes the SyntaxEBLangFiles folder is in the same location as the dll
void TMC_SEB_LB_LoadAllLanguageFiles()
{
	if(tmcSyntaxhModule)
	{
		char wcFilename[MAX_PATH * 4];
		GetModuleFileNameA(tmcSyntaxhModule, wcFilename, MAX_PATH * 4);
		string Path = wcFilename;
		//- len of "tmcsyntaxEBx??.dll"
		Path = Path.substr(0, Path.length() - 18);
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
				TMC_SEB_LB_LoadLanguageA(ln, lf);
			} while (FindNextFileA(hFind, &FindFileData));

			FindClose(hFind);
		}
		else
		{
#ifdef _WIN64
			string tmcModuleName = "tmcsyntaxEBx64.dll";
#else
			string tmcModuleName = "tmcsyntaxEBx86.dll";
#endif
			string error = TMC_SEB_LB_GetLastErrorAsString();
			error += "\r\n";
			error += "Is the folder 'SyntaxEBLangFiles' present in the directory where " + tmcModuleName + " is located?";
			MessageBoxA(GetActiveWindow(), error.c_str(), "Error", MB_ICONINFORMATION);
		}
	}
}

void TMC_SEB_LB_LoadAllColorSpecFiles()
{
	if (tmcSyntaxhModule)
	{
		char wcFilename[MAX_PATH * 4];
		GetModuleFileNameA(tmcSyntaxhModule, wcFilename, MAX_PATH * 4);
		string Path = wcFilename;
		//- len of "tmcsyntaxEBx??.dll"
		Path = Path.substr(0, Path.length() - 18);
		Path += "SyntaxEBLangFiles\\";
		string SearchPth = Path;
		SearchPth += "*.col";
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
				TMC_SEB_LB_LoadColorSpecA(ln, lf);
			} while (FindNextFileA(hFind, &FindFileData));

			FindClose(hFind);
		}
		else
		{
#ifdef _WIN64
			string tmcModuleName = "tmcsyntaxEBx64.dll";
#else
			string tmcModuleName = "tmcsyntaxEBx86.dll";
#endif
			string error = TMC_SEB_LB_GetLastErrorAsString();
			error += "\r\n";
			error += "Is the folder 'SyntaxEBLangFiles' present in the directory where " + tmcModuleName + " is located?";
			MessageBoxA(GetActiveWindow(), error.c_str(), "Error", MB_ICONINFORMATION);
		}
	}
}

void TMC_SEB_LB_LoadAllLanguageAndColorSpecFiles()
{
	TMC_SEB_LB_LoadAllLanguageFiles();
	TMC_SEB_LB_LoadAllColorSpecFiles();
}