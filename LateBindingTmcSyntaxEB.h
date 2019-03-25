#pragma once
//NOTES:
//SendMessage commands
//Along with the EM_ messages, which you should investigate to know how to control an edit box, while hooked
//these supplemental user messages allow to set and get options in the code box. Most the API is implemented that way

//performs a redo, like EM_UNDO WM_UNDO performs an undo, returs 0 on fail 1 on success. Related Msgs WM_UNDO, EM_UNDO, EM_CANUNDO, UM_REDO, UM_CANREDO
#define UM_REDO (WM_USER + 1) //To Test With Test App
//returns if redo is possible, like EM_CANUNDO for undo. Related Msgs WM_UNDO, EM_UNDO, EM_CANUNDO, UM_REDO, UM_CANREDO
#define UM_CANREDO (WM_USER + 2)
//Block indent unindent (isert tabs in a selection) wParam 1 to indent, 0 to unident 
#define UM_BLOCKINDENT (WM_USER + 3)
//Block comment/uncomment the selection, wParam 1 to comment, 0 to uncomment
#define UM_BLOCKCOMMENT (WM_USER + 4)
//Looks at character next to the caret to find matchting () [] {} <> "" '', return -1 on fail, or the found position on success
#define UM_MATCHBRACKET (WM_USER + 5)
//Turn Syntax Highlighting on or off without unhooking the edit box. wParam 0 Off 1 ON, 
#define UM_SETHILIGHT (WM_USER + 6)
//Returns 1 or 0 if highliting is on or off
#define UM_GETHILIGHT (WM_USER + 7)
//Sets if the line numbers should be shown or not, wParam 1 or 0, return the old value
#define UM_SETLINEMARGIN (WM_USER + 8)
//Returns 1 or 0 if line numbers are shown or not
#define UM_GETLINEMARGIN (WM_USER + 9)
//Tells the system to search for the next occurence of the selected text (Does the F3 command). wParam 0 is forwards, 1 is backwards, lParam is a 2 bit combination, bit 1, set to 1 for case sensitive, bit 2, set to 1 for Whole word, return -1 on error or the position of the found text
#define UM_SEARCHSEL (WM_USER + 10)
//You can bitwise or the 2 bits for various settings, but here is all possible options predefined
//Any case any location
#define UMSS_ANY 0x0
//case sensitive
#define UMSS_CS 0x1
//whole word
#define UMSS_WW 0x2
//case sensitive and whole word
#define UMSS_CSWW (UMSS_CSENSITIVE | UMSS_WHOLEWORD)

//Tells the edit box to got to a line number (wParam) and caret position (lParam) on the line, supplements the edit box EM_ features with exact code line and caret position on the line
#define UM_GOTOLINECOL (WM_USER + 11)
//These 2 are used to get the line and the column of a character. lparam is the Text Box Character Position, -1 will ask for the info at the caret location
#define UM_GETLINENUMBER (WM_USER + 12)
#define UM_GETCOLINDEX (WM_USER + 13)
//set the insert or overwrite mode, wParam 1 is insert, 0 is overwrite, 1 is default, returns the old mode
#define UM_SETINSERTMODE (WM_USER + 14) //To Test With Test App
//Gets the insert mode, return 1 if insert mode is on, 0 if overwrite
#define UM_GETINSERTMODE (WM_USER + 15) //To Test With Test App
//Sets the edit box No Hide Selection, wparam 1, dont hide selection when lost focus, wparam 0, hide selection when lost focus, this is tied to the ES_NOHIDESEL style wich is usually not set be default
#define UM_SETNOHIDESEL (WM_USER + 16)
//Get if NO HIDE SELECTION is on
#define UM_GETNOHIDESEL (WM_USER + 17)
//gets the actual number of lines of code, if word wrap is enabled, the method is wrap aware and will use a slow method if word warapping is on and wrapped lines are continuation of a code line
#define UM_GETCODELINECOUNT (WM_USER + 18)
//Since I broke the 64k barrier, the culprit was the EM_CHARFROMPOS, this message is a gift you will likely never use. it returns the character position only, no line data, using the full range of LRESULT. wParam is x, lparam is y, relative to the client rectangle, both are shorts internally
#define UM_CHARFROMPOS64k (WM_USER + 19)
//Returns if the search (F2 feature can be calle (if there is a selection in the box really)
#define UM_CANSEARCHSEL (WM_USER + 20)
//Triggers the Goto Line feature, showing a dialog box to enter a line number, wParam 1 shows it, 0 hides it
#define UM_SHOWGOTOLINE (WM_USER + 21)
//Triggers the Search Replace doalog, showing a dialog box to enter text, wParam 1 shows it, wparam 0 is search, 1 is replace, lparam 0 is hide, 1 is show
#define UM_SHOWSEARCHREPLACE (WM_USER + 22)


//This sets the font size in pixel (determine as height in pixels) or in point (pt size) only a certain set of values are allowed. wparam 0 means use height, 1 means uses pt. lparam is the size. 
//pt sizes 1 through 10, 10 through 30 step 2, 30 through 74, step 4, 74 through
//use this site http://pttopx.com/ to find the matching sizes in pixel. the pixel size is rounded to the nearest integer value
//if passing a value that does not exist in the reference table, the nearest size is selected, the function returns the size it chose. this creates a font internaly managed
//there is no guaranties the font size set and returned will match what is actually being used to draw the text as the font may not be drawble at the desired size. 
#define UM_SETFONTSIZE (WM_USER + 23)
//This gets the font size currently set. specify wParam to 0 to get size in pixel(height), 1 for pt.
//Set GetLastFontSize() which returns the last size used, globally, without the need to query an existing code box handle
#define UM_GETFONTSIZE (WM_USER + 24)
//This zooms the font in, wparam 1 to next larger font, and out, wparam -1 to next smaller font. returns the new size, set lparam to 1 to get the size in pt value, 0 for height
#define UM_ZOOMFONT (WM_USER + 25)


//This sets the scroll and related autoscroll, set wparam to 0 for vscroll, 1 for hscroll, lparam 1 for ON, 0 for off, setting the hscroll overrides the WORDWRAP option
//For this function to work properly the edit box must have been created with ES_AUTOVSCROLL or ES_AUTOHSCROLL respectively for each scroll bar WS_VSCROLL and WS_HSCROLL, else the scroll bar will reappear
#define UM_SETSCROLLBAR (WM_USER + 26)
//This get if the scrollbar and autoscrool for the h/vscroll are set (properly) for the feature to be ON. 0 for vscroll, 1 for hscroll. 
#define UM_GETSCROLLBAR (WM_USER + 27)

//Sets the wordwrap on or off, the value is tied to autohscroll, and the hscroll bar. 
//***NOTE***
//This message does not work,  it is tied to ES_AUTOHSCROLL wich changing at run time does not affect the behavior established on creation
//Create your window without the ES_AUTOHSCROLL style if you want word wrapping, but this cannot be changed at runtime
#define UM_SETWORDWRAP (WM_USER + 28)
//Check if word wrap is on (if autohscroll is off)
//***NOTE***
//The message will return on or off after you called UM_SETWORDWRAP, which does not work, the style is set but the feature is not enabled in the edit box
#define UM_GETWORDWRAP (WM_USER + 29)

//This indents the entiore code in the styles mentionned here
// https://en.wikipedia.org/wiki/Indentation_style
//will indent and beatify the code by adding indent and spacing. the feature is only compatible is c/c++/javascript/java and other curly braced baced languages
//NOTE: bracketless conditionals are not supported
#define UM_PERFORMAUTOINDENT (WM_USER + 30)
#define UMS_KR 0
#define UMS_ALLMAN 1
#define UMS_GNU 2
#define UMS_WHITESMITHS 3
#define UMS_HORSTMANN 4
#define UMS_PICO 5
#define UMS_RATLIFF 6
#define UMS_LISP 7

/*
Hot Keys
left right up down, move caret, +ctrl scroll as well, +shift select as well
page up page down, VScroll Up Down, +ctrl, HScroll Left Right
ctrl+ x, c, v cut copy paste
ctr+ins, shift+ins, copy paste
shift+del, cut
ctrl+a Select all
esc Select None, dismiss dialog boxes
insert, toggle overwrite mode on and off
ctrl+ z, y undo, redo (undoes undo)
tab, indent, multiline suport, +shift, unindent
ctrl+k Comment, +shift uncomment. Multiline support
F3 Search current selected text in rest of code. +shift backwards, if the search box is not opened, it is opened and the search field is set to the selected text, if opened, no change is made to the search field and the field is used
ctrl+ ], find matching bracket, +shift select. Works on () [] {} <> '' "", caret can be on either side of bracket
ctrl+ g Open Goto Line Interface
ctrl+ f Open Search Interface, also copies the selected text to the search box
ctrl+ r Open replace Interface, also copies the selected text to the search box
shift+ mousewheel, scroll horizontal
ctrl+ mousewheel, zoom in out ****NOTE The quality of the zoom depends on the quality specs of the font initialy set to the box, see CreateFont win api and WM_SETFONT. Some wheel (single) ticks may not seem to change the font due to the font size mapping system used and the font size reference table used vs the sizes available. this is not really noticeable as one usually scrolls more than one tic at a time

If you wish to have a Main UI Menu that Mirrors the Hot Keys that dont have a speciffic
UM_ message alternative, use the WM_KEYCODE message and pass the keycode as wParam
*/
/* TO TEST
UM_STYLIZE
UM_GETLINENUMBER
UM_GETCOLINDEX
UM_SETHILIGHT
UM_GETHILIGHT
UM_GOTOLINECOL
UM_GETCODELINECOUNT
UM_SHOWGOTOLINE
UM_SETWORDWRAP //Added, Tested, Fail. the style change at run time does not change the behaviour established at creation time, like for UM_SETNOHIDESEL, but unlike UM_SETNOHIDESEL I can't do anything about it since the character position, formating, scrolling, word wrapping, character from mouse position, etc, is still done by the edit box
UM_GETWORDWRAP //Added, Tested, works but value is meaningless because UM_SETWORDWRAP does nothing.
UM_SETSCROLLBAR //Added, tested. For this function to work properly the edit box must have been created with ES_AUTOVSCROLL or ES_AUTOHSCROLL respectively for each scroll bar WS_VSCROLL and WS_HSCROLL, else the scroll bar will reappear
UM_GETSCROLLBAR //Added, Tested
UM_SHOWSEARCHREPLACE //Added, Tested
UM_SETFONTSIZE //Added, Tested
UM_GETFONTSIZE //Added, Tested
UM_ZOOMFONT //Added, Tested
UM_SETINSERTMODE //Added, Tested
UM_GETINSERTMODE //Added, Tested
UM_SETNOHIDESEL //Add, Tested, weirdism: the edit box reads the ES_NOHIDESEL only when it starts, say it was OFF and you change the setting while hooked to ON, when you unhook the edit box, it will still behave as though the setting is OFF even though the setting is still ON in the styles.
UM_GETNOHIDESEL //Added, Tested
UM_MATCHBRACKET //Added, Tested
UM_CANREDO //Added, Tested
UM_BLOCKINDENT //Added, Tested
UM_BLOCKCOMMENT //Added, Tested
UM_CHARFROMPOS64k //Added, Tested, used internally
UM_GETLINEMARGIN //Added, Tested
UM_SETLINEMARGIN //Added, Tested
UM_CANSEARCHSEL //Added, Tested
UM_SEARCHSEL //Added, Tested
UM_CANREDO //Added, Tested. Related Msgs WM_UNDO, EM_UNDO, EM_CANUNDO, UM_REDO, UM_CANREDO
UM_REDO //Added, Tested. Related Msgs WM_UNDO, EM_UNDO, EM_CANUNDO, UM_REDO, UM_CANREDO

//Edit Box Messages, many are modified to simply make sure the redraw does not flicker
//Some are modified to track settings
//Some are modified to change behaviour slightly
//Some are total rewrite with not percievable changes on the user's end
EM_SETLIMITTEXT //Original, no change, default is 32k, calling with wParam 0 sets the maximum size. You must call this for more. Tested To 1.3 Million lines, 42000000 characters, ~80Mb (TCHARS are 2 bytes) with the Undos holding a few thousand pastes of about 200k each, resulting in about 140 MB app total. I broke the 64 K limit, then the 64k lines limit. I Don't know how far it'll go, assume max value of signed LONG as the character position is signed as, internally, some functions return -1 on fail for the character position.
EM_LIMITTEXT //Tested, Same value as EM_SETLIMITTEXT, see above
EM_CHARFROMPOS //Tested, original, no change
EM_EMPTYUNDOBUFFER //Modified, Tested, will clear the undo redo stacks
EM_FMTLINES //Modified, the feature is cancelled, the highlight cannot deal with soft carriage returns. On hook, soft cr mode it disabled, the feature is off by default, be aware if you use soft cr
EM_GETFIRSTVISIBLELINE //Tested, original, no change
EM_GETLIMITTEXT //Tested, original, no change
EM_GETLINE //Tested, original, no change
EM_GETLINECOUNT //Tested, original, no change. See UM_GETCODELINECOUNT for word wrap enabled code text box
EM_GETMARGINS //Tested, original, no change. see notes on EM_SETMARGINS
EM_GETMODIFY //Tested, original, no change
EM_GETSEL //Tested, original, no change *Use the wParam lParam passing a LONG* for text box with limit > 64K as opposed the LOWORD HWORD on the return value
EM_GETTHUMB //Tested, original, no change, well it returns 0 all the time no matter what on hooked and unhooked edit box... so pass?
EM_LINEFROMCHAR //Tested, original, no change
EM_LINEINDEX //Tested, original, no change
EM_LINELENGTH //Tested, original, no change
EM_LINESCROLL //Modified, simply suplements proper redraw
EM_POSFROMCHAR //Tested, original, no change
EM_REPLACESEL //Modified to record undoable action, Tested, passing 0 in wParam - Action cannot be undone, with actually destroy the entire undo redo stack, as it kills the single undo in the regular edit box
EM_SCROLL //Modified, Tested, simply suplements proper redraw
EM_SCROLLCARET //Modified, overrides original completely, a little smoother and returns if a scrolling happened, Tested
EM_SETMARGINS //Original, not change THOUGH: this has no effect when hooked as the code uses its own margins for line numbers, the margin is reset, in the draw event, technically the message went through but the setting is later nuked. if set before hooking, the setting is restored on unhook
EM_SETMODIFY //Tested, No change
EM_SETREADONLY //Added, Tested, the difference is only visual, not active line instead of that ugly grayed out box
EM_SETSEL //Modified to track selection and smoother refresh, Tested all options and -1s, passed
EM_CANUNDO //Modified to use the undo stack count for reference. Related Msgs WM_UNDO, EM_UNDO, EM_CANUNDO, UM_REDO, UM_CANREDO
EM_UNDO //Modified to use the redo stack. Related Msgs WM_UNDO, EM_UNDO, EM_CANUNDO, UM_REDO, UM_CANREDO

//Window Messages
WM_SETFONT //Modified, to recalculate the kerning and to prevent flicker and to update restore data, tested. Kerning support makes it so non monospace fonts work :) a bitch to find how that kerning winapi function worked...
// Note, even if lParam is 0, meaning LOWRD(lParam) is not 1, which, according to the documentation, 0 should be no redrawing, when hooked, the feature does cause a redraw, no intentionally, but redraws nevertheless, possibly due to the test harness code.
//     In any case, I dont see the use of changing the font without telling it to refresh, especially if the window is visible because that would look buggy.
// recomendation is to always pass 1 for lParam when hooked, 0 when changing the font before hooking, usually that happens when the box is not yet visible so really it matters little
// in any case, lparam 1 redraws the window so make sure to set it if changing the font when the edit box is visible
// WARNING if you created and set your own font to the text box, you must DeleteObject your original font, unless you are holding on to it; I recomend you dont.
// The code box manages it's own created font set internaly, typically with the ctrl+mousewheel, such font may currently be set in the box.
// DO NOT BLINDLY use WM_GETFONT to find the "Old Font" thinking it's yours to delete, it may be the font set by the ctrl+mousewheel.
// Any font you set will become the font used to reset the edit box state on unhook. If your brain is about to tilt, follow the logic:
// Font Management Philosophy, for every code box instance
// Before Hooking
//    MyFont = Create Font() (recomended since you want a monospace font)
//    Set Edit Box Font to MyFont, lparam set to NoRedraw, if you had got the font prior you would have got 0, the system font
//    Hook the Text Box
//To Change Font:
//    Send WM_GETFONT to get CurrentFont
//    create a NewFont
//    Send WM_SETFONT to set Newfont, lparam set to redraw
//    if(CurrentFont is NOT 0 meaning the system font, and CurrentFont is MyFont), DeleteObject(MyFont), MyFont = NewFont;
// After Unhook and destruction of the textbox (or dilaog box or window managing it), or just after it's destruction, the edit box died holding MyFont. It's no longer in use. there is/was no need to re-add the system font in the box
//    delete MyFont
// When you change the font when hooked, that will be the font used to restore the text box on unhook, it is still yours to manage

WM_SETTEXT //Modified for nuking undo stack, Tested. As original does, this resets the undo buffer, meaning the undo redo stack, and clears the modified flag
WM_GETTEXT //Original, no change, Tested
WM_CLEAR //Modified to record undoable action, Teted
WM_COPY //Original, no change, Tested
WM_CUT //Modified to record undoable action, Tested
WM_PASTE //Modified to record undoable action, Tested
WM_UNDO //Modified to do multiple level undo, Tested. Related Msgs WM_UNDO, EM_UNDO, EM_CANUNDO, UM_REDO, UM_CANREDO
SetWindowText(); //Tested (WM_SETTEXT)
GetWindowText(); //Tested (WM_GETTEXT)
EnableWindow(); //Tested (Does not gray the box like the edit box does)

TODO
Fix the colors referenced in the search dialog to be more specific. you got lines using text color, you got buttons using field color... what are field? textbox would be more apropriate
suggest to be as specific as possible,
TextFieldBackColor
TextFieldFontColor
DownArrow Edge, Normal, Hover, Down, Focused
DownArrow Face, Normal, Hover, Down, Focused
DownArrow Back, Normal, Hover, Down, Focused
Toolribbon, same, edge, face, back
And remove this silly color to brush nonesense for these... create the brushes on the fly and be done with it


Add GetLastFontHeight API
Add SetDefaultFont API will be used as default...
Check the USE of the KEYDOWN macro for ctrl and shift. it may not be right to use it as such, instead of looking at the extra data passed to the window proc
as the use may be tied to special keyboard entring sequences...
add parser, change colorref to use byte reference to color index. the memory grows now that the limit is broken so every byte counts

add set language message to get the current language for the control
add set colorspec message to get the current color specification from the control


sort the word list array maybe... but this can be pre sorted in the language definition file
add color to the word list text to have a per type color

add goto line dialiog

add code indenting
add auto-identing

fix the lag when closing a box with large text

cover the colorrise to use memset
add set changed region to the parser, called on changes made, it will start parsing from the spot the change was made, and set a largerst processed position
so that only stuff above than can be processed.
also a time can be used to split the parsing since the cursor refrses the box every half second... chunkify the colorising that way

check that business of using new for the wstringex when adding it to the list or words. does not make sense with other list using wstring seemd to works without creating new items

check dragging cursor on a line wider than box width, should autohscroll but not working. need to check against the settings if hscroll on

todo: sort every word lists so items appear in order in the picker list

*/
//bug: color not updating sometimes
//type in a empty box:
/* s
s
*/

//load the dll module and setup the function pointers for late binding calls
void TMC_SEB_LB_InitSyntaxModule();
//setup hilighting for an edit box, telling it which language to use. It is recomended, whenever possible, to call this api initially when the edit box is still invisible, in the form/dialog creation event so that the user does not see it change
void TMC_SEB_LB_SetHilite(HWND hWnd, const TCHAR* LanguageName, const TCHAR* ColorSpecName);
void TMC_SEB_LB_SetHiliteA(HWND hWnd, LPCSTR LanguageName, LPCSTR ColorSpecName);
//restores the edit box to plain jane
void TMC_SEB_LB_RemoveHilite(HWND hWnd);
//frees all hooks and languages, call at the end of the program (optional since windows takes care of this) or before you unload the dll
void TMC_SEB_LB_CleanUp();

//get the selected language so you can save to an ini if you want or use in subsequent TMC_SEB_SetHilite calls
//the return is the poiter to the global language name string in the dll which you can only read from. you can copy it to your own string buffer. the pointer will be valid as long as the dll is in memory
const TCHAR * TMC_SEB_LB_GetSelectedLanguage();
LPCSTR TMC_SEB_LB_GetSelectedLanguageA();
//get the selected color spec so you can save to an ini if you want or use in subsequent TMC_SEB_SetHilite calls
//the return is the poiter to the global colorspec name string in the dll which you can only read from. you can copy it to your own string buffer. the pointer will be valid as long as the dll is in memory
const TCHAR * TMC_SEB_LB_GetSelectedColorSpec();
LPCSTR TMC_SEB_LB_GetSelectedColorSpecA();
//Load a language file, and assign it a language name for use with SetHilite
void TMC_SEB_LB_LoadLanguage(const TCHAR* LanguageName, const TCHAR* LanguageFile);
void TMC_SEB_LB_LoadLanguageA(LPCSTR LanguageName, LPCSTR LanguageFile);
//Load a color spec file, and assign it a color spec name for use with SetHilite
void TMC_SEB_LB_LoadColorSpec(const TCHAR* ColorSpecName, const TCHAR* ColorSpecFile);
void TMC_SEB_LB_LoadColorSpecA(LPCSTR ColorSpecName, LPCSTR ColorSpecFile);
//Quit Load all language files under the SyntaxEBLangFiles folder loacated in the dll directory, for you lazy guys. each filename sans path and sans ext is used as the language name
void TMC_SEB_LB_LoadAllLanguageFiles();
//Quit Load all colorspec files under the SyntaxEBLangFiles folder loacated in the dll directory, for you lazy guys. each filename sans path and sans ext is used as the color spec name
void TMC_SEB_LB_LoadAllColorSpecFiles();
//Quit Load all language and colorspec files under the SyntaxEBLangFiles folder loacated in the dll directory, for you lazy guys. each filename sans path and sans ext is used as the language name and the color spec name 
void TMC_SEB_LB_LoadAllLanguageAndColorSpecFiles();
