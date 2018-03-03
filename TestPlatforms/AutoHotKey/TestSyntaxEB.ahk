;TO DO 
;Figure out why ReadIni is returning "ERROR"
;Figure out why Height is no longer a valid Variable in the OnResize Event
#NoEnv
#SingleInstance, FORCE
#Warn ALL
#Persistent
SetWorkingDir %A_ScriptDir%  ; Ensures a consistent starting directory.

;SingleInstance FORCE Alternative. The DLL does not like a reload
if(WinExist("Test Syntax EB"))
{
    DllCall("SetForegroundWindow", "int", WinExist("Test Syntax EB"))
    ExitApp
    return
}
#Include tmcSyntaxEB.ahk

OnExit("Cleanup")

ReadIni(Filename, Section, Key , Default)
{
    IniRead, OutputVar, %Filename%, %Section%, %Key% , %Default%
    return OutputVar
}
WriteIni(Filename, Section, Key , Value)
{
    IniWrite, %Value%, %Filename%, %Section%, %Key%
}

Cleanup(ExitReason, ExitCode)
{
    TMC_SEB_CleanUp()
    WriteIni("Settings.ini","CodeBox","Syntax", gCodeBoxSyntax)
}
global gCodeBoxSyntax := "HKS + JScript 3.0 Syntax"
;For some reason this retursn ERROR
;global gCodeBoxSyntax := ReadIni("Settings.ini","CodeBox","Syntax", "HKS + JScript 3.0 Syntax")
MsgBox, 4, , %gCodeBoxSyntax%
global gWindowBackColor := 272727
global gLineNumberBackColor := 0x272727
global gLineNumberColor := 0x4A4A4A

TMC_EB_InitSyntaxModule()
TMC_EB_LoadAllLanguageFiles()


global gCode
gCode =
(
/*
Something to do
*/
function MyFunction() {
{
    try //Try to do this...
    {
        Send("Hello");
        HKS.Action.Run("AnAction");
        var pi = Math.PI
        pi *= 0.5;
        OutputDebug(pi.toString());
    }
    catch(e) //catch error
    {
    }
}
MyFunction();

)

global gCodeBoxHandle
global gCodeTextBox
global MainUIWindow_
MainUIWindow_CreateWindow()
{
    Gui, MainUIWindow_:Default
    ;allow resize
    Gui, Color, %gWindowBackColor%
    Gui, +resize +OwnDialogs -MinimizeBox
    Gui, +MinSize600x400
    
    Gui, Font, s12, Courier New
    Gui, Add, Edit, x30 w560 r15 -Wrap vgCodeTextBox hwndgCodeBoxHandle, %gCode%
    TMC_SEB_SetHilite(gCodeBoxHandle,gCodeBoxSyntax,gLineNumberBackColor,gLineNumberColor)
}
MainUIWindow_Show()
{
    Gui, MainUIWindow_:Show,, Test Syntax EB
}
;Create the window
MainUIWindow_CreateWindow()
;Show the Window
MainUIWindow_Show()

;event handles for MainUIWindow_
MainUIWindow_GuiSize(GuiHwnd, EventInfo, Width, Height)
{
    Gui, MainUIWindow_:Default
    ;and for some reason Height is now erroring at ahk startup
    MovePos := " W" . (Width - 40) ;. " H". (Height)
    GuiControl, Move, gCodeTextBox, %MovePos%
}
MainUIWindow_GuiClose()
{
    TMC_SEB_RemoveHilite(gCodeBoxHandle)
}
