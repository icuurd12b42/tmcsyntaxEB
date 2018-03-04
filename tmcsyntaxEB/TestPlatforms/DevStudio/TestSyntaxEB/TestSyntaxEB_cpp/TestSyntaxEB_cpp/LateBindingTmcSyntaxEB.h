#pragma once

extern "C" {
void TMC_SEB_InitSyntaxModule();
void TMC_SEB_SetHilite(HWND hWnd, const TCHAR* LanguageName, COLORREF LineNumberBackColor, COLORREF LineNumberColor);
void TMC_SEB_SetHiliteA(HWND hWnd, LPCSTR LanguageName, COLORREF LineNumberBackColor, COLORREF LineNumberColor);
void TMC_SEB_RemoveHilite(HWND hWnd);
void TMC_SEB_CleanUp();
LPCSTR TMC_SEB_GetSelectedLanguage();
void TMC_SEB_LoadLanguage(const TCHAR* LanguageName, const TCHAR* LanguageFile);
void TMC_SEB_LoadLanguageA(LPCSTR LanguageName, LPCSTR LanguageFile);
void TMC_SEB_LoadAllLanguageFiles();
void TMC_SEB_LoadAllLanguageFilesA();
}