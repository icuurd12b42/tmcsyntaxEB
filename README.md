# tmcsyntaxEB

Code Syntax Highlighting for Windows Standard Edit Box Control

This code hooks to the standard windows edit box MainWindowProc and supplements it's drawing to allow code syntax highlighting.

In theory, you simply need to call the dll functions to hook to an existing windows edit box passing its window handle and the syntax to use.


![alt text](https://cdn.pbrd.co/images/HabSMJC.png)


##Features


1. No complex libraries and system to add to your project
2. Just tell it the edit box you want to do highliting with and you are off
3. Colorize Keywords, Literals, Functions, Classes, Single Line Comment, Multi Line Comment, Quotes, Numbers, Braces and so on
4. Editing Features, Unlimited Undo/Redo, Indent, Block Comment, Find, Cut, Copy Paste, Select All
5. Switch Language Feature
6. Toggle Highliting On Off
7. Line Numbering (On the side of the box), Toggle On Off
8. Multiple syntax support
9. Simple Interface


##Limitations

1. 64K Limit on windows 32
2. Alpha Release!!


##ALPHA RELEASE

-Tested with AutoHotKey Ansi 32 BIT Version 1.1.26.01


##API

All exported functions are non mangled cdecl convention do you can statically link in your project or load the library manually.

+ export void TMC_SEB_LoadLanguage(LPCSTR LanguageName, LPCSTR LanguageFile)

  This functions loads a language file in the system. 

  LanguageName is the name you would reference the laguage with later on. This name is also displayed in the context menu in the select language option

  LanguageFile is the language.lng file you wish to load

  Commonly you would Load all your files at startup. it is also commont to uses the filename without extention for the Language Name

+ export void TMC_SEB_SetHilite(HWND hWnd, LPCSTR LanguageName, DWORD LineNumberBackColor, DWORD LineNumberColor)

  This function hooks the edit box and tells it to perform highligthing using the language specified

  hWnd is the Edit Box handle

  LanguageName is the name you used loading the file

  LineNumberBackColor is the color of the parent window

  LineNumberColor is the color of the code line shown to the side of the edit box


  You have to provide room to the left of the edit box for the line numbers


+ export void TMC_SEB_RemoveHilite(HWND hWnd)

  This function removes the highlighting from the box. Good behaviour suggest you should call this when you destroy the parent window of the edit box. You can call SetHilite again on the same edit box handle to re-enable highlighting.

  It is not required the edit box still exists for the function to work.


+ export CHAR*  TMC_SEB_GetSelectedLanguage()

  Gets the selected language the user chose when editing. In memeory this is set to the language you last called SetHilite with.

  It is also set when the user changes language while editing

  If you get the language back every time a dialog closes and use it in the next dialog create, this will show a consistent interface with the language the user selected every time.

  Good practice is to save the setting in an ini file for the next time your program is run


+ export void TMC_SEB_CleanUp(HWND hWnd)

  This will free all the memory used by the system. Unhooks all windows, releases every language. Call when your program ends. It's optional, Windows takes care of this really


##AutoHotKey Ansi 32 BIT Version 1.1.26.01 Release Notes

  You must load the library manually as the dll must stay in memory for the duration of the application execution
  See the AutoHotKey sub folder for details


