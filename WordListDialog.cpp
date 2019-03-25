#include "stdafx.h"
#include "WordListDialog.h"
//#include <Commctrl.h>
#include <Strsafe.h>
/*
Like SearchReplaceDialog. It's almost as insane. same concepts used, but it's simpler here because the only control involved is a list box
*/
WordListDialog::WordListDialog()
{
}




void WordListDialog::Init(HWND hParent)
{
	//the init is called to set the parent, the dialog box is actually create on first show, usually when the user hits f3
	m_hParent = hParent;
}

BOOL CALLBACK WlEnumUpdateRedraw(HWND   hWnd, LPARAM lParam)
{
	RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME);
	UpdateWindow(hWnd);
	return true;
}
void WordListDialog::ForceRefresh()
{
	RedrawWindow(m_hDlg, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME);
	UpdateWindow(m_hDlg);
	EnumChildWindows(m_hDlg, WlEnumUpdateRedraw, 0);
	UpdateWindow(m_hDlg);
}
WNDPROC LBOrgiWindowProc = NULL;

void DoLBPaint(HWND hWnd, HDC hDC)
{
	HookedCtrl *EBCtrl = (HookedCtrl *)SendMessage(GetParent(GetParent(hWnd)), UM_INTERNAL_GETEBHOOK, 0, 0);
	RECT Rect; GetClientRect(hWnd, &Rect);
	HBRUSH hBackBrush = (HBRUSH) SendMessage(GetParent(hWnd), WM_CTLCOLORLISTBOX, (WPARAM)hDC, 0);
	COLORREF DefaultCol = GetTextColor(hDC);
	DrawFilledRect(hDC, Rect, GetBkColor(hDC));
	LONG FirstVisibleItem = (LONG)SendMessage(hWnd, LB_GETTOPINDEX, 0, 0);
	LONG NumItems = (LONG) SendMessage(hWnd, LB_GETCOUNT, 0, 0);
	LONG SelectedItem = (LONG)SendMessage(hWnd, LB_GETCARETINDEX, 0, 0);
	
	//TEXTMETRIC tm; GetTextMetrics(hDC, &tm);
	//MEASUREITEMSTRUCT mis;
	//SendMessage(GetParent(hWnd), WM_MEASUREITEM, GetDlgCtrlID(hWnd), (LPARAM)&mis);
	INT Height = (INT)SendMessage(hWnd, LB_GETITEMHEIGHT, 0, 0); 
	INT OuterGap = Height;
	INT InnerBullet = OuterGap/2;
	INT StartOff = OuterGap / 2 - InnerBullet / 2;
	INT EndOff = OuterGap / 2 + InnerBullet / 2;

	short y = 0;
	HFONT oldFont = (HFONT)SelectObject(hDC,(HFONT)SendMessage(hWnd, WM_GETFONT, 0, 0));
	COLORREF OldCol = -1;
	COLORREF ThisCol = 0;
	if (FirstVisibleItem > 0)
	{
		wstringex *word = (wstringex*)SendMessage(hWnd, LB_GETITEMDATA, (WPARAM)FirstVisibleItem-1, 0);
		//if word type is set, get the color for it
		if(word->type!=-1)	OldCol = EBCtrl->m_ColorSpec->COLORS[word->type];
	}
	for(LONG itemIndex = FirstVisibleItem; itemIndex <NumItems; itemIndex++)
	{
		//get the word stored in the extra data. we can also use it as the text to simplyfie the string fetch
		wstringex *word = (wstringex*)SendMessage(hWnd, LB_GETITEMDATA, (WPARAM)itemIndex, 0);
		//The word has a extra data, marking the word type. default -1 if not used
		if (word->type != -1)
		{
			//Get the color from type
			ThisCol = EBCtrl->m_ColorSpec->COLORS[word->type];
			//Draw a bullet with he color on the left
			DrawFilledRect(hDC, { 2 + StartOff ,y + StartOff ,2 + EndOff,y + 2 + EndOff }, ThisCol);
		}
		
		// Draw the text, if the word is a valid type, add a gap for the bullet
		TextOut(hDC, 2+ OuterGap * (int)(word->type != -1), y, word->c_str(), (int)word->length());
		
		//if we switched word type/category, draw seperator line
		if (OldCol != ThisCol && word->type != -1)
		{
			//draw end of group line
			if (OldCol != -1) DrawFilledRect(hDC, { 2 + OuterGap ,y - 2 ,Rect.right - 2,y -1 }, OldCol);
			//draw start of group line
			DrawFilledRect(hDC, { 2 + OuterGap ,y+1 ,Rect.right - 2,y + 2 }, ThisCol);
			OldCol = ThisCol;
		}
		//last item, draw the end sep line
		if(itemIndex == NumItems-1 && word->type != -1)
		{
			DrawFilledRect(hDC, { 2 + OuterGap ,y+Height-2 ,Rect.right - 2,y + Height - 1}, ThisCol);
			OldCol = ThisCol;
		}
		
		
		//Is Highlight
		if (SelectedItem == itemIndex)
		{
			
			DrawFrameRect(hDC, {1, y, Rect.right, y + Height }, GetTextColor(hDC));
		}

		
		y += Height;
		if (y > Rect.bottom)
		{
			break;
		}
	   
	}
	SelectObject(hDC, (HFONT)oldFont);
}
//We overrided the main windowproc of the list box for more control
LRESULT CALLBACK LBWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	
	
	case WM_MOUSEWHEEL:
	{
		short Delta = GET_WHEEL_DELTA_WPARAM(wParam);
		int MoveDir = -1;
		if (Delta < 0) MoveDir = 1;
		LRESULT topIndex = SendMessage(hWnd, LB_GETTOPINDEX, 0, 0);
		LRESULT ItemHeight = SendMessage(hWnd, LB_GETITEMHEIGHT, 0, 0);
		RECT r; GetClientRect(hWnd, &r);
		int NumVisible = r.bottom / (int)ItemHeight;
		int MoveAmount = max(1,NumVisible / 3);
		SendMessage(hWnd, LB_SETTOPINDEX, max(0,topIndex+ MoveDir * MoveAmount), 0);
		return 0l;
	}
	
	case WM_KEYDOWN:
	{
		if (wParam == VK_RETURN)
		{
			SendMessage(GetParent(hWnd), UM_INTERNAL_WORDSELECTED, 0, (LPARAM)hWnd);
			return 0l;
		}
		else if (wParam == VK_ESCAPE)
		{
			SendMessage(GetParent(hWnd), UM_INTERNAL_CANCELED, 0, (LPARAM)hWnd);
			return 0l;
		}
		break;
	}
	
	case WM_LBUTTONDBLCLK:
	{
		SendMessage(GetParent(hWnd), UM_INTERNAL_WORDSELECTED, 0, (LPARAM)hWnd);
		return 0l;
		break;
	}
	case WM_KILLFOCUS:
	{
		SendMessage(GetParent(hWnd), UM_INTERNAL_CANCELED, 0, (LPARAM)hWnd);
		return 0l;
		break;
	}
	case WM_ERASEBKGND:
	{
		DoLBPaint(hWnd, (HDC)wParam);
		return 1l;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hDC = BeginPaint(hWnd, &ps);
		DoLBPaint(hWnd, hDC);
		EndPaint(hWnd, &ps);
		return 0l;
	}
	
	}
	LONG CaretIndex = (LONG)CallWindowProc(LBOrgiWindowProc,hWnd, LB_GETCARETINDEX, 0, 0);
	LONG TopIndex = (LONG)CallWindowProc(LBOrgiWindowProc, hWnd, LB_GETTOPINDEX, 0, 0);
	CallWindowProc(LBOrgiWindowProc, hWnd, WM_SETREDRAW, 0, 0);
	LRESULT r = CallWindowProc(LBOrgiWindowProc, hWnd, message, wParam, lParam);
	CallWindowProc(LBOrgiWindowProc, hWnd, WM_SETREDRAW, 1, 0);
	if (CaretIndex != (LONG)CallWindowProc(LBOrgiWindowProc, hWnd, LB_GETCARETINDEX, 0, 0))
	{
		RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else if (TopIndex != (LONG)CallWindowProc(LBOrgiWindowProc, hWnd, LB_GETTOPINDEX, 0, 0))
	{
		RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	return r;
	
}
INT_PTR CALLBACK WordListDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	switch (message)
	{
	case UM_INTERNAL_GETEBHOOK:
	{
		return SendMessage(GetParent(hDlg), UM_INTERNAL_GETEBHOOK, 0, 0);
	}
	case WM_INITDIALOG:
	{
		//so we set the window procedure of every control under the sun to ours so we can catch the keys.
		LBOrgiWindowProc = (WNDPROC)GetWindowLongPtr(GetDlgItem(hDlg, IDC_WORDLIST), GWLP_WNDPROC);
		SetWindowLongPtr(GetDlgItem(hDlg, IDC_WORDLIST), GWLP_WNDPROC, (LONG_PTR)LBWindowProc);
		return (INT_PTR)TRUE;
	}
	
	case UM_INTERNAL_WORDSELECTED:
	{
		HookedCtrl *EBCtrl = (HookedCtrl *)SendMessage(GetParent(hDlg), UM_INTERNAL_GETEBHOOK, 0, 0);
		SendMessage(EBCtrl->m_WordListDlg.m_hWndDispatch, UM_INTERNAL_WORDSELECTED, EBCtrl->m_WordListDlg.m_IDCDispatch, lParam);
		EBCtrl->m_WordListDlg.Hide();
		return 0l;
	}
	case UM_INTERNAL_CANCELED:
	{
		HookedCtrl *EBCtrl = (HookedCtrl *)SendMessage(GetParent(hDlg), UM_INTERNAL_GETEBHOOK, 0, 0);
		SendMessage(EBCtrl->m_WordListDlg.m_hWndDispatch, UM_INTERNAL_CANCELED, EBCtrl->m_WordListDlg.m_IDCDispatch, lParam);
		EBCtrl->m_WordListDlg.Hide();
		return 0l;
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
	case UM_INTERNAL_FORCEREFRESH:
	{
		HookedCtrl *EBCtrl = (HookedCtrl *)SendMessage(GetParent(hDlg), UM_INTERNAL_GETEBHOOK, 0, 0);
		EBCtrl->m_WordListDlg.ForceRefresh();
		return 0;
	}
	case WM_SIZE:
	{
		RECT r; GetClientRect(hDlg, &r);
		MoveWindow(GetDlgItem(hDlg, IDC_WORDLIST), r.left, r.top, r.right, r.bottom,  1);
		HookedCtrl *EBCtrl = (HookedCtrl *)SendMessage(GetParent(hDlg), UM_INTERNAL_GETEBHOOK, 0, 0);
		EBCtrl->m_WordListDlg.ForceRefresh();
		break;
	}
	

	}
	return (INT_PTR)FALSE;
}
void WordListDialog::Show(DWORD ShowStyle, short x, short y, short w, short h, short InvertedOffset)
{
	//create if not created
	if (m_hDlg == NULL || !IsWindow(m_hDlg))
	{
		m_hDlg = CreateDialog(ghDllHandle, MAKEINTRESOURCE(IDD_WORDLISTBOX), m_hParent, WordListDialogProc);
		GetInitialPositions();
		ResizeToParentFont((HFONT)SendMessage(m_hParent, WM_GETFONT, 0, 0));
		//UpdatePosition();

	}
	if (x != -9999 && y != -9999 && m_hDlg != NULL && IsWindow(m_hDlg))
	{
		int ww = w, hh = h;

		if (w == -9999 || h == -9999)
		{   
			RECT r; GetWindowRect(m_hDlg, &r);
			if (w == -9999) ww = (r.right - r.left);
			if (h == -9999) hh = (r.bottom - r.top);
		}
		RECT par_r; GetClientRect(m_hParent, &par_r);
		short xx = max(0, min(x, (short)par_r.right - ww));
		short yy = max(0, min(y, (short)par_r.bottom - hh));
		if (yy + hh >= par_r.bottom && yy != 0)
		{
			yy -= InvertedOffset;
		}
		MoveWindow(m_hDlg, xx, yy, ww, hh, 0);

		SetWindowPos(m_hDlg, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
	//show if nor shown
	if (!IsVisible())
	{
		ShowWindow(m_hDlg, ShowStyle);
		m_Visible = ShowStyle == SW_SHOW || ShowStyle == SW_SHOWNA;
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
		SetFocus(m_hWndDispatch); //yep focus back to control echoing content
	}
	if (IsVisible()) //Got you sonofabitch
	{
		
		ForceRefresh();
		//Sleep(30);
		PostMessage(m_hDlg, UM_INTERNAL_FORCEREFRESH, 0, 0);
	}
}
void WordListDialog::Hide()
{
	//if (IsVisible()) ShowWindow(m_hDlg, SW_HIDE);
	if (m_hDlg)
	{
		if (IsWindow(m_hDlg))
		{
			//if (IsWindowVisible(m_hDlg))
			{
				ShowWindow(m_hDlg, SW_HIDE);
				m_Visible = false; //not sure why but for some reason IsWindowVisible() returns false when the window is visible, so this flag hacks it
			}
		}
	}
}

bool WordListDialog::IsVisible(bool UseFlag)
{
	if (UseFlag) return m_Visible;
	return (m_hDlg && IsWindow(m_hDlg) && IsWindowVisible(m_hDlg));
}
bool WordListDialog::IsPtOverDlg(POINT pt)
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
void WordListDialog::Kill()
{
	//called at the end. cleanup and kile the window
	if (m_hDlg && IsWindow(m_hDlg))
	{
		DestroyWindow(m_hDlg);
		m_Visible = false;
		//we need to wait a bit otherwise KABOOOM. because this is called when the window is unhooked before the edit box is destroyed. if we dont wait. the edit box may destroy first
		while (IsWindow(m_hDlg)) Sleep(10);
	}
	
	m_hDlg = NULL;
}
WordListDialog::~WordListDialog()
{
	if (m_hDlg && IsWindow(m_hDlg)) DestroyWindow(m_hDlg);
}


void WordListDialog::UpdatePosition()
{

	//this is called when the position of the dialog needs updating, like when the edit box is resized
	if (m_hDlg && IsWindow(m_hDlg))
	{
		/*
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
			Rect.top + 300,
			(Rect.right - Rect.left),
			(Rect.bottom - Rect.top),
			1
		);
		*/
		if (IsWindowVisible(m_hDlg))
		{
			SetWindowPos(m_hDlg, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			ForceRefresh();
			PostMessage(m_hDlg, UM_INTERNAL_FORCEREFRESH, 0, 0);

		}

	}
}

BOOL CALLBACK WlEnumResizeToParent(HWND   hWnd, LPARAM lParam)
{
	SEARCHDLGPOS *PosData = (SEARCHDLGPOS *)lParam;
	HFONT hFont = PosData->m_hFont;// (HFONT)SendMessage(GetParent(hWnd), WM_GETFONT, 0, 0);
	SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, 0);
	//No need for this now
	/*
	float ratio = float(PosData->m_nFontData.lfHeight) / float(PosData->m_oFontData.lfHeight);
	RECT *oR = &PosData->m_oCtrlRects[PosData->m_at];
	RECT nR;
	nR.left = (long)(float(oR->left) * ratio);
	nR.top = (long)(float(oR->top) * ratio * 0.8);
	nR.right = (long)(float(oR->right) * ratio);
	nR.bottom = (long)(float(oR->bottom) * ratio * 0.8);
	MoveWindow(
		hWnd,
		nR.left,
		nR.top,
		(nR.right - nR.left),
		(nR.bottom - nR.top),
		0
	);
	*/
	PosData->m_at++;
	
	return true;
}

void WordListDialog::ResizeToParentFont(HFONT hFnt)
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
	nR->right = (long)(float(oR->right - 2) * ratio);
	nR->bottom = (long)(float(oR->bottom - 2) * ratio * 0.8);
	m_PosData.m_at = 0;
	EnumChildWindows(m_hDlg, WlEnumResizeToParent, (LPARAM)&m_PosData);
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
BOOL CALLBACK WlEnumPosData(HWND   hWnd, LPARAM lParam)
{
	SEARCHDLGPOS *PosData = (SEARCHDLGPOS *)lParam;
	RECT *MyRect = &PosData->m_oCtrlRects[PosData->m_at];
	GetWindowRect(hWnd, MyRect);
	//MyRect become relative to parent window rect
	MapWindowPoints(HWND_DESKTOP, GetParent(hWnd), (LPPOINT)MyRect, 2);

	PosData->m_at++;

	return true;
}
void WordListDialog::GetInitialPositions()
{
	GetWindowRect(m_hDlg, &m_PosData.m_oRect);
	HFONT hFont = (HFONT)SendMessage(m_hDlg, WM_GETFONT, 0, 0);

	GetObject(hFont, sizeof(LOGFONT), &m_PosData.m_oFontData);
	m_PosData.m_oFontData.lfHeight = abs(m_PosData.m_oFontData.lfHeight);

	//oRect becomes 0,0,width,height
	MapWindowPoints(HWND_DESKTOP, m_hDlg, (LPPOINT)&m_PosData.m_oRect, 2);
	m_PosData.m_at = 0;
	EnumChildWindows(m_hDlg, WlEnumPosData, (LPARAM)&m_PosData);
}

void  WordListDialog::SetWordList(WORDLIST WordList)
{
	if (m_hDlg != NULL)
	{
	HWND hwndList = GetDlgItem(m_hDlg, IDC_WORDLIST);
	SendMessage(hwndList, LB_RESETCONTENT, 0, 0);
	}
	m_WordList = WordList;

}
void WordListDialog::RefreshListContent(wstring Word, int trim, wstring Alternative)
{
	if (!IsVisible()) return;
	HWND hwndList = GetDlgItem(m_hDlg, IDC_WORDLIST);
	SendMessage(hwndList, LB_RESETCONTENT, 0, 0);

	std::list<wstringex>::iterator it;
	LONG i = 0;
	size_t len = Word.length();
	const TCHAR* wrd = Word.c_str();
	const TCHAR* altwrd = Alternative.c_str();
	size_t  altlen = Alternative.length();
	const TCHAR* listwrd;
	bool addit = true;
	bool added = false;
	for (it = m_WordList.begin(); it != m_WordList.end(); ++it)
	{
		
		
		addit = trim == 0;
		listwrd = it->c_str();
		//if (trim && ((_wcsnicmp(wrd, listwrd, len) == 0) || (_wcsnicmp(altwrd, listwrd, altlen)) == 0))
		//if (trim && ((wcsstr(listwrd,wrd) != NULL) || (wcsstr(listwrd, altwrd) == 0)))
		if (trim && (IsInStr(listwrd, wrd) || IsInStr(listwrd, altwrd)))
		{
			addit = true;
		}
		if(addit)
		{
			
			LONG pos = (LONG)SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)listwrd);
			// This is silly to set the items data to the string we've just set in the item 
			// but getting the selected item text (later) explodes, so fuck it, I aint got time for that
			wstringex *t =  &(*it);
			SendMessage(hwndList, LB_SETITEMDATA, pos, (LPARAM)t);
			added = true;
		}
		i++;
	}

	if (added == false) //if nothing added re-do it and fill with everything
	{
		for (it = m_WordList.begin(); it != m_WordList.end(); ++it)
		{
			listwrd = it->c_str();
			LONG pos = (LONG)SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)listwrd);
			wstringex *t = &(*it);
			SendMessage(hwndList, LB_SETITEMDATA, pos, (LPARAM)t);
		}
	}
	

	LRESULT found = SendMessage(hwndList, LB_FINDSTRING, -1, (LPARAM)wrd);
	if (found != LB_ERR)
	{
		SendMessage(hwndList, LB_SETCARETINDEX, found, 0);
		SendMessage(hwndList, LB_SETCURSEL, found, 0);
	}
	else
	{
		SendMessage(hwndList, LB_SETCARETINDEX, 0, 0);
		SendMessage(hwndList, LB_SETCURSEL, 0, 0);
	}
}
void WordListDialog::SetDispatch(HWND hReceiver, DWORD ControlID)
{
	//sets the handle of the window to recieve messages... and the control id (or an arbitrary number) which may be a different window control than the receiver
	//the id will be passed to the reciever
	m_hWndDispatch = hReceiver;
	m_IDCDispatch = ControlID;
}

