global tmcSyntaxhModule := 0
global tmcRemoveHilite := 0
global tmcFuncSetHilite := 0
global tmcLoadLanguage := 0
global tmcCleanupSEB := 0
global tmcGetSelectedLanguage := 0

TMC_EB_InitSyntaxModule()
{
    tmcSyntaxhModule := DllCall("LoadLibrary", "Str", "tmcsyntaxEB.dll", "Ptr")
    ;MyOutputDebug("hModule: " . tmcSyntaxhModule)
    if(tmcSyntaxhModule != 0)
    {
        tmcRemoveHilite  := DllCall("GetProcAddress", Ptr, tmcSyntaxhModule, AStr, "TMC_SEB_RemoveHilite", "Ptr")
        tmcFuncSetHilite := DllCall("GetProcAddress", Ptr, tmcSyntaxhModule, AStr, "TMC_SEB_SetHilite", "Ptr")
        tmcLoadLanguage := DllCall("GetProcAddress", Ptr, tmcSyntaxhModule, AStr, "TMC_SEB_LoadLanguage", "Ptr")
        tmcCleanupSEB := DllCall("GetProcAddress", Ptr, tmcSyntaxhModule, AStr, "TMC_SEB_CleanUp", "Ptr")
        tmcGetSelectedLanguage := DllCall("GetProcAddress", Ptr, tmcSyntaxhModule, AStr, "TMC_SEB_GetSelectedLanguage", "Ptr")
    }
}
TMC_SEB_SetHilite(hWnd, LanguageName, LineNumberBackColor, LineNumberColor)
{
    if(tmcFuncSetHilite != 0)
    {
        DllCall(tmcFuncSetHilite, "UInt", hWnd, "AStr", LanguageName, "UInt", LineNumberBackColor, "UInt", LineNumberColor)
    }
}

TMC_SEB_RemoveHilite(hWnd)
{
   if(tmcRemoveHilite != 0)
    {
        DllCall(tmcRemoveHilite, "UInt", hWnd)
    }
}

TMC_SEB_CleanUp()
{
    if(tmcCleanupSEB != 0)
    {
        DllCall(tmcCleanupSEB)
    }
}

TMC_SEB_GetSelectedLanguage()
{
    if(tmcGetSelectedLanguage != 0)
    {
        return DllCall(tmcGetSelectedLanguage,AStr)

    }
    return ""
}
TMC_SEB_LoadLanguage(LanguageName,LanguageFile)
{
    if(tmcLoadLanguage != 0)
    {
        return DllCall(tmcLoadLanguage,"AStr",LanguageName, "AStr", LanguageFile)

    }
}
TMC_EB_LoadAllLanguageFiles()
{
    Loop, Files, %A_ScriptDir%\SyntaxEBLangFiles\*.*, R
    {
        SplitPath, A_LoopFileFullPath , OutFileName, OutDir, OutExtension, OutNameNoExt, OutDrive
        TMC_SEB_LoadLanguage(OutNameNoExt,A_LoopFileFullPath)
    }
}