#include "stdafx.h"
#include "SearchDialog.h"
#include <Commctrl.h>
#include <algorithm>
/* 
This is insanity!!!
I will attempt to explain the reasoning behind all this convolution...


First creating the dialg box is done with CreateDialog, passing the edit box hwnd as parent. the styles in the reousurce edit box allow it to be set as child and has no border
Second, I need to update the window position when I resize the edit box. this is done in the edit box HooedCtrl
HookedCtrl creates the dialog box uppon it being Hooked, invisibly, so it's always there. and Kill()s is when it gets unhooked. it will call UpdatePosition every time it matters
closing and showing it is simply done with hide and show function.

To solve the flickering problem, I added the CS_CLIPCHILDREN style to the edit box and I set the dialog to also clip children. uppon moving the dialog box, when the edit box is resized for example
The window is refreshed immediatly and so are the children

As for the controls on the form they are owner drawn, except the text box. 
Because the DialogProc is global, but the dialog box is an window instance created by SearchDilaog box class instance. I need to re-tie those 2 together somehow
So the dialog box window can use the data stored in it's SearchDialog instance...
I added a user message to the HookedCtrl which returns the Pointer to the HookedCtrl that owns the Dialog Box. 
From there I have access to the edit box data, and I have access to the instance holding the content/data of this dialog box. 
meaning I can EditCtrl->Something() or EditCtrl->m_TheDialogBox.Something()

You can see how I map the owner drawn control data to the editbox.m_MyDialog in the code

As for the drawing and the handling of the clicking of the button. it's all done in the wm_command and the ctldraw notifications. It's long and tedious and poorly/repetively written...

Final concern is the keyboard handling while the control on the form are active. because of the way the dialog box is operating, the tab for example would not work to tab next and would caul the edit box to beep.
So what I had to do is override the mainwindow proc of the controls to sneak at the keyboard and other messages... wan then I would trigger the right wm_command to perform the disired action in the dialog box code.

That's it... All this to make it work in the edit box results in an insane amount of code that does crazy things!

addn:
Adding a drop down list, I decided to make my own because the default one was causing me greif... the drop down list is in another window, a sigleton in charcge
of showing a list of words and informing the parent of the control attached to it (say a edit box) what the user selected. communication is in the form of user messages, basically a message
for when the user selected a word and another if he canceled the list box. the dialig box proc recieving the message and acts accordingly, in this case updating either
the earchtext or the replace text box content.

addn: hot keys like f3 and so on seen in the main code box are mirrored in each individial controls to allow the user to use F3, for example, while the search box has input focus
*/
/*
TODO

*/

extern HINSTANCE ghDllHandle;

wstring GetWindowWStr(HWND hWnd)
{
	wstring ret = L"";
	const int len = GetWindowTextLength(hWnd) + 1;
	if (len == 0) return ret;

	HANDLE hstr = LocalAlloc(LPTR, len * sizeof(TCHAR));
	TCHAR *Text = (TCHAR*)LocalLock(hstr);
	GetWindowText(hWnd, Text, len);
	ret = Text;
	LocalUnlock(hstr);
	LocalFree(hstr);
	return ret;
}
//Globally shared search replace word lists
WORDLIST gSearchWordList;
WORDLIST gReplaceWordList;

typedef struct pointf{
	float x;
	float y;
} POINTF;
//templates for shapes, in uv form 0,0, where 0,0 is center and upper left is -1,-1 and 1,1 is lower right. it makes it easier to make shaped and aslo mirror them if needed
const int DownArrowTemplateSize = 3;
const POINTF DownArrowTemplate[] = { {-.5f, -.5f}, { .5f,-.5f }, {0,.5f} };

const int HLineTemplateSize = 4;
const POINTF HLineTemplate[] = { { -.6f, -.6f },{ .6f,-.6f },{ .6f,-.55f }, { -.6f,-.55f } };

const int VLineTemplateSize = 4;
const POINTF VLineTemplate[] = { { -.6f, -.6f },{ -.54f,-.6f },{ -.54f, 0.6f },{ -.6f,.6f } };

const int SearchNextTemplateSize = 7;
const POINTF SearchNextTemplate[] = { { .6f, .0f }, { .0f,.5f }, { .0f,.25f }, { -.6f,.25f }, { -.6f,-.25f }, { .0f, -.25f }, { .0f,-.51f } };

const int AbTemplateSize = 17;
const POINTF AbTemplate[] = { { -.6f, .6f },{ -.3f,-.6f },{ .0f,.6f },
{ .0f,-.6f },{ .2f,-.6f },{ .2f,.1f },
{ .3f,-.1f },
{ .5f,-.1f },
{ .6f,.1f },
{ .6f,.4f },
{ .5f, .6f },
{ .3f, .6f },
{ .2f, .4f },
{ .2f, .6f },
{ -.2f, .6f },
{ -.3f, .2f },
{ -.4f, .6f },
};

const int ReplaceNextTemplateSize = 10;
const POINTF ReplaceNextTemplate[] = { { .6f, .0f },{ .0f,.5f },{ .0f,.25f },
{ -.3f,.25f },
{ -.5f,-.5f },
{ -.6f,0.0f },
{ -.5f,.5f },
{ -.3f,-.25f },
{ .0f, -.25f },{ .0f,-.51f } };

const int ReplaceAllTemplateSize = 12;
const POINTF ReplaceAllTemplate[] = { { .6f, .0f },{ .0f,.5f },{ .0f,.25f },
{ -.15f,.25f },

{ -.3f,-.25f },
{ -.5f,.5f },

{ -.6f,0.0f },

{ -.5f,-.5f },
{ -.3f,.25f },

{ -.15f,-.25f },
{ .0f, -.25f },
{ .0f,-.51f } };

const int ReplaceTemplateSize = 18;
const POINTF ReplaceTemplate[] = { { -.6f, -.25f },{-.4f,-.25f},{ -.4f,-.5f }, 

{ -.1f, 0.0f },

{ .0f,.5f },

{ .1f, 0.0f },


{ .4f,-.5f },
{ .4f,-.25f },
{ .6f, -.25f },
{ .6f, .25f },
{ .4f,.25f },
{ .4f,.5f },

{ .1f, 0.0f },

{ .0f,-.5f },

{ -.1f, 0.0f },


{ -.4f,.5f },
{ -.4f,.25f },
{ -.6f, .25f }
};

const int CloseXTemplateSize = 12;
const POINTF CloseXTemplate[] = { { .0f, -.15f }, { .41f,-.5f }, { .75f,-.5f }, { .15f,.0f },
                                                               { .75f,.5f }, 
                                                 { .4f, .5f },
                                  { .0f, .15f },
                                                 { -.4f, .5f },
                                                               { -.75f,.5f },
                                                                           { -.15f,.0f },
                                                               { -.75f,-.5f },
                                                  { -.4f,-.5f },
};


void TemplateToPoints(const POINTF Template[], POINT Points[], const int NumPoints, const RECT ClientRect, const float xscale = 1.0, const float yscale = 1.0)
{
	float cw = (float)ClientRect.right;
	float ch = (float)ClientRect.bottom;
	for (int i = 0; i < NumPoints; i++)
	{
		Points[i].x = (LONG) ((Template[i].x * xscale + 1.0) * 0.5 * cw);
		Points[i].y = (LONG) ((Template[i].y * yscale + 1.0) * 0.5 * ch);
	}
}



//This function looks if the mouse is over a button and does the hover code... I definitly could have streemline all that code, that is true throughout this file
void SetButtonsState(HWND hDlg)
{
	if (IsWindowVisible(hDlg))
	{
		POINT pt; GetCursorPos(&pt);
		//what control are we hovering
		HWND hCtrl = WindowFromPoint(pt);
		HWND FoundhCtrl = NULL;
		HookedCtrl *EBCtrl = (HookedCtrl *)SendMessage(GetParent(hDlg), UM_INTERNAL_GETEBHOOK, 0, 0);
		HWND hOther;
		int *OtherState;
		if (EBCtrl->m_SearchDlg.m_HasAnHover)
		{
			//Un-hover all controls, invalidate if they are set to hover AND they are not this control
			OtherState = &EBCtrl->m_SearchDlg.m_SearchDropDownBut.MouseState;
			hOther = GetDlgItem(hDlg, IDC_SEARCH_DROPDOWN_BUTTON);
			if (*OtherState && hOther != hCtrl)
			{
				*OtherState = 0;
				RedrawWindow(hOther, NULL, NULL, RDW_INVALIDATE);
			}

			OtherState = &EBCtrl->m_SearchDlg.m_SearchLeftBut.MouseState;
			hOther = GetDlgItem(hDlg, IDC_SEARCH_LEFT_BUTTON);
			if (*OtherState && hOther != hCtrl)
			{
				*OtherState = 0;
				RedrawWindow(hOther, NULL, NULL, RDW_INVALIDATE);
			}

			OtherState = &EBCtrl->m_SearchDlg.m_SearchRightBut.MouseState;
			hOther = GetDlgItem(hDlg, IDC_SEARCH_RIGHT_BUTTON);
			if (*OtherState && hOther != hCtrl)
			{
				*OtherState = 0;
				RedrawWindow(hOther, NULL, NULL, RDW_INVALIDATE);
			}

			OtherState = &EBCtrl->m_SearchDlg.m_SearchCaseBut.MouseState;
			hOther = GetDlgItem(hDlg, IDC_SEARCH_CASE_BUTTON);
			if (*OtherState && hOther != hCtrl)
			{
				*OtherState = 0;
				RedrawWindow(hOther, NULL, NULL, RDW_INVALIDATE);
			}

			OtherState = &EBCtrl->m_SearchDlg.m_SearchWordBut.MouseState;
			hOther = GetDlgItem(hDlg, IDC_SEARCH_WORD_BUTTON);
			if (*OtherState && hOther != hCtrl)
			{
				*OtherState = 0;
				RedrawWindow(hOther, NULL, NULL, RDW_INVALIDATE);
			}

			OtherState = &EBCtrl->m_SearchDlg.m_CloseSearchBut.MouseState;
			hOther = GetDlgItem(hDlg, IDC_CLOSE_SEARCH_BUTTON);
			if (*OtherState && hOther != hCtrl)
			{
				*OtherState = 0;
				RedrawWindow(hOther, NULL, NULL, RDW_INVALIDATE);
			}

			OtherState = &EBCtrl->m_SearchDlg.m_ReplaceDropDownBut.MouseState;
			hOther = GetDlgItem(hDlg, IDC_REPLACE_DROPDOWN_BUTTON);
			if (*OtherState && hOther != hCtrl)
			{
				*OtherState = 0;
				RedrawWindow(hOther, NULL, NULL, RDW_INVALIDATE);
			}
			
			OtherState = &EBCtrl->m_SearchDlg.m_ReplaceLeftBut.MouseState;
			hOther = GetDlgItem(hDlg, IDC_REPLACE_LEFT_BUTTON);
			if (*OtherState && hOther != hCtrl)
			{
				*OtherState = 0;
				RedrawWindow(hOther, NULL, NULL, RDW_INVALIDATE);
			}

			OtherState = &EBCtrl->m_SearchDlg.m_ReplaceRightBut.MouseState;
			hOther = GetDlgItem(hDlg, IDC_REPLACE_RIGHT_BUTTON);
			if (*OtherState && hOther != hCtrl)
			{
				*OtherState = 0;
				RedrawWindow(hOther, NULL, NULL, RDW_INVALIDATE);
			}

			OtherState = &EBCtrl->m_SearchDlg.m_ReplaceAllBut.MouseState;
			hOther = GetDlgItem(hDlg, IDC_REPLACE_ALL_BUTTON);
			if (*OtherState && hOther != hCtrl)
			{
				*OtherState = 0;
				RedrawWindow(hOther, NULL, NULL, RDW_INVALIDATE);
			}
			
			OtherState = &EBCtrl->m_SearchDlg.m_ReplaceCurrentBut.MouseState;
			hOther = GetDlgItem(hDlg, IDC_REPLACE_CURRENT_BUTTON);
			if (*OtherState && hOther != hCtrl)
			{
				*OtherState = 0;
				RedrawWindow(hOther, NULL, NULL, RDW_INVALIDATE);
			}

			EBCtrl->m_SearchDlg.m_HasAnHover = NULL;
		}
		//if the control is on this dialog box
		if (GetParent(hCtrl) == hDlg)
		{
			//the the resource id
			LONG ControlID = GetDlgCtrlID(hCtrl);
			int MouseDown = (KEY_STATE_DOWN(VK_LBUTTON) != 0);
			//set the control to hover state
			switch (ControlID)
			{
			case IDC_SEARCH_DROPDOWN_BUTTON:
			{
				int oldstate = EBCtrl->m_SearchDlg.m_SearchDropDownBut.MouseState;
				int newstate = 1;
				EBCtrl->m_SearchDlg.m_SearchDropDownBut.MouseState = 1;
				RedrawWindow(GetDlgItem(hDlg, IDC_SEARCH_DROPDOWNBUTTON), NULL, NULL, RDW_INVALIDATE);
				EBCtrl->m_SearchDlg.m_HasAnHover = hCtrl;
				if(oldstate != newstate) RedrawWindow(hCtrl, NULL, NULL, RDW_INVALIDATE);
				break;
			}
			case IDC_SEARCH_LEFT_BUTTON:
			{
				int oldstate = EBCtrl->m_SearchDlg.m_SearchLeftBut.MouseState;
				int newstate = 1;
				EBCtrl->m_SearchDlg.m_SearchLeftBut.MouseState = 1;
				RedrawWindow(GetDlgItem(hDlg, IDC_SEARCH_LEFT_BUTTON), NULL, NULL, RDW_INVALIDATE);
				EBCtrl->m_SearchDlg.m_HasAnHover = hCtrl;
				if (oldstate != newstate) RedrawWindow(hCtrl, NULL, NULL, RDW_INVALIDATE);
				break;
			}
			case IDC_SEARCH_RIGHT_BUTTON:
			{
				int oldstate = EBCtrl->m_SearchDlg.m_SearchRightBut.MouseState;
				int newstate = 1;
				EBCtrl->m_SearchDlg.m_SearchRightBut.MouseState = 1;
				RedrawWindow(GetDlgItem(hDlg, IDC_SEARCH_RIGHT_BUTTON), NULL, NULL, RDW_INVALIDATE);
				EBCtrl->m_SearchDlg.m_HasAnHover = hCtrl;
				if (oldstate != newstate) RedrawWindow(hCtrl, NULL, NULL, RDW_INVALIDATE);
				break;
			}
			case IDC_SEARCH_CASE_BUTTON:
			{
				int oldstate = EBCtrl->m_SearchDlg.m_SearchCaseBut.MouseState;
				int newstate = 1; 
				EBCtrl->m_SearchDlg.m_SearchCaseBut.MouseState = 1;
				RedrawWindow(GetDlgItem(hDlg, IDC_SEARCH_CASE_BUTTON), NULL, NULL, RDW_INVALIDATE);
				EBCtrl->m_SearchDlg.m_HasAnHover = hCtrl;
				if (oldstate != newstate) RedrawWindow(hCtrl, NULL, NULL, RDW_INVALIDATE);
				break;
			}
			case IDC_SEARCH_WORD_BUTTON:
			{
				int oldstate = EBCtrl->m_SearchDlg.m_SearchWordBut.MouseState;
				int newstate = 1;
				EBCtrl->m_SearchDlg.m_SearchWordBut.MouseState = 1;
				RedrawWindow(GetDlgItem(hDlg, IDC_SEARCH_WORD_BUTTON), NULL, NULL, RDW_INVALIDATE);
				EBCtrl->m_SearchDlg.m_HasAnHover = hCtrl;
				if (oldstate != newstate) RedrawWindow(hCtrl, NULL, NULL, RDW_INVALIDATE);
				break;
			}
			case IDC_CLOSE_SEARCH_BUTTON:
			{
				int oldstate = EBCtrl->m_SearchDlg.m_CloseSearchBut.MouseState;
				int newstate = 1; 
				EBCtrl->m_SearchDlg.m_CloseSearchBut.MouseState = 1;
				RedrawWindow(GetDlgItem(hDlg, IDC_CLOSE_SEARCH_BUTTON), NULL, NULL, RDW_INVALIDATE);
				EBCtrl->m_SearchDlg.m_HasAnHover = hCtrl;
				if (oldstate != newstate) RedrawWindow(hCtrl, NULL, NULL, RDW_INVALIDATE);
				break;
			}

			case IDC_REPLACE_DROPDOWN_BUTTON:
			{
				int oldstate = EBCtrl->m_SearchDlg.m_ReplaceDropDownBut.MouseState;
				int newstate = 1;
				EBCtrl->m_SearchDlg.m_ReplaceDropDownBut.MouseState = 1;
				RedrawWindow(GetDlgItem(hDlg, IDC_REPLACE_DROPDOWN_BUTTON), NULL, NULL, RDW_INVALIDATE);
				EBCtrl->m_SearchDlg.m_HasAnHover = hCtrl;
				if (oldstate != newstate) RedrawWindow(hCtrl, NULL, NULL, RDW_INVALIDATE);
				break;
			}
			case IDC_REPLACE_LEFT_BUTTON:
			{
				int oldstate = EBCtrl->m_SearchDlg.m_ReplaceLeftBut.MouseState;
				int newstate = 1;
				EBCtrl->m_SearchDlg.m_ReplaceLeftBut.MouseState = 1;
				RedrawWindow(GetDlgItem(hDlg, IDC_REPLACE_LEFT_BUTTON), NULL, NULL, RDW_INVALIDATE);
				EBCtrl->m_SearchDlg.m_HasAnHover = hCtrl;
				if (oldstate != newstate) RedrawWindow(hCtrl, NULL, NULL, RDW_INVALIDATE);
				break;
			}
			case IDC_REPLACE_RIGHT_BUTTON:
			{
				int oldstate = EBCtrl->m_SearchDlg.m_ReplaceRightBut.MouseState;
				int newstate = 1;
				EBCtrl->m_SearchDlg.m_ReplaceRightBut.MouseState = 1;
				RedrawWindow(GetDlgItem(hDlg, IDC_REPLACE_RIGHT_BUTTON), NULL, NULL, RDW_INVALIDATE);
				EBCtrl->m_SearchDlg.m_HasAnHover = hCtrl;
				if (oldstate != newstate) RedrawWindow(hCtrl, NULL, NULL, RDW_INVALIDATE);
				break;
			}
			case IDC_REPLACE_ALL_BUTTON:
			{
				int oldstate = EBCtrl->m_SearchDlg.m_ReplaceAllBut.MouseState;
				int newstate = 1;
				EBCtrl->m_SearchDlg.m_ReplaceAllBut.MouseState = 1;
				RedrawWindow(GetDlgItem(hDlg, IDC_REPLACE_ALL_BUTTON), NULL, NULL, RDW_INVALIDATE);
				EBCtrl->m_SearchDlg.m_HasAnHover = hCtrl;
				if (oldstate != newstate) RedrawWindow(hCtrl, NULL, NULL, RDW_INVALIDATE);
				break;
			}
			case IDC_REPLACE_CURRENT_BUTTON:
			{
				int oldstate = EBCtrl->m_SearchDlg.m_ReplaceCurrentBut.MouseState;
				int newstate = 1;
				EBCtrl->m_SearchDlg.m_ReplaceCurrentBut.MouseState = 1;
				RedrawWindow(GetDlgItem(hDlg, IDC_REPLACE_CURRENT_BUTTON), NULL, NULL, RDW_INVALIDATE);
				EBCtrl->m_SearchDlg.m_HasAnHover = hCtrl;
				if (oldstate != newstate) RedrawWindow(hCtrl, NULL, NULL, RDW_INVALIDATE);
				break;
			}
			}
		}
	}
}


//this function tabs to the next/previous control. the code is naive. all the dialog box controls are tab stoppable
void TabToNext(HWND hWnd)
{
	int Shift = KEY_STATE_DOWN(VK_SHIFT);
	if (Shift)
	{
		HWND Next = GetNextWindow(hWnd, GW_HWNDPREV);
		if (Next == NULL)
		{
			Next = GetWindow(hWnd, GW_HWNDLAST);
		}
		SetFocus(Next);
	}
	else
	{
		HWND Next = GetNextWindow(hWnd, GW_HWNDNEXT);
		if (Next == NULL)
		{

			Next = GetWindow(hWnd, GW_HWNDFIRST);
		}
		SetFocus(Next);
	}
}
//The nasty overriding of the controls on this form. tankfuly I only have 2 control types
WNDPROC EBOrgiWindowProc = NULL;
WNDPROC BUTOrgiWindowProc = NULL;
#define VK_F 0x46
#define VK_R 0x52
//If tab or any of the expected hot key is hit while the control has cofus, trigger the action in the dialog box
LRESULT CALLBACK EBWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	
	switch (message)
	{
	
	case WM_ERASEBKGND:
		return true;
	case WM_SETFOCUS:
	{
		CallWindowProc(EBOrgiWindowProc, hWnd, message, wParam, lParam);
		SendMessage(hWnd, EM_SETSEL, 0, -1);
		return 0;
	}
	case WM_KILLFOCUS:
	{
		CallWindowProc(EBOrgiWindowProc, hWnd, message, wParam, lParam);
		SendMessage(hWnd, EM_SETSEL, -1, 0);
		return 0;
	}
	
	case WM_CHAR:
	{
		WPARAM key = wParam;
		switch (wParam)
		{
		case 6: //ctrl+f
		case 18: //ctrl+r
		//case VK_R: //handled above, this was supposed to catch ctrl+f/r, but ctrl changes the vk, see above
		//case VK_F:
		case VK_TAB:
		case VK_RETURN:
		case VK_ESCAPE:
		{
			return 0;
		}
		
		}
		break;
	}
	case WM_KEYDOWN:
	{
		bool DoReplace = false;
		switch (wParam)
		{
		case VK_RETURN:
		{
			wstring text = L"";
			WORDLIST *wl = NULL;
			if (GetDlgCtrlID(hWnd) == IDC_SEARCHTEXT)
			{
				text = GetWindowWStr(hWnd);
				wl = &gSearchWordList;
			}
			else if (GetDlgCtrlID(hWnd) == IDC_REPLACETEXT)
			{
				text = GetWindowWStr(hWnd);
				wl = &gReplaceWordList;
				DoReplace = true;
			}
			if (text != L"" && wl != NULL)
			{
				bool found = (std::find(wl->begin(), wl->end(), text) != wl->end());
				if (!found)
				{
					//add new text
					wstringex t;// = new wstringex();
					t.assign(text);
					wl->push_front(t);
				}
				else
				{
					//remove and re-add so it's topmost, ordering searched from most recent onward
					wstringex t;// = new wstringex();
					t.assign(text);
					wl->remove(t);
					wl->push_front(t);
				}
			}
			//return  0; fall through to the search
		}
		case VK_F3:
		{
			INT zeButtonID = IDC_SEARCH_LEFT_BUTTON;
			if (KEY_STATE_DOWN(VK_SHIFT))
			{
				zeButtonID = IDC_SEARCH_LEFT_BUTTON;
				if(DoReplace) zeButtonID = IDC_REPLACE_LEFT_BUTTON;
			}
			else
			{
				zeButtonID = IDC_SEARCH_RIGHT_BUTTON;
				if (DoReplace) zeButtonID = IDC_REPLACE_RIGHT_BUTTON;
			}
			PostMessage(GetParent(hWnd), WM_COMMAND,
				MAKELPARAM(zeButtonID, BN_CLICKED),
				(LPARAM)GetDlgItem(GetParent(hWnd), zeButtonID));
			return 0;
		}
		case VK_ESCAPE:
		{
			PostMessage(GetParent(hWnd), WM_COMMAND,
					MAKELPARAM(IDC_CLOSE_SEARCH_BUTTON, BN_CLICKED),
					(LPARAM)GetDlgItem(GetParent(hWnd), IDC_CLOSE_SEARCH_BUTTON)
				);
			return 0;
		}
		case VK_R:
		{
			if (KEY_STATE_DOWN(VK_CONTROL))
			{
				SetFocus(GetDlgItem(GetParent(hWnd), IDC_REPLACETEXT));
				return 0l;
			}
			break;
		}
		case VK_F:
		{
			if (KEY_STATE_DOWN(VK_CONTROL))
			{
				SetFocus(GetDlgItem(GetParent(hWnd), IDC_SEARCHTEXT));
				return 0l;
			}
			break;
		}
		case VK_TAB:
		{
			TabToNext(hWnd);
			return  0;
		}
		
		case VK_DOWN:
		{
			if (GetDlgCtrlID(hWnd) == IDC_SEARCHTEXT)
				PostMessage(GetParent(hWnd), WM_COMMAND,
					MAKELPARAM(IDC_SEARCH_DROPDOWN_BUTTON, BN_CLICKED),
					(LPARAM)GetDlgItem(GetParent(hWnd), IDC_SEARCH_DROPDOWN_BUTTON)
				);
			else if (GetDlgCtrlID(hWnd) == IDC_REPLACETEXT)
				PostMessage(GetParent(hWnd), WM_COMMAND,
					MAKELPARAM(IDC_REPLACE_DROPDOWN_BUTTON, BN_CLICKED),
					(LPARAM)GetDlgItem(GetParent(hWnd), IDC_REPLACE_DROPDOWN_BUTTON)
				);
			return  0;
		}
		}
	
	}
	
	}
	return CallWindowProc(EBOrgiWindowProc, hWnd, message, wParam, lParam);
}
//same as func above
LRESULT CALLBACK BUTWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_ERASEBKGND:
		return true;

	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_ESCAPE:
		{
			PostMessage(GetParent(hWnd), WM_COMMAND,
					MAKELPARAM(IDC_CLOSE_SEARCH_BUTTON, BN_CLICKED),
					(LPARAM)GetDlgItem(GetParent(hWnd), IDC_CLOSE_SEARCH_BUTTON)
				);
			return  0;
		}
		
		case VK_F3:
		{
			if (KEY_STATE_DOWN(VK_SHIFT))
			{
				PostMessage(GetParent(hWnd), WM_COMMAND,
					MAKELPARAM(IDC_SEARCH_LEFT_BUTTON, BN_CLICKED),
					(LPARAM)GetDlgItem(GetParent(hWnd), IDC_SEARCH_LEFT_BUTTON)
				);
			}
			else
			{
				PostMessage(GetParent(hWnd), WM_COMMAND,
					MAKELPARAM(IDC_SEARCH_RIGHT_BUTTON, BN_CLICKED),
					(LPARAM)GetDlgItem(GetParent(hWnd), IDC_SEARCH_RIGHT_BUTTON)
				);
			}
			return  0;
		}
		case VK_TAB:
		{
			TabToNext(hWnd);
			return  0;
		}
		case VK_RETURN:
		{
			return CallWindowProc(BUTOrgiWindowProc, hWnd, message, VK_SPACE, lParam);
		}
		case VK_R:
		{
			if (KEY_STATE_DOWN(VK_CONTROL))
			{
				SetFocus(GetDlgItem(GetParent(hWnd), IDC_REPLACETEXT));
				return 0l;
			}
			break;
		}
		case VK_F:
		{
			if (KEY_STATE_DOWN(VK_CONTROL))
			{
				SetFocus(GetDlgItem(GetParent(hWnd), IDC_SEARCHTEXT));
				return 0l;
			}
			break;
		}
		}
	}
	}
	return CallWindowProc(BUTOrgiWindowProc, hWnd, message, wParam, lParam);
}

BOOL CALLBACK SrEnumChildrenHasInvadidRect(HWND   hWnd, LPARAM lParam)
{
	int *InvalidCount = (int*)lParam;
	*InvalidCount += (int)GetUpdateRect(hWnd, 0, 0);
	//ShowWindow(hWnd, SW_SHOW);
	return true;
}


INT_PTR CALLBACK SearchDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
	{
		//ask the parent EB for the search dialog object
		HookedCtrl *EBCtrl = (HookedCtrl *)SendMessage(GetParent(hDlg), UM_INTERNAL_GETEBHOOK, 0, 0);
		EBCtrl->m_SearchDlg.m_CheckTimer = (int)SetTimer(hDlg, 6502, 5, 0);

		//Override all the controls so I can get the fraken enter, escape and tab to work
		//...the pain of bending windows to my will using a dialog box resource as the search box... well too late now

		//since the dialog procedure does not allow me to catch enter and escape before the edit box beeps at you
		// and the tab to go to the next control, that too dont work because the way I am using the dialog

		//so we set the window procedure of every control under the sun to ours so we can catch the keys.
		EBOrgiWindowProc = (WNDPROC) GetWindowLongPtr(GetDlgItem(hDlg, IDC_SEARCHTEXT), GWLP_WNDPROC);
		SetWindowLongPtr(GetDlgItem(hDlg, IDC_SEARCHTEXT), GWLP_WNDPROC, (LONG_PTR)EBWindowProc);
		SetWindowLongPtr(GetDlgItem(hDlg, IDC_REPLACETEXT), GWLP_WNDPROC, (LONG_PTR)EBWindowProc);

		//do the same for each instance of the button
		BUTOrgiWindowProc = (WNDPROC)GetWindowLongPtr(GetDlgItem(hDlg, IDC_SEARCH_DROPDOWN_BUTTON), GWLP_WNDPROC);
		SetWindowLongPtr(GetDlgItem(hDlg, IDC_SEARCH_DROPDOWN_BUTTON), GWLP_WNDPROC, (LONG_PTR)BUTWindowProc);

		SetWindowLongPtr(GetDlgItem(hDlg, IDC_SEARCH_LEFT_BUTTON), GWLP_WNDPROC, (LONG_PTR)BUTWindowProc);
		SetWindowLongPtr(GetDlgItem(hDlg, IDC_SEARCH_RIGHT_BUTTON), GWLP_WNDPROC, (LONG_PTR)BUTWindowProc);
		SetWindowLongPtr(GetDlgItem(hDlg, IDC_SEARCH_CASE_BUTTON), GWLP_WNDPROC, (LONG_PTR)BUTWindowProc);
		SetWindowLongPtr(GetDlgItem(hDlg, IDC_SEARCH_WORD_BUTTON), GWLP_WNDPROC, (LONG_PTR)BUTWindowProc);
		SetWindowLongPtr(GetDlgItem(hDlg, IDC_CLOSE_SEARCH_BUTTON), GWLP_WNDPROC, (LONG_PTR)BUTWindowProc);
		SetWindowLongPtr(GetDlgItem(hDlg, IDC_REPLACE_DROPDOWN_BUTTON), GWLP_WNDPROC, (LONG_PTR)BUTWindowProc);
		SetWindowLongPtr(GetDlgItem(hDlg, IDC_REPLACE_LEFT_BUTTON), GWLP_WNDPROC, (LONG_PTR)BUTWindowProc);
		SetWindowLongPtr(GetDlgItem(hDlg, IDC_REPLACE_RIGHT_BUTTON), GWLP_WNDPROC, (LONG_PTR)BUTWindowProc);
		SetWindowLongPtr(GetDlgItem(hDlg, IDC_REPLACE_ALL_BUTTON), GWLP_WNDPROC, (LONG_PTR)BUTWindowProc);
		SetWindowLongPtr(GetDlgItem(hDlg, IDC_REPLACE_CURRENT_BUTTON), GWLP_WNDPROC, (LONG_PTR)BUTWindowProc);
		
		
		return (INT_PTR)TRUE;
	}
	case UM_INTERNAL_GETEBHOOK:
	{
		//if the WordListDialog Boxes I created are trying to get the edit box point, pass on the message to the edit box
		return SendMessage(GetParent(hDlg), UM_INTERNAL_GETEBHOOK, 0, 0);
	}
	
	case UM_INTERNAL_DOSEARCH:
	{
		SetFocus((HWND)wParam);
		HookedCtrl *EBCtrl = (HookedCtrl *)SendMessage(GetParent(hDlg), UM_INTERNAL_GETEBHOOK, 0, 0);
		bool DoReplace = wParam;
		if (EBCtrl->m_SelEnd != EBCtrl->m_SelStart)
		{
			LONG SelLineStart = (LONG)SendMessage(GetParent(hDlg), EM_LINEFROMCHAR, EBCtrl->m_SelStart, 0);
			LONG SelLineEnd = (LONG)SendMessage(GetParent(hDlg), EM_LINEFROMCHAR, EBCtrl->m_SelEnd, 0);
			if (SelLineStart == SelLineEnd)
			{
				wstring t = EBCtrl->GetSelText();
				if (DoReplace)
				{
					SetWindowText(GetDlgItem(hDlg, IDC_REPLACETEXT), t.c_str());
					//SetFocus(GetDlgItem(hDlg, IDC_REPLACETEXT));
				}
				else
				{
					SetWindowText(GetDlgItem(hDlg, IDC_SEARCHTEXT), t.c_str());
					//SetFocus(GetDlgItem(hDlg, IDC_SEARCHTEXT));

				}
			}
		}
		if (DoReplace)
		{
			SetFocus(GetDlgItem(hDlg, IDC_REPLACETEXT));

		}
		else
		{

			SetFocus(GetDlgItem(hDlg, IDC_SEARCHTEXT));
		}
		break;
	}
	case UM_INTERNAL_FORCEREFRESH:
	{
		HookedCtrl *EBCtrl = (HookedCtrl *)SendMessage(GetParent(hDlg), UM_INTERNAL_GETEBHOOK, 0, 0);
		EBCtrl->m_SearchDlg.ForceRefresh();
	}
	
	case WM_SETCURSOR:
	{
		//This message triggers when the mouse hovers something on the form, so do the hover code
		SetButtonsState(hDlg);
		break;
	}
	case WM_TIMER:
	{
		//this time tics so we can do things over time. initially I wanted to use this to move the window in view. 
		
		HookedCtrl *EBCtrl = (HookedCtrl *)SendMessage(GetParent(hDlg), UM_INTERNAL_GETEBHOOK, 0, 0);

		return 0l;
	}
	//case WM_PAINT:
	//{
	//	HookedCtrl *EBCtrl = (HookedCtrl *)SendMessage(GetParent(hDlg), UM_GETEBHOOK, 0, 0);
	//	//EBCtrl->m_SearchDlg.m_GetSnapshot = true;
	//	return DefWindowProc(hDlg, message, wParam, lParam);
	//
	//}
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDOWN:
	{
		SetFocus(GetDlgItem(hDlg, IDC_SEARCHTEXT));
		break;
	}
	case UM_INTERNAL_CANCELED:
	{
		//list box tells the dialog box that it was canceled. passing the linked control idc
		int idcCtrl = (int)wParam;
		SetFocus(GetDlgItem(hDlg, idcCtrl)); //refocus on the text box
		return 0l;
	}
	case UM_INTERNAL_WORDSELECTED:
	{
		//list box tells the dialog box that a selection was made. passing the list handle and the linked control idc
		HWND ListhWnd = (HWND)lParam;
		int idcCtrl = (int)wParam;
		if (SendMessage(ListhWnd, LB_GETSELCOUNT, 0, 0))
		{
			LRESULT Sel = SendMessage(ListhWnd, LB_GETCURSEL, 0, 0);
			if (Sel != LB_ERR)
			{
				wstringex *Word = (wstringex *)SendMessage(ListhWnd, LB_GETITEMDATA, Sel, 0);
				if (Word->length())
				{
					SetWindowText(GetDlgItem(hDlg, idcCtrl), Word->c_str());
				}
			}
		}
		SetFocus(GetDlgItem(hDlg, idcCtrl)); //refocus on the text box
		
		return 0l;
	}
	case WM_COMMAND:
	{
		//control notification messages
		HookedCtrl *EBCtrl = (HookedCtrl *)SendMessage(GetParent(hDlg), UM_INTERNAL_GETEBHOOK, 0, 0);
		//EBCtrl->m_SearchDlg.m_GetSnapshot = true;
		//EBCtrl->m_SearchDlg.ScreenShot();
		WORD NotificationCode = HIWORD(wParam);
		WORD CtrlID = LOWORD(wParam);
		HWND hCtrl = (HWND)lParam;
		SetButtonsState(hDlg);
		
		if (NotificationCode == BN_CLICKED)
		{
			

			switch (CtrlID)
			{
			case IDC_SEARCH_DROPDOWN_BUTTON:
			{
				RECT r; GetWindowRect(GetDlgItem(hDlg, IDC_SEARCHTEXT), &r);
				MapWindowPoints(NULL, GetParent(hDlg), (POINT*)&r, 2);
				EBCtrl->m_WordListDlg.SetWordList(gSearchWordList);
				EBCtrl->m_WordListDlg.SetDispatch(hDlg, IDC_SEARCHTEXT);
				EBCtrl->m_WordListDlg.Show(SW_SHOW, (short) r.left, (short)r.bottom, (short)(r.right-r.left), (short)(r.bottom- r.top) * 8);
				wstring text = GetWindowWStr(GetDlgItem(hDlg, IDC_SEARCHTEXT));
				EBCtrl->m_WordListDlg.RefreshListContent(text.c_str(), 0);
				RedrawWindow((hDlg), NULL, NULL, RDW_INVALIDATE);
				RedrawWindow(GetParent(hDlg), NULL, NULL, RDW_INVALIDATE);
				return 0l;
			}
			case IDC_REPLACE_DROPDOWN_BUTTON:
			{
				RECT r; GetWindowRect(GetDlgItem(hDlg, IDC_REPLACETEXT), &r);
				MapWindowPoints(NULL, GetParent(hDlg), (POINT*)&r, 2);
				EBCtrl->m_WordListDlg.SetWordList(gReplaceWordList);
				EBCtrl->m_WordListDlg.SetDispatch(hDlg, IDC_REPLACETEXT);
				EBCtrl->m_WordListDlg.Show(SW_SHOW, (short)r.left, (short)r.bottom, (short)(r.right - r.left), (short)(r.bottom - r.top) * 8);
				wstring text = GetWindowWStr(GetDlgItem(hDlg, IDC_REPLACETEXT));
				EBCtrl->m_WordListDlg.RefreshListContent(text.c_str(), 0);
				RedrawWindow((hDlg), NULL, NULL, RDW_INVALIDATE);
				RedrawWindow(GetParent(hDlg), NULL, NULL, RDW_INVALIDATE);
				return 0l;
			}
			case IDC_SEARCH_LEFT_BUTTON:
			{
				//Do the search
				EBCtrl->SearchText(-1, EBCtrl->m_SearchDlg.m_SearchCaseBut.State, EBCtrl->m_SearchDlg.m_SearchWordBut.State, EBCtrl->m_SearchDlg.GetSearchText());
				return 0L;
			}
			case IDC_SEARCH_RIGHT_BUTTON:
			{
				//Do the search
				EBCtrl->SearchText(1, EBCtrl->m_SearchDlg.m_SearchCaseBut.State, EBCtrl->m_SearchDlg.m_SearchWordBut.State, EBCtrl->m_SearchDlg.GetSearchText());
				return 0L;
			}
			case IDC_REPLACE_LEFT_BUTTON:
			{
				//Do the replace
				wstring ReplaceText = EBCtrl->m_SearchDlg.GetReplaceText();
				wstring SearchText = EBCtrl->m_SearchDlg.GetSearchText();
				if (EBCtrl->SelectedTextIsSameAs(SearchText))
				{
					EBCtrl->AddUndoableContent(ReplaceText,true,true);
					
				}
				EBCtrl->SearchText(-1, EBCtrl->m_SearchDlg.m_SearchCaseBut.State, EBCtrl->m_SearchDlg.m_SearchWordBut.State, SearchText);
				return 0L;
			}
			case IDC_REPLACE_RIGHT_BUTTON:
			{
				//Do the replace
				wstring ReplaceText = EBCtrl->m_SearchDlg.GetReplaceText();
				wstring SearchText = EBCtrl->m_SearchDlg.GetSearchText();
				if (EBCtrl->SelectedTextIsSameAs(SearchText))
				{
					EBCtrl->AddUndoableContent(ReplaceText, true, true);
					EBCtrl->SearchText(1, EBCtrl->m_SearchDlg.m_SearchCaseBut.State, EBCtrl->m_SearchDlg.m_SearchWordBut.State, SearchText);
				}
				return 0L;
			}
			case IDC_REPLACE_CURRENT_BUTTON:
			{
				//Do the replace
				wstring ReplaceText = EBCtrl->m_SearchDlg.GetReplaceText();
				wstring SearchText = EBCtrl->m_SearchDlg.GetSearchText();
				if (EBCtrl->SelectedTextIsSameAs(SearchText))
				{
					EBCtrl->AddUndoableContent(ReplaceText, true, true);
				}
				return 0L;
			}
			case IDC_REPLACE_ALL_BUTTON:
			{
				//Do the replace
				//if (EBCtrl->m_SelStart != EBCtrl->m_SelEnd)
				{
					EBCtrl->RedrawStop();
					EBCtrl->m_ScrollCaretEnabled = false;
					LONG OldSelStart = EBCtrl->m_SelStart;
					LONG OldSelEnd = EBCtrl->m_SelEnd;
					EBCtrl->m_SelStart = 0;
					EBCtrl->m_SelEnd = 0;
					bool FoundSome = false;
					wstring ST = EBCtrl->m_SearchDlg.GetSearchText();
					wstring RT = EBCtrl->m_SearchDlg.GetReplaceText();
					while (EBCtrl->SearchText(1, EBCtrl->m_SearchDlg.m_SearchCaseBut.State, EBCtrl->m_SearchDlg.m_SearchWordBut.State, ST) != -1)
					{
						if (!EBCtrl->SelectedTextIsSameAs(ST))
							break;
						FoundSome = true;
						EBCtrl->AddUndoableContent(RT, true, true);
					}
					if (FoundSome)
					{

						SendMessage(EBCtrl->m_hWnd, EM_SETSEL, EBCtrl->m_SelStart - RT.length(), EBCtrl->m_SelEnd);
					}
					else
					{
						SendMessage(EBCtrl->m_hWnd, EM_SETSEL, OldSelStart, OldSelEnd);
					}
					EBCtrl->RedrawResume();
					EBCtrl->m_ScrollCaretEnabled = true;
					SendMessage(EBCtrl->m_hWnd, EM_SCROLLCARET, 0,0);
				}
				return 0L;
			}
			case IDC_CLOSE_SEARCH_BUTTON:
			{
				//Close the dialog, well hide it
				SetFocus(EBCtrl->m_hWnd);
				ShowWindow(hDlg, SW_HIDE);
				return 0l;
			}
			case IDC_SEARCH_CASE_BUTTON:
			{
				//Toggle down state
				EBCtrl->m_SearchDlg.m_SearchCaseBut.State = !EBCtrl->m_SearchDlg.m_SearchCaseBut.State;
				RedrawWindow(hCtrl, NULL, NULL, RDW_INVALIDATE);
				return 0L;
			}
			case IDC_SEARCH_WORD_BUTTON:
			{
				//Toggle down state
				EBCtrl->m_SearchDlg.m_SearchWordBut.State = !EBCtrl->m_SearchDlg.m_SearchWordBut.State;
				RedrawWindow(hCtrl, NULL, NULL, RDW_INVALIDATE);
				return 0L;
			}
			case IDOK:
			{
				//Hide the box
				ShowWindow(hDlg, SW_HIDE);
				return 0L;
			}
			}
		}

		break;
	}
	case WM_DESTROY:
	{
		//kil the time as cleanup
		HookedCtrl *EBCtrl = (HookedCtrl *)SendMessage(GetParent(hDlg), UM_INTERNAL_GETEBHOOK, 0, 0);
		KillTimer(hDlg, EBCtrl->m_SearchDlg.m_CheckTimer);
		break;
	}
	case WM_CTLCOLORDLG:
	{
		//Override the color of the dialog box using the specs in the language file, that color is held in the edit box m_Color structure
		HookedCtrl *EBCtrl = (HookedCtrl *)SendMessage(GetParent(hDlg), UM_INTERNAL_GETEBHOOK, 0, 0);
		return (INT_PTR)EBCtrl->m_Colors.DlgBkColorBrush;
	}
	case WM_CTLCOLORLISTBOX:
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLORBTN:
	{
		//similarly, do that for the controls on the form too
		HookedCtrl *EBCtrl = (HookedCtrl *)SendMessage(GetParent(hDlg), UM_INTERNAL_GETEBHOOK, 0, 0);
		HDC hdc = (HDC)wParam;
		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, EBCtrl->m_Colors.CtrlTextColor);
		SetBkColor(hdc, EBCtrl->m_Colors.CtrlBkColor);
		return (INT_PTR)EBCtrl->m_Colors.CtrlBkColorBrush;
	}
	case WM_DRAWITEM:
	{
		
		//draw each item according to it's type and id. this is a copy paste nightmare which I could have streamlined
		HookedCtrl *EBCtrl = (HookedCtrl *)SendMessage(GetParent(hDlg), UM_INTERNAL_GETEBHOOK, 0, 0);
		//EBCtrl->m_SearchDlg.m_GetSnapshot = true;
		//if (EBCtrl->m_SearchDlg.m_ReShowChildrenTime != 0)
		//	return 1l;

		
		LPDRAWITEMSTRUCT pDIS = (LPDRAWITEMSTRUCT)lParam;
		LONG ControlID = (LONG)wParam;
		HWND ControlhWnd = pDIS->hwndItem;
		HDC hDC = pDIS->hDC;
		RECT Rect = pDIS->rcItem;
		switch (ControlID)
		{
		case IDC_SEARCH_DROPDOWN_BUTTON:
		{
			SetBkMode(hDC, TRANSPARENT);
			SelectObject(hDC, EBCtrl->m_Colors.CtrlBkColorBrush);
			
			LRESULT State = SendMessage(ControlhWnd, BM_GETSTATE, 0, 0);
			if (EBCtrl->m_SearchDlg.m_SearchDropDownBut.MouseState == 1 || (State & BST_FOCUS) == BST_FOCUS)
			{
				SelectObject(hDC, EBCtrl->m_Colors.CtrlButColorBrush);
			}
			if ((State & BST_PUSHED) == BST_PUSHED)
			{
				SelectObject(hDC, EBCtrl->m_Colors.CtrlButDownColorBrush);
			}
			
			HPEN hOlPen = (HPEN)SelectObject(hDC, CreatePen(PS_SOLID,	1, GetTextColor(hDC)));
			POINT pts[DownArrowTemplateSize];
			TemplateToPoints(DownArrowTemplate, pts, DownArrowTemplateSize, Rect);
			Polygon(hDC,pts, DownArrowTemplateSize);
			DeleteObject(SelectObject(hDC, hOlPen));
			return TRUE;
		}
		case IDC_REPLACE_DROPDOWN_BUTTON:
		{
			SetBkMode(hDC, TRANSPARENT);
			SelectObject(hDC, EBCtrl->m_Colors.CtrlBkColorBrush);

			LRESULT State = SendMessage(ControlhWnd, BM_GETSTATE, 0, 0);
			if (EBCtrl->m_SearchDlg.m_ReplaceDropDownBut.MouseState == 1 || (State & BST_FOCUS) == BST_FOCUS)
			{
				SelectObject(hDC, EBCtrl->m_Colors.CtrlButColorBrush);
			}
			if ((State & BST_PUSHED) == BST_PUSHED)
			{
				SelectObject(hDC, EBCtrl->m_Colors.CtrlButDownColorBrush);
			}

			HPEN hOlPen = (HPEN)SelectObject(hDC, CreatePen(PS_SOLID, 1, GetTextColor(hDC)));
			POINT pts[DownArrowTemplateSize];
			TemplateToPoints(DownArrowTemplate, pts, DownArrowTemplateSize, Rect);
			Polygon(hDC, pts, DownArrowTemplateSize);
			DeleteObject(SelectObject(hDC, hOlPen));
			return TRUE;
		}
		case IDC_SEARCH_LEFT_BUTTON:
		{
			
			SetBkMode(hDC, TRANSPARENT);
			SelectObject(hDC, EBCtrl->m_Colors.CtrlBkColorBrush);
			TEXTMETRIC tm; GetTextMetrics(hDC, &tm);
			LRESULT State = SendMessage(ControlhWnd, BM_GETSTATE, 0, 0);
			if ((State & BST_PUSHED) == BST_PUSHED)
			{
				FillRect(hDC, &Rect, EBCtrl->m_Colors.CtrlButDownColorBrush);
			}
			else if (EBCtrl->m_SearchDlg.m_SearchLeftBut.MouseState == 1 || (State & BST_FOCUS) == BST_FOCUS)
			{
				FillRect(hDC, &Rect, EBCtrl->m_Colors.CtrlButHoverColorBrush);
			}
			else
			{
				FillRect(hDC, &Rect, EBCtrl->m_Colors.DlgBkColorBrush);
			}
			
			HPEN hOlPen = (HPEN)SelectObject(hDC, CreatePen(PS_SOLID, 1, GetTextColor(hDC)));
			POINT pts[SearchNextTemplateSize];
			TemplateToPoints(SearchNextTemplate, pts, SearchNextTemplateSize, Rect, -1.0f);
			Polygon(hDC, pts, SearchNextTemplateSize);
			DeleteObject(SelectObject(hDC, hOlPen));
			return TRUE;
		}
		case IDC_REPLACE_LEFT_BUTTON:
		{

			SetBkMode(hDC, TRANSPARENT);
			SelectObject(hDC, EBCtrl->m_Colors.CtrlBkColorBrush);
			LRESULT State = SendMessage(ControlhWnd, BM_GETSTATE, 0, 0);
			if ((State & BST_PUSHED) == BST_PUSHED)
			{
				FillRect(hDC, &Rect, EBCtrl->m_Colors.CtrlButDownColorBrush);
				
			}
			else if (EBCtrl->m_SearchDlg.m_ReplaceLeftBut.MouseState == 1 || (State & BST_FOCUS) == BST_FOCUS)
			{
				FillRect(hDC, &Rect, EBCtrl->m_Colors.CtrlButHoverColorBrush);
			}
			else
			{
				FillRect(hDC, &Rect, EBCtrl->m_Colors.DlgBkColorBrush);
			}

			HPEN hOlPen = (HPEN)SelectObject(hDC, CreatePen(PS_SOLID, 1, GetTextColor(hDC)));
			POINT pts[ReplaceNextTemplateSize];
			TemplateToPoints(ReplaceNextTemplate, pts, ReplaceNextTemplateSize, Rect,-1.0f);
			Polygon(hDC, pts, ReplaceNextTemplateSize);
			DeleteObject(SelectObject(hDC, hOlPen));
			return TRUE;
		}
		case IDC_SEARCH_RIGHT_BUTTON:
		{

			SetBkMode(hDC, TRANSPARENT);
			SelectObject(hDC, EBCtrl->m_Colors.CtrlBkColorBrush);
			LRESULT State = SendMessage(ControlhWnd, BM_GETSTATE, 0, 0);
			if ((State & BST_PUSHED) == BST_PUSHED)
			{
				FillRect(hDC, &Rect, EBCtrl->m_Colors.CtrlButDownColorBrush);
			}
			else if (EBCtrl->m_SearchDlg.m_SearchRightBut.MouseState == 1 || (State & BST_FOCUS) == BST_FOCUS)
			{
				FillRect(hDC, &Rect, EBCtrl->m_Colors.CtrlButHoverColorBrush);
			}
			else
			{
				FillRect(hDC, &Rect, EBCtrl->m_Colors.DlgBkColorBrush);
			}
			
			HPEN hOlPen = (HPEN)SelectObject(hDC, CreatePen(PS_SOLID, 1, GetTextColor(hDC)));
			POINT pts[SearchNextTemplateSize];
			TemplateToPoints(SearchNextTemplate, pts, SearchNextTemplateSize, Rect);
			Polygon(hDC, pts, SearchNextTemplateSize);
			DeleteObject(SelectObject(hDC, hOlPen));
			return TRUE;
		}
		case IDC_REPLACE_RIGHT_BUTTON:
		{

			SetBkMode(hDC, TRANSPARENT);
			SelectObject(hDC, EBCtrl->m_Colors.CtrlBkColorBrush);
			LRESULT State = SendMessage(ControlhWnd, BM_GETSTATE, 0, 0);
			if ((State & BST_PUSHED) == BST_PUSHED)
			{
				FillRect(hDC, &Rect, EBCtrl->m_Colors.CtrlButDownColorBrush);
			}
			else if (EBCtrl->m_SearchDlg.m_ReplaceRightBut.MouseState == 1 || (State & BST_FOCUS) == BST_FOCUS)
			{
				FillRect(hDC, &Rect, EBCtrl->m_Colors.CtrlButHoverColorBrush);
			}
			else
			{
				FillRect(hDC, &Rect, EBCtrl->m_Colors.DlgBkColorBrush);
			}

			HPEN hOlPen = (HPEN)SelectObject(hDC, CreatePen(PS_SOLID, 1, GetTextColor(hDC)));
			POINT pts[ReplaceNextTemplateSize];
			TemplateToPoints(ReplaceNextTemplate, pts, ReplaceNextTemplateSize, Rect);
			Polygon(hDC, pts, ReplaceNextTemplateSize);
			DeleteObject(SelectObject(hDC, hOlPen));
			return TRUE;
		}
		case IDC_REPLACE_ALL_BUTTON:
		{

			SetBkMode(hDC, TRANSPARENT);
			SelectObject(hDC, EBCtrl->m_Colors.CtrlBkColorBrush);
			LRESULT State = SendMessage(ControlhWnd, BM_GETSTATE, 0, 0);
			if ((State & BST_PUSHED) == BST_PUSHED)
			{
				FillRect(hDC, &Rect, EBCtrl->m_Colors.CtrlButDownColorBrush);
			}
			else if (EBCtrl->m_SearchDlg.m_ReplaceAllBut.MouseState == 1 || (State & BST_FOCUS) == BST_FOCUS)
			{
				FillRect(hDC, &Rect, EBCtrl->m_Colors.CtrlButHoverColorBrush);
			}
			else
			{
				FillRect(hDC, &Rect, EBCtrl->m_Colors.DlgBkColorBrush);
			}

			HPEN hOlPen = (HPEN)SelectObject(hDC, CreatePen(PS_SOLID, 1, GetTextColor(hDC)));
			POINT pts[ReplaceAllTemplateSize];
			TemplateToPoints(ReplaceAllTemplate, pts, ReplaceAllTemplateSize, Rect);
			Polygon(hDC, pts, ReplaceAllTemplateSize);
			DeleteObject(SelectObject(hDC, hOlPen));
			return TRUE;
		}
		case IDC_REPLACE_CURRENT_BUTTON:
		{
			SetBkMode(hDC, TRANSPARENT);
			SelectObject(hDC, EBCtrl->m_Colors.CtrlBkColorBrush);
			LRESULT State = SendMessage(ControlhWnd, BM_GETSTATE, 0, 0);
			if ((State & BST_PUSHED) == BST_PUSHED)
			{
				FillRect(hDC, &Rect, EBCtrl->m_Colors.CtrlButDownColorBrush);
			}
			else if (EBCtrl->m_SearchDlg.m_ReplaceCurrentBut.MouseState == 1 || (State & BST_FOCUS) == BST_FOCUS)
			{
				FillRect(hDC, &Rect, EBCtrl->m_Colors.CtrlButHoverColorBrush);
			}
			else
			{
				FillRect(hDC, &Rect, EBCtrl->m_Colors.DlgBkColorBrush);
			}

			HPEN hOlPen = (HPEN)SelectObject(hDC, CreatePen(PS_SOLID, 1, GetTextColor(hDC)));
			POINT pts[ReplaceTemplateSize];
			TemplateToPoints(ReplaceTemplate, pts, ReplaceTemplateSize, Rect);
			Polygon(hDC, pts, ReplaceTemplateSize);
			DeleteObject(SelectObject(hDC, hOlPen));
			return TRUE;
		}
		case IDC_CLOSE_SEARCH_BUTTON:
		{
			SetBkMode(hDC, TRANSPARENT);
			SelectObject(hDC, EBCtrl->m_Colors.CtrlBkColorBrush);
			TEXTMETRIC tm; GetTextMetrics(hDC, &tm);
			LRESULT State = SendMessage(ControlhWnd, BM_GETSTATE, 0, 0);
			if ((State & BST_PUSHED) == BST_PUSHED)
			{
				FillRect(hDC, &Rect, EBCtrl->m_Colors.CtrlButDownColorBrush);
			}
			else if (EBCtrl->m_SearchDlg.m_CloseSearchBut.MouseState == 1 || (State & BST_FOCUS) == BST_FOCUS)
			{
				FillRect(hDC, &Rect, EBCtrl->m_Colors.CtrlButHoverColorBrush);
			}
			else
			{
				FillRect(hDC, &Rect, EBCtrl->m_Colors.DlgBkColorBrush);
			}

			HPEN hOlPen = (HPEN)SelectObject(hDC, CreatePen(PS_SOLID, 1, GetTextColor(hDC)));
			POINT pts[CloseXTemplateSize];
			TemplateToPoints(CloseXTemplate, pts, CloseXTemplateSize, Rect);
			Polygon(hDC, pts, CloseXTemplateSize);
			DeleteObject(SelectObject(hDC, hOlPen));
			return TRUE;
		}
		
		case IDC_SEARCH_CASE_BUTTON:
		{

			SetBkMode(hDC, TRANSPARENT);
			SelectObject(hDC, EBCtrl->m_Colors.CtrlBkColorBrush);
			RECT FullRect = Rect;
			LRESULT State = SendMessage(ControlhWnd, BM_GETSTATE, 0, 0);
			if ((State & BST_PUSHED) == BST_PUSHED)
			{
				if (EBCtrl->m_SearchDlg.m_SearchCaseBut.State == 1) //Toggled
				{
					FillRect(hDC, &Rect, EBCtrl->m_Colors.CtrlButDownColorBrush);
					Rect.left += 1;
					Rect.top += 1;
					Rect.bottom -= 1;
					Rect.right -= 1;
				}
				FillRect(hDC, &Rect, EBCtrl->m_Colors.CtrlButDownColorBrush);
			}
			else if (EBCtrl->m_SearchDlg.m_SearchCaseBut.MouseState == 1 || (State & BST_FOCUS) == BST_FOCUS)
			{
				if (EBCtrl->m_SearchDlg.m_SearchCaseBut.State == 1) //Toggled
				{
					FillRect(hDC, &Rect, EBCtrl->m_Colors.CtrlButDownColorBrush);
					Rect.left += 1;
					Rect.top += 1;
					Rect.bottom -= 1;
					Rect.right -= 1;
				}
				FillRect(hDC, &Rect, EBCtrl->m_Colors.CtrlButHoverColorBrush);
			}
			else
			{
				if (EBCtrl->m_SearchDlg.m_SearchCaseBut.State == 1) //Toggled
				{
					FillRect(hDC, &Rect, EBCtrl->m_Colors.CtrlButDownColorBrush);
					Rect.left += 1;
					Rect.top += 1;
					Rect.bottom -= 1;
					Rect.right -= 1;
				}
				FillRect(hDC, &Rect, EBCtrl->m_Colors.DlgBkColorBrush);
			}
			
			{
				HPEN hOlPen = (HPEN)SelectObject(hDC, CreatePen(PS_SOLID, 1, GetTextColor(hDC)));
				POINT pts[AbTemplateSize];
				TemplateToPoints(AbTemplate, pts, AbTemplateSize, FullRect, 0.8f,0.6f);
				Polygon(hDC, pts, AbTemplateSize);
				DeleteObject(SelectObject(hDC, hOlPen));
			}
			{
				COLORREF BackColor = GetBkColor(hDC);
				COLORREF TextColor = GetTextColor(hDC);
				COLORREF LineColor = RGB(
					(int(GetRValue(BackColor)) + int(GetRValue(TextColor)) + int(GetRValue(TextColor))) / 3,
					(int(GetGValue(BackColor)) + int(GetGValue(TextColor)) + int(GetGValue(TextColor))) / 3,
					(int(GetBValue(BackColor)) + int(GetBValue(TextColor)) + int(GetBValue(TextColor))) / 3);
					

				HPEN hOlPen = (HPEN)SelectObject(hDC, CreatePen(PS_SOLID, 1, LineColor));
				POINT pts[HLineTemplateSize];
				TemplateToPoints(HLineTemplate, pts, HLineTemplateSize, FullRect);
				Polygon(hDC, pts, HLineTemplateSize);
				TemplateToPoints(HLineTemplate, pts, HLineTemplateSize, FullRect, 1.0f,-1.0f);
				Polygon(hDC, pts, HLineTemplateSize);
				DeleteObject(SelectObject(hDC, hOlPen));
			}
			
			return TRUE;
		}
		case IDC_SEARCH_WORD_BUTTON:
		{

			SetBkMode(hDC, TRANSPARENT);
			SelectObject(hDC, EBCtrl->m_Colors.CtrlBkColorBrush);
			RECT FullRect = Rect;
			TEXTMETRIC tm; GetTextMetrics(hDC, &tm);
			LRESULT State = SendMessage(ControlhWnd, BM_GETSTATE, 0, 0);
			if ((State & BST_PUSHED) == BST_PUSHED)
			{
				if (EBCtrl->m_SearchDlg.m_SearchWordBut.State == 1) //Toggled
				{
					FillRect(hDC, &Rect, EBCtrl->m_Colors.CtrlButDownColorBrush);
					Rect.left += 1;
					Rect.top += 1;
					Rect.bottom -= 1;
					Rect.right -= 1;
				}
				FillRect(hDC, &Rect, EBCtrl->m_Colors.CtrlButDownColorBrush);
			}
			else if (EBCtrl->m_SearchDlg.m_SearchWordBut.MouseState == 1 || (State & BST_FOCUS) == BST_FOCUS)
			{
				if (EBCtrl->m_SearchDlg.m_SearchWordBut.State == 1) //Toggled
				{
					FillRect(hDC, &Rect, EBCtrl->m_Colors.CtrlButDownColorBrush);
					Rect.left += 1;
					Rect.top += 1;
					Rect.bottom -= 1;
					Rect.right -= 1;
				}
				FillRect(hDC, &Rect, EBCtrl->m_Colors.CtrlButHoverColorBrush);
			}
			else
			{
				if (EBCtrl->m_SearchDlg.m_SearchWordBut.State == 1) //Toggled
				{
					FillRect(hDC, &Rect, EBCtrl->m_Colors.CtrlButDownColorBrush);
					Rect.left += 1;
					Rect.top += 1;
					Rect.bottom -= 1;
					Rect.right -= 1;
				}
				FillRect(hDC, &Rect, EBCtrl->m_Colors.DlgBkColorBrush);
			}

			{
				HPEN hOlPen = (HPEN)SelectObject(hDC, CreatePen(PS_SOLID, 1, GetTextColor(hDC)));
				POINT pts[AbTemplateSize];
				TemplateToPoints(AbTemplate, pts, AbTemplateSize, FullRect, 0.8f, 0.6f);
				Polygon(hDC, pts, AbTemplateSize);
				DeleteObject(SelectObject(hDC, hOlPen));
			}
			{
				COLORREF BackColor = GetBkColor(hDC);
				COLORREF TextColor = GetTextColor(hDC);
				COLORREF LineColor = RGB(
					(int(GetRValue(BackColor)) + int(GetRValue(TextColor)) + int(GetRValue(TextColor))) / 3,
					(int(GetGValue(BackColor)) + int(GetGValue(TextColor)) + int(GetGValue(TextColor))) / 3,
					(int(GetBValue(BackColor)) + int(GetBValue(TextColor)) + int(GetBValue(TextColor))) / 3);


				HPEN hOlPen = (HPEN)SelectObject(hDC, CreatePen(PS_SOLID, 1, LineColor));
				POINT pts[VLineTemplateSize];
				TemplateToPoints(VLineTemplate, pts, VLineTemplateSize, FullRect);
				Polygon(hDC, pts, VLineTemplateSize);
				TemplateToPoints(VLineTemplate, pts, VLineTemplateSize, FullRect, -1.0f);
				Polygon(hDC, pts, VLineTemplateSize);
				DeleteObject(SelectObject(hDC, hOlPen));
			}
			return TRUE;
		}
		}
		
		return TRUE;
	}
	
	}
	return (INT_PTR)FALSE;
}
SearchDialog::SearchDialog()
{
	//Nothing to do, everything is setup in the declarations
}
void SearchDialog::Init(HWND hParent)
{
	//the init is called to set the parent, the dialog box is actually create on first show, usually when the user hits f3
	m_hParent = hParent;
}
BOOL CALLBACK SrEnumUpdateRedraw(HWND   hWnd, LPARAM lParam)
{
	RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME);
	UpdateWindow(hWnd);
	return true;
}
void SearchDialog::ForceRefresh()
{
	RedrawWindow(m_hDlg, NULL, NULL, RDW_INVALIDATE|RDW_ERASE| RDW_FRAME);
	UpdateWindow(m_hDlg);
	EnumChildWindows(m_hDlg, SrEnumUpdateRedraw, 0);
	UpdateWindow(m_hDlg);
}
void SearchDialog::Show(DWORD ShowStyle)
{
	//create if not created
	if (m_hDlg == NULL || !IsWindow(m_hDlg))
	{
		m_hDlg = CreateDialog(ghDllHandle, MAKEINTRESOURCE(IDD_SEARCHREPLACE), m_hParent, SearchDialogProc);
		
		GetInitialPositions();
		ResizeToParentFont((HFONT)SendMessage(m_hParent, WM_GETFONT, 0, 0));
		UpdatePosition();
		
	}
	//show if nor shown
	if (!IsVisible())
	{
		ShowWindow(m_hDlg, ShowStyle);
	}
	//activate it in 2 different way, according to how it's shown. there extra step make sure the dialog is visible in fully drawn and active
	if (IsVisible() && ShowStyle == SW_SHOW)
	{
		SetActiveWindow(m_hDlg);
		SetFocus(m_hDlg);
	}
	if (IsVisible() && ShowStyle == SW_SHOWNA) //if not active show, focus back, this fixes the not drawing right on f3... ummm no it don't wth, intermitent fix
	{
		//RedrawWindow(m_hDlg, NULL, NULL, RDW_INVALIDATE); urgh, the final post message way does it
		SetFocus(m_hParent); //yep focus back to parent
	}
	if (IsVisible()) //Got you sonofabitch
	{
		ForceRefresh();
		//Sleep(30);
		PostMessage(m_hDlg, UM_INTERNAL_FORCEREFRESH,0,0);

	}

	
}

void SearchDialog::Hide()
{
	if (IsVisible()) ShowWindow(m_hDlg, SW_HIDE);
}
bool SearchDialog::IsVisible()
{
	return (m_hDlg && IsWindow(m_hDlg) && IsWindowVisible(m_hDlg));
}
bool SearchDialog::IsPtOverDlg(POINT pt)
{
	//if the point is over the dialog box. the HookedCtrl need to know this in orter to not step on interfered with the dialog mouse and cursor handling
	if (IsVisible())
	{
		RECT WindowRect; GetWindowRect(m_hDlg, &WindowRect);
		MapWindowPoints(NULL, m_hDlg, (POINT*)&WindowRect, 2);
		if (PtInRect(&WindowRect, pt))
		{
			true;
		}
	}
	return false;
}
void SearchDialog::Kill()
{
	//called at the end. cleanup and kile the window
	if (m_hDlg && IsWindow(m_hDlg))
	{
		DestroyWindow(m_hDlg);
		//we need to wait a bit otherwise KABOOOM. because this is called when the window is unhooked before the edit box is destroyed. if we dont wait. the edit box may destroy first
		while (IsWindow(m_hDlg)) Sleep(10);
	}
	
	m_hDlg = NULL;
}
SearchDialog::~SearchDialog()
{
	if (m_hDlg && IsWindow(m_hDlg)) DestroyWindow(m_hDlg);
}



void SearchDialog::UpdatePosition()
{
	
	//this is called when the position of the dialog needs updating, like when the edit box is resized
	if (m_hDlg && IsWindow(m_hDlg))
	{
		RECT ParentRect; GetClientRect(m_hParent, &ParentRect);
		RECT Rect; GetWindowRect(m_hDlg, &Rect);
		Rect.right -= Rect.left; //becomes width
		Rect.bottom -= Rect.top; //become height
		Rect.left = ParentRect.right - Rect.right - 4; //becomes pos, to the right
		Rect.top = 4; //becomes pos, top ofset 4
		Rect.right += Rect.left; //become end pos
		Rect.bottom += Rect.top; //become end pos
		MoveWindow(
			m_hDlg,
			Rect.left,
			Rect.top,
			(Rect.right - Rect.left),
			(Rect.bottom - Rect.top),
			1
		);
		if (IsWindowVisible(m_hDlg))
		{
			
			ForceRefresh();
			PostMessage(m_hDlg, UM_INTERNAL_FORCEREFRESH, 0, 0);
			
		}
		
	}
}

wstring SearchDialog::GetSearchText()
{
	TCHAR Buff[1025];
	GetWindowText(GetDlgItem(m_hDlg, IDC_SEARCHTEXT), Buff, 1024);
	return Buff;
}
wstring SearchDialog::GetReplaceText()
{
	TCHAR Buff[1025];
	GetWindowText(GetDlgItem(m_hDlg, IDC_REPLACETEXT), Buff, 1024);
	return Buff;
}

BOOL CALLBACK SrEnumResizeToParent(HWND   hWnd, LPARAM lParam)
{
	SEARCHDLGPOS *PosData = (SEARCHDLGPOS *)lParam;
	HFONT hFont = PosData->m_hFont;// (HFONT)SendMessage(GetParent(hWnd), WM_GETFONT, 0, 0);
	SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, 0);
	float ratio = float(PosData->m_nFontData.lfHeight) / float(PosData->m_oFontData.lfHeight);
	RECT *oR = &PosData->m_oCtrlRects[PosData->m_at];
	RECT nR;
	float vadjust = 1.0f;
	UINT id = GetDlgCtrlID(hWnd);
	if ( id == IDC_REPLACETEXT || id == IDC_SEARCHTEXT)
	{
		vadjust = 0.8f;
	}
	
	nR.left = (long)(float(oR->left) * ratio);
	nR.top = (long)(float(oR->top) * ratio);
	nR.right = (long)(float(oR->right) * ratio);
	nR.bottom = nR.top + (long)(float(oR->bottom-oR->top) * ratio * vadjust);
	MoveWindow(
		hWnd,
		nR.left,
		nR.top,
		(nR.right - nR.left),
		(nR.bottom - nR.top),
		0
	);
	PosData->m_at++;

	return true;
}
void SearchDialog::ResizeToParentFont(HFONT hFnt)
{
	if (m_hDlg == NULL) return;
	if (!IsWindow(m_hDlg)) return;
	HFONT hFont = hFnt;// (HFONT)SendMessage(m_hParent, WM_GETFONT, 0, 0);
	SendMessage(m_hDlg, WM_SETFONT, (WPARAM)hFont, 0);
	GetObject(hFont, sizeof(LOGFONT), &m_PosData.m_nFontData);
	m_PosData.m_nFontData.lfHeight = abs(m_PosData.m_nFontData.lfHeight);
	m_PosData.m_hFont = hFont;
	float ratio = float(m_PosData.m_nFontData.lfHeight) / float(m_PosData.m_oFontData.lfHeight);
	RECT *oR = &m_PosData.m_oRect;
	RECT *nR = &m_PosData.m_nRect;
	nR->left = 0;
	nR->top = 0;
	nR->right = (long)(float(oR->right-2) * ratio);
	nR->bottom = (long)(float(oR->bottom-2) * ratio);
	m_PosData.m_at = 0;
	EnumChildWindows(m_hDlg, SrEnumResizeToParent, (LPARAM)&m_PosData);
	//resize
	MoveWindow(
		m_hDlg,
		0,
		0,
		(nR->right - nR->left),
		(nR->bottom - nR->top),
		0
	);

	//move to right position
	UpdatePosition();
}
BOOL CALLBACK SrEnumPosData(HWND   hWnd, LPARAM lParam)
{
	SEARCHDLGPOS *PosData = (SEARCHDLGPOS *) lParam;
	RECT *MyRect = &PosData->m_oCtrlRects[PosData->m_at];
	GetWindowRect(hWnd, MyRect);
	//MyRect become relative to parent window rect
	MapWindowPoints(HWND_DESKTOP, GetParent(hWnd), (LPPOINT)MyRect, 2);

	PosData->m_at++;

	return true;
}
void SearchDialog::GetInitialPositions()
{
	GetWindowRect(m_hDlg, &m_PosData.m_oRect);
	HFONT hFont = (HFONT)SendMessage(m_hDlg,WM_GETFONT, 0, 0);
	
	GetObject(hFont, sizeof(LOGFONT), &m_PosData.m_oFontData);
	m_PosData.m_oFontData.lfHeight = abs(m_PosData.m_oFontData.lfHeight);

	//oRect becomes 0,0,width,height
	MapWindowPoints(HWND_DESKTOP, m_hDlg, (LPPOINT)&m_PosData.m_oRect, 2);
	m_PosData.m_at = 0;
	EnumChildWindows(m_hDlg, SrEnumPosData, (LPARAM)&m_PosData);
}