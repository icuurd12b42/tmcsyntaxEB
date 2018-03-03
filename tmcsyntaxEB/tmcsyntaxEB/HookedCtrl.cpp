#include "stdafx.h"
#include "HookedCtrl.h"

#define XY_TO_LPARAM(x,y) MAKELPARAM(x,y)

#define EB_CHARPOS_FROM_LRESULT(n) LOWORD(n)
#define EB_LINE_FROM_LRESULT(n) HIWORD(n)
#define EB_XPOS_FROM_LRESULT(n) LOWORD(n)
#define EB_YPOS_FROM_LRESULT(n) HIWORD(n)

#define CAP_TO_EB_SIZE(n) max(0,min(n,65535))


//int m_SelEnd, m_SelStart, delta;
#define EB_MOVE_CURSOR(delta) {m_SelEnd = CAP_TO_EB_SIZE(m_SelEnd+delta); m_SelStart = m_SelEnd; EB_SET_SEL(m_SelStart, m_SelEnd);}
#define EB_MOVE_CURSOR_SELECT(delta) {m_SelEnd = CAP_TO_EB_SIZE(m_SelEnd+delta); EB_SET_SEL(m_SelStart, m_SelEnd);}


#define RM_UNDO 1
#define RM_REDO 2
#define RM_CUT 3
#define RM_COPY 4
#define RM_PASTE 5
#define RM_DELETE 6
#define RM_SELECTALL 7
#define RM_MATCHINGBRACKET 8
#define RM_COMMENT 9
#define RM_UNCOMMENT 10
#define RM_FIND 11
#define RM_INDENT 12
#define RM_LINENUMBER 13

#define RE_SYNTAXENABLED 100
HookedCtrl::HookedCtrl(HWND hWnd)
{
	m_hWnd = hWnd;
	COLORREF color = 0xAAAAAA;
	
	for (int i = 0; i < 65536; i++)	{
		m_EBText[i] = 0;
		m_EBLine[i] = 0;
		m_EBTextMirror[i] = 0;
		m_CharColors[i] = color;
		
	}
	m_Parser.LinkToEbContent(m_EBText, m_EBTextMirror, m_CharColors, this);

	//m_RightClickMenu = CreatePopupMenu();
	
	
}


HookedCtrl::~HookedCtrl()
{
	OnUnHook();
	//DestroyMenu(m_RightClickMenu);
}

void HookedCtrl::OnHooked(DWORD LineNumberBackColor, DWORD LineNumberColor)
{
	//Init Vars Here, Initial (re-)Configuration
	int ts[1];
	ts[0] = m_TabSize;
	DirectSendMessage(EM_SETLIMITTEXT, 65535, 0);
	DirectSendMessage(EM_SETTABSTOPS, 1, (LPARAM)ts); //set the tabs to half, essentially 4 chars intead of 8
	m_LineNumberBackColor = (COLORREF)LineNumberBackColor;
	m_LineNumberColor = (COLORREF)LineNumberColor;
	if (IsWindowVisible(m_hWnd))
	{
		RedrawWindow(GetParent(m_hWnd), NULL, NULL, RDW_ERASE | RDW_INVALIDATE);
		RedrawWindow(m_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);
	}
	
}

void HookedCtrl::OnUnHook()
{
	if (IsWindow(m_hWnd)) {
		if (IsWindowVisible(m_hWnd))
		{
			SetWindowLong(m_hWnd, GWL_WNDPROC, (LONG_PTR)m_origWndProc);
			RedrawWindow(m_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);
		}
	}
}

LRESULT HookedCtrl::DirectSendMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	return CallWindowProc(m_origWndProc, m_hWnd, message, wParam, lParam);
}

void HookedCtrl::InvalidateSelectRegion(int SelStart, int SelEnd)
{
	int selStart = SelStart;
	int selEnd = SelEnd;
	if (selStart > selEnd)	{
		swap(selStart, selEnd);
	}
	//if a selection
	if (selEnd - selStart)	{
		RECT rect = this->GetClientRect(); 
		rect.top = max(rect.top, this->GetPosFromChar(selStart).y);
		rect.bottom = min(rect.bottom, this->GetPosFromChar(max(0,selEnd-1)).y + m_TextHeight);
		//InvalidateRect(m_hWnd, &rect, FALSE);
	}
}

void HookedCtrl::InvalidateEditChar(int CharPos)
{
	RECT rect = this->GetClientRect();
	rect.top = max(rect.top, this->GetPosFromChar(CharPos).y);
	//InvalidateRect(m_hWnd, &rect, FALSE);
}



//that is one nasty ass long piece of code
LRESULT HookedCtrl::Do_WM_PAINT(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//text the text on redraw, because we always nee that reference data
	if(m_SyntaxHighlighting)
		m_Parser.Parse(DirectSendMessage(WM_GETTEXT, 65536, (LPARAM)m_EBText));

	//The region of the control
	RECT rect = this->GetClientRect();
	RECT    rcCli = { rect.left, rect.top, rect.right+2, rect.bottom+2 };
	int rightEdge = rect.right;
	//get the dc/begin drawPAINTSTRUCT ps;
	PAINTSTRUCT ps;
	int TerminatePaintMode = 0; //determines how we clean up the dc
	//get the dc
	HDC whdc = BeginPaint(hwnd, &ps);
	if (whdc == NULL) {
		//if failed, try window
		whdc = GetDC(hwnd);
		TerminatePaintMode = 1;
		if (whdc == NULL) {
			//failed, quit
			return 0L;
		}
	}
	else {
		//paint, therefore a possibly smaller region
		CopyRect(&rect, &ps.rcPaint);
	}
	
	int nWidth = rcCli.right - rcCli.left;
	int nHeight = rcCli.bottom - rcCli.top;

	HBITMAP hbm = 0, oldbm = 0;;
	HDC hdc = CreateCompatibleDC(whdc);
	if (hdc == NULL)
	{
		hdc = whdc;
		
	}
	else
	{
		
		// then you might have: 
		hbm = CreateCompatibleBitmap(whdc, nWidth, nHeight);
		if (hbm)
		{
			oldbm = (HBITMAP)SelectObject(hdc, hbm);
		}
		else
		{
			hdc = whdc;
		}
	}

	
	//set the port clipping region
	//HRGN hrgn = CreateRectRgn(rect.left, rect.top, rect.right, rect.bottom);
	//HRGN hrgn = CreateRectRgn(rcCli.left, rcCli.top, rcCli.right, rcCli.bottom);
	//Clip that port
    //SelectClipRgn(whdc, hrgn);

   	//clear the background
	if (1)
	{
		RECT clearRect = { rcCli.left - 10,rcCli.top,rcCli.right,rcCli.bottom };

		HBRUSH BackBrush = CreateSolidBrush(m_Language->TEXTBACKCOLOR);
		FillRect(hdc, &clearRect, BackBrush);
		DeleteObject(BackBrush);
	}

	//set the font as defined by the control settings
	HFONT hfont = (HFONT)DirectSendMessage(WM_GETFONT, NULL, NULL);
	HFONT hfOld = (HFONT)SelectObject(hdc, hfont);
	//Get the text metrics
	//Font size
	TEXTMETRIC textMetrics;
	GetTextMetrics(hdc, &textMetrics);
	m_TextHeight = textMetrics.tmHeight;
	//set the select color
	SetBkMode(hdc, OPAQUE);
	//draw the selection 
	int selStart = m_SelStart;
	int selEnd = m_SelEnd;
	if (selStart > selEnd)	{
		swap(selStart, selEnd);
	}
	//if a selection
	if(selEnd-selStart)	{
		//make the brush for the draw
		HBRUSH SelBrush = CreateSolidBrush(m_Language->SELECTEDTEXTCOLOR);
		
		//get the x,y coords for start
		PosFromChar StartPos = this->GetPosFromChar(selStart);

		int charOff = 0;
		//special case for selection end after last character on line, resulting is error in pos from char
		if (this->GetTextLenght() == selEnd) charOff = 1;
		
		//get the x,y coords for end
		PosFromChar EndPos = this->GetPosFromChar(selEnd - charOff);
		
		//get the line pos for those too
		int StartLine = this->GetLineFromPos(selStart);
		int EndLine = this->GetLineFromPos(selEnd - charOff);

		//add the width of the last character
		if (charOff){
			int len = this->GetEBLine(EndLine);
			GetCharWidth32(hdc,	m_EBLine[len-1],m_EBLine[len - 1], &charOff);
			EndPos.x += charOff + 10;
		}

		//loop though each line selected
		short lineXStart, lineYStart, lineXEnd, lineYEnd;
		lineYStart = StartPos.y;
		
		for (int i = StartLine; i <= EndLine; i++)	{
			//default
			lineXStart = 0;
			lineXEnd = 0;
			lineYEnd = lineYStart + (short)textMetrics.tmHeight;
			//ignore lines above dirty region
			if (lineYEnd > rect.top) {
				//get the extent of the selection from start to end of line, 
				int len = this->GetEBLine(i);
				if (len) {
					int CharPos = this->GetFirstPosInLine(i);
					lineXStart = 0;
					CharPos = CharPos + len - 1;
					PosFromChar position = this->GetPosFromChar(CharPos);
					lineXEnd = position.x;

					GetCharWidth32(hdc,	m_EBLine[len - 1], m_EBLine[len - 1], &charOff);
					lineXEnd += charOff;
					lineXEnd = (short)min(lineXEnd, rect.right);
				}
				//drawing rectagle dimention
				RECT selRect = { lineXStart, lineYStart, lineXEnd, lineYEnd };
				//adjust if first sel line
				if (i == StartLine) {
					selRect.left = max(StartPos.x, selRect.left);
				}
				//same for last sel line
				if (i == EndLine) {
					selRect.right = min(EndPos.x, selRect.right);
				}
				//draw
				FillRect(hdc, &selRect, SelBrush);
			}
			lineYStart += (short)textMetrics.tmHeight;
			if (lineYStart + textMetrics.tmHeight > rect.bottom) {
				break; //exit if below dirty region
			}
		}
		DeleteObject(SelBrush);

	}
	//set the background transparent so character dont mess with each other's edges and does not erase the selection. it's the reason I draw the selection first
	SetBkMode(hdc, TRANSPARENT);


	//delete the region and reset to new smaller size
	//DeleteObject(hrgn);
	//rect.right = min(rightEdge, rect.right);
	//hrgn = CreateRectRgn(rect.left, rect.top, rect.right, rect.bottom);
	//Clip that port
	//SelectClipRgn(whdc, hrgn);
	//loop from first visible line to the end
	//int FirstLine = DirectSendMessage(EM_GETFIRSTVISIBLELINE, NULL, NULL);
	
	//Text Drawing
	{
		CharFromPos charFromPos = this->GetCharFromPos(rect.left, rect.top);
		int FirstLine = charFromPos.LinePos;

		int LineCount = this->GetLineCount(); 
		short posX = 0,
			  posY = 0;
		for(int i = FirstLine; i< LineCount; i++){
			//get the line
			int len = this->GetEBLine(i);

			//Setup Word information variables
			COLORREF color = -1;
			int accumulatedlen = 0;
			int wordPosX = 0;
			int wordStarPosOnLine = 0;
			int wordStarPosinBox = 0;
			int wordPosY = 0;
			//if line is not empty
			if (len){
				int FirstCharIndexInLine = this->GetFirstPosInLine(i);
				
				//end position in buffer
				int EndPos = FirstCharIndexInLine + len;
				//I also need the position on the line buffer
				int CharPosOnLine = 0;
			
				//for each character in line
				for (int cat = FirstCharIndexInLine; cat < EndPos; cat++){
					//get position for the character using it's overall position in buffer
					PosFromChar position = this->GetPosFromChar(cat);
					posX = position.x;
					posY = position.y;
					//break if below the box this will happen on char[0] of the line
					if (posY + textMetrics.tmHeight > rect.bottom)
					{
						break;
					}
					//if it's passed the left top edge of the port
					if (posX >= rect.left && posY >= rect.top)
					{
						//is it passed the right edge?
						if (posX >= rect.right){
							//break out of character loop
							//but first draw the word accumulated
							if (accumulatedlen != 0){
								SetTextColor(hdc, m_CharColors[wordStarPosinBox]);
								TextOut(hdc, wordPosX, wordPosY, (TCHAR*)(m_EBLine + wordStarPosOnLine), accumulatedlen);
								accumulatedlen = 0;
							}
							break;
						}
						else
						{
							//if it's tab or a a color change
							bool isTab = m_EBLine[CharPosOnLine] == '\t';
							if (m_CharColors[cat] != color || isTab) {
								//draw accumulated word
								if (accumulatedlen != 0)
								{
									SetTextColor(hdc, m_CharColors[wordStarPosinBox]);
									TextOut(hdc, wordPosX, wordPosY, (TCHAR*)(m_EBLine + wordStarPosOnLine), accumulatedlen);
								}
								//setup for next word
								accumulatedlen = 0;
								color = m_CharColors[cat];
								if (isTab) color = -1; //if tab, hard set the color to cause it to draw as a word on next iteration
								//this new word will start at this position
								wordPosX = posX;
								wordPosY = posY;
								wordStarPosOnLine = CharPosOnLine;
								wordStarPosinBox = cat;
							}
							//acumulate the word length
							accumulatedlen++;
						}
					}
					CharPosOnLine++;
				}
				//Dangling word to process?
				if (accumulatedlen != 0) {
					SetTextColor(hdc, m_CharColors[wordStarPosinBox]);
					TextOut(hdc, wordPosX, wordPosY, (TCHAR*)(m_EBLine + wordStarPosOnLine), accumulatedlen);
				}
			}
		
			//if the last posY was below below the port
			if (posY > rect.bottom)	{
				//break out of line loop
				break;
			}
		}
	}
	
	//end the draw reset things
	//SetBkMode(hdc, OPAQUE);
	//re-add the old font in the dc
	SelectObject(hdc, hfOld);
	//delete the region
	//DeleteObject(hrgn);

	//used mem buffer
	if (hdc != whdc)
	{
		
		 BitBlt(
			 whdc,//_In_ HDC   hdcDest,
			 0,//_In_ int   nXDest,
			 0,//_In_ int   nYDest,
			 nWidth, //_In_ int   nWidth,
			 nHeight, //_In_ int   nHeight,
			 hdc,//_In_ HDC   hdcSrc,
			 0,//_In_ int   nXSrc,
			 0,//_In_ int   nYSrc,
			 SRCCOPY//_In_ DWORD dwRop
		 );
		 
		 
	}

	//done painting, releace dc
	if (TerminatePaintMode == 0){
		EndPaint(hwnd, &ps);
	}
	else if (TerminatePaintMode == 1){
		ReleaseDC(hwnd, whdc);
	}

	
	
	if (hdc != whdc)
	{
		if (m_DoLineNumbers)
		{
			HBRUSH BackBrush = CreateSolidBrush(m_LineNumberBackColor);
			RECT clearRect = { rcCli.left - 10,rcCli.top - 10,rcCli.right,rcCli.bottom };
			FillRect(hdc, &clearRect, BackBrush);
			DeleteObject(BackBrush);

			SIZE sz;

			GetTextExtentPoint32(
				hdc,
				L"999",
				3,
				&sz
			);
			int width = sz.cx + 4;
			SetTextAlign(
				hdc,
				TA_TOP | TA_RIGHT
			);
			int TextY = 0;
			int LineAt = this->DirectSendMessage(EM_GETFIRSTVISIBLELINE, 0, 0) + 1;
			SetTextColor(hdc, m_LineNumberColor);
			wstring t;
			while (TextY < rcCli.bottom)
			{
				t = to_wstring(LineAt);
				TextOut(hdc, width - 2, TextY, t.c_str(), t.length());
				TextY += m_TextHeight;
				LineAt++;
			}


			//RECT wRect; GetWindowRect(hwnd, &wRect);
			//HWND hOtherhWnd = GetParent(hwnd);
			//HDC OtherDC = GetWindowDC(hOtherhWnd);
			//MapWindowPoints(HWND_DESKTOP, hOtherhWnd, (LPPOINT)&wRect, 2);

			RECT wRect = GetClientRect();
			HWND hOtherhWnd = hwnd;
			HDC OtherDC = GetDC(hOtherhWnd);



			BitBlt(
				OtherDC,//_In_ HDC   hdcDest,
				wRect.left - width - 6,//_In_ int   nXDest,
				wRect.top + 1,//_In_ int   nYDest,
				width, //_In_ int   nWidth,
				nHeight, //_In_ int   nHeight,
				hdc,//_In_ HDC   hdcSrc,
				0,//_In_ int   nXSrc,
				0,//_In_ int   nYSrc,
				SRCCOPY//_In_ DWORD dwRop
			);
			ReleaseDC(hOtherhWnd, OtherDC);
		}
		//clean my mem dc
		DeleteObject((HBITMAP)SelectObject(hdc, oldbm));
		DeleteDC(hdc);

	}
	return 0L;
}
extern ZLoadedMap LoadedLanguage;
extern string SelectedLanguage;
LRESULT HookedCtrl::DoWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (m_Language != NULL)
	{
		if (0)//(InPopUpMenu)
		{
			if (message == WM_PAINT || message == WM_COMMAND)
			{

			}
			else
			{
				return 0l;
			}
		}

		switch (message)
		{
			case WM_CONTEXTMENU:
			{
				if (InPopUpMenu) return 0l;
				POINT point;
				GetCursorPos(&point);

				//point.x = LOWORD(lParam);
				//point.y = HIWORD(lParam);

				HMENU hLangMenu = CreatePopupMenu();

				TCHARString ItemName;



				HMENU hMenu = CreatePopupMenu();
				//ClientToScreen(hwnd, &point);

				if(UndoStack.empty())
					AppendMenu(hMenu, MF_ENABLED | MF_DISABLED | MF_GRAYED, RM_UNDO, L"Undo\tCtrl+Z");
				else
					AppendMenu(hMenu, MF_ENABLED, RM_UNDO, L"Undo\tCtrl+Z");
				if (RedoStack.empty())
					AppendMenu(hMenu, MF_ENABLED | MF_DISABLED | MF_GRAYED, RM_REDO, L"Redo\tCtrl+Y");
				else
					AppendMenu(hMenu, MF_ENABLED, RM_REDO, L"Redo\tCtrl+Y");
				

				AppendMenu(hMenu, MF_MENUBREAK, 0, 0);
				AppendMenu(hMenu, MF_ENABLED, RM_CUT, L"Cut\tCtrl+X");
				AppendMenu(hMenu, MF_ENABLED, RM_COPY, L"&Copy\tCtrl+C");
				AppendMenu(hMenu, MF_ENABLED, RM_PASTE, L"&Paste\tCtrl+V");
				AppendMenu(hMenu, MF_ENABLED, RM_SELECTALL, L"&Select All\tCtrl+A");
				AppendMenu(hMenu, MF_MENUBREAK, 0, 0);
				if (m_Language != NULL)
				{
					AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hLangMenu, L"Syntax Hilighting");
					if (m_SyntaxHighlighting) 
						AppendMenu(hLangMenu, MF_ENABLED | MF_CHECKED, RE_SYNTAXENABLED, L"Enabled");
					else
						AppendMenu(hLangMenu, MF_ENABLED, RE_SYNTAXENABLED, L"Enabled");

				
					if (m_DoLineNumbers)
						AppendMenu(hLangMenu, MF_ENABLED | MF_CHECKED, RM_LINENUMBER, L"Line Numbers");
					else
						AppendMenu(hLangMenu, MF_ENABLED, RM_LINENUMBER, L"Line Numbers");
					

					AppendMenu(hLangMenu, MF_MENUBREAK, 0, 0);
					TCHARString SelectedLang = m_Language->m_LanguageName;
					int MnuCt = RE_SYNTAXENABLED;
					for (ZLoadedMap::const_iterator it = LoadedLanguage.begin(); it != LoadedLanguage.end(); ++it)
					{
						MnuCt++;
						ItemName = it->first;
						if (ItemName == SelectedLang)
							AppendMenu(hLangMenu, MF_ENABLED | MF_CHECKED, MnuCt, ItemName.c_str());
						else
							AppendMenu(hLangMenu, MF_ENABLED, MnuCt, ItemName.c_str());
					}
					AppendMenu(hMenu, MF_MENUBREAK, 0, 0);
				}
				AppendMenu(hMenu, MF_ENABLED, RM_MATCHINGBRACKET, L"&Matching Bracket\t(Shift+) Ctrl+]");
				AppendMenu(hMenu, MF_ENABLED, RM_COMMENT, L"&Block Comment\tCtrl+K");
				AppendMenu(hMenu, MF_ENABLED, RM_UNCOMMENT, L"&Block Uncomment\tCtrl+U");
				AppendMenu(hMenu, MF_ENABLED, RM_INDENT, L"&Indent\t(Shift+) Selection+Tab");
				AppendMenu(hMenu, MF_MENUBREAK, 0, 0);
				AppendMenu(hMenu, MF_ENABLED, RM_FIND, L"&Find Next\t(Shift+) (Ctrl+) F3");

				SetFocus(NULL);
				ReleaseCapture();
				InPopUpMenu = true;
				TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, point.x, point.y, 0, hwnd, NULL);
				InPopUpMenu = false;
				SetFocus(hwnd);
				SetActiveWindow(hwnd);

				DestroyMenu(hMenu);
				DestroyMenu(hLangMenu);
				return 0;;
			}

			case WM_COMMAND:
			{
				if (LOWORD(wParam) == RM_COPY) {
					this->EditCopy();
				}
				else if (LOWORD(wParam) == RM_PASTE) {
					this->EditPaste();
				}
				else if (LOWORD(wParam) == RM_CUT) {
					this->EditCut();
				}
				else if (LOWORD(wParam) == VK_DELETE && KEY_STATE_DOWN(VK_SHIFT)) {
					this->EditCut();
				}
				else if (LOWORD(wParam) == RM_SELECTALL) {
					this->EditSelectAll();
				}
				else if (LOWORD(wParam) == RM_UNDO) {
					this->EditUndo();
				}
				else if (LOWORD(wParam) == RM_REDO) {
					this->EditRedo();
				}
				else if (LOWORD(wParam) == RM_COMMENT) {
					this->EditComment();
				}
				else if (LOWORD(wParam) == RM_UNCOMMENT) {
					this->EditComment(1);
				}
				else if (LOWORD(wParam) == RM_MATCHINGBRACKET) {
					this->EditMatchBrace(KEY_STATE_DOWN(VK_SHIFT));
				}
				else if (LOWORD(wParam) == RM_FIND) {
					this->EditFind(KEY_STATE_DOWN(VK_CONTROL), KEY_STATE_DOWN(VK_SHIFT));
				}
				else if (LOWORD(wParam) == RM_INDENT) {
					this->EditIndent(KEY_STATE_DOWN(VK_SHIFT));
				}
				else if (LOWORD(wParam) == RM_LINENUMBER) {
					m_DoLineNumbers = !m_DoLineNumbers;
					RedrawWindow(GetParent(m_hWnd), NULL, NULL, RDW_ERASE | RDW_UPDATENOW | RDW_ERASENOW | RDW_INVALIDATE);
				}
				else if (LOWORD(wParam) == RE_SYNTAXENABLED) {
					this->ToggleSyntax();
				}
				else if (LOWORD(wParam) > RE_SYNTAXENABLED) {
					int findVal = LOWORD(wParam) - RE_SYNTAXENABLED;
					int numAt = 0;
					TCHARString ItemName;
					for (ZLoadedMap::const_iterator it = LoadedLanguage.begin(); it != LoadedLanguage.end(); ++it)
					{
						numAt++;
						
						if (findVal == numAt)
						{
							ItemName = it->first;
							m_Language = LoadedLanguage[ItemName];
							SelectedLanguage = TCHARStringToString(ItemName);
							break;
						}
						
					}
				}

				this->ScrollToCaret();
				RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
				return 0l;
			}
			case WM_ERASEBKGND:
			{
				HDC hdc = (HDC)wParam;
				RECT rc;
				GetWindowRect(hwnd,&rc);
				rc.right -= rc.left;
				rc.right -= GetSystemMetrics(SM_CXVSCROLL);
				rc.right -= 4;
				rc.bottom -= rc.top;
				rc.bottom -= 4;
				rc.left = 0; 
				rc.top = 0;
				
				//SetMapMode(hdc, MM_ANISOTROPIC);
				//SetWindowExtEx(hdc, 100, 100, NULL);
				//SetViewportExtEx(hdc, rc.right, rc.bottom, NULL);
				HBRUSH BackBrush = CreateSolidBrush(m_Language->TEXTBACKCOLOR);
				FillRect(hdc, &rc, BackBrush);
				/*
				DeleteObject(BackBrush);
				for (i = 0; i < 13; i++)
				{
					x = (i * 40) % 100;
					y = ((i * 40) / 100) * 20;
					SetRect(&rc, x, y, x + 20, y + 20);
					FillRect(hdc, &rc, hbrGray);
				}*/
				return 1L;
				return CallWindowProc(m_origWndProc, hwnd, message, wParam, lParam);
			}
			case WM_PAINT: 
			{
				if (InPopUpMenu) 
					return 0l;  
				else
					return this->Do_WM_PAINT(hwnd, message, wParam, lParam);	
			}
			case EM_GETSEL:
			{
				if (wParam != NULL)
				{
					DWORD *t = (DWORD *)wParam;
					*t = m_SelStart;
				}
				if (lParam != NULL)
				{
					DWORD *t = (DWORD *)lParam;
					*t = m_SelEnd;
				}
				return MAKELPARAM((short)m_SelStart, (short)m_SelEnd);
				//EB_SET_SEL(m_SelStart, m_SelEnd);
				//return CallWindowProc(m_origWndProc, hwnd, message, wParam, lParam);
			}
			case EM_SETSEL:
			{
				//this->InvalidateSelectRegion();
				m_SelStart = LOWORD(lParam); m_SelEnd = HIWORD(lParam);
				//this->InvalidateSelectRegion();
				RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
				return CallWindowProc(m_origWndProc, hwnd, message, wParam, lParam);
			}
			case EM_REPLACESEL:
			{
				//this->InvalidateSelectRegion();
				LRESULT r = CallWindowProc(m_origWndProc, hwnd, message, wParam, lParam);
				this->CaretSetPosition(EB_SEL_END_FROM_LRESULT(EB_GET_SEL()));
				//this->InvalidateEditChar(m_SelStart);
				RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
				return r;
			}
			
			
			case WM_KEYDOWN:
			{
				if (InPopUpMenu) return 0l;
				
				if (wParam == VK_CONTROL || wParam == VK_SHIFT || wParam == VK_CAPITAL) return 0l;
				//SendMessage(this->m_hWnd, WM_SETREDRAW, FALSE, 0);
				//this->InvalidateSelectRegion();
				if (0)
				{
				}
				else if (wParam == 0x43 && KEY_STATE_DOWN(VK_CONTROL)) {
					this->EditCopy();
				}
				else if (wParam == VK_INSERT && KEY_STATE_DOWN(VK_CONTROL)) {
					this->EditCopy();
				}
				else if (wParam == 0x56 && KEY_STATE_DOWN(VK_CONTROL)) {
					this->EditPaste();
				}
				else if (wParam == VK_INSERT && KEY_STATE_DOWN(VK_SHIFT)) {
					this->EditPaste();
				}
				else if (wParam == 0x58 && KEY_STATE_DOWN(VK_CONTROL)) {
					this->EditCut();
				}
				else if (wParam == VK_DELETE && KEY_STATE_DOWN(VK_SHIFT)) {
					this->EditCut();
				}
				else if (wParam == 0x41 && KEY_STATE_DOWN(VK_CONTROL)) {
					this->EditSelectAll();
				}
				else if (wParam == 0x5A && KEY_STATE_DOWN(VK_CONTROL)) {
					this->EditUndo();
				}
				else if (wParam == 0x59 && KEY_STATE_DOWN(VK_CONTROL)) {
					this->EditRedo();
				}
				if (wParam == 0x4B && KEY_STATE_DOWN(VK_CONTROL)) {
					
					this->EditComment();
					RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
					return 0l;
				}
				else if (wParam == 0x55 && KEY_STATE_DOWN(VK_CONTROL)) {
					this->EditComment(1);
					RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
					return 0l;
				}
				else if (wParam == 0xDD && KEY_STATE_DOWN(VK_CONTROL)) {
					this->EditMatchBrace(KEY_STATE_DOWN(VK_SHIFT));
				}
				else if (wParam == VK_F3) {
					this->EditFind(KEY_STATE_DOWN(VK_CONTROL), KEY_STATE_DOWN(VK_SHIFT));
				}
				else if (wParam == VK_LEFT) {
					this->CaretMoveChar(-1, KEY_STATE_DOWN(VK_CONTROL),  KEY_STATE_DOWN(VK_SHIFT));
				}
				else if (wParam == VK_RIGHT) {
					this->CaretMoveChar(1, KEY_STATE_DOWN(VK_CONTROL), KEY_STATE_DOWN(VK_SHIFT));
				}
				else if (wParam == VK_UP) {
					this->CaretMoveLine(-1, KEY_STATE_DOWN(VK_SHIFT));
					if (KEY_STATE_DOWN(VK_CONTROL)) DirectSendMessage(WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
				}
				else if (wParam == VK_DOWN) {
					this->CaretMoveLine(1, KEY_STATE_DOWN(VK_SHIFT));
					if (KEY_STATE_DOWN(VK_CONTROL)) DirectSendMessage(WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
				}
				else if (wParam == VK_END) {
					this->CaretMoveEndOfLine(KEY_STATE_DOWN(VK_CONTROL), KEY_STATE_DOWN(VK_SHIFT));
				}
				else if (wParam == VK_HOME) {
					this->CaretMoveStartOfLine(KEY_STATE_DOWN(VK_CONTROL), KEY_STATE_DOWN(VK_SHIFT));
				}
				else if (wParam == VK_PRIOR) {
					this->PageScroll(-1, KEY_STATE_DOWN(VK_CONTROL), KEY_STATE_DOWN(VK_SHIFT));
				}
				else if (wParam == VK_NEXT) {
					this->PageScroll(1, KEY_STATE_DOWN(VK_CONTROL), KEY_STATE_DOWN(VK_SHIFT));
				}
				else if (wParam == VK_DELETE) {
				
					if (m_SelStart == m_SelEnd)
					{

						if (KEY_STATE_DOWN(VK_CONTROL))
						{
							this->CaretMoveEndOfLine(0, 1);
							DirectSendMessage(EM_SETSEL, m_SelStart, m_SelEnd);
							AddUndoableContent(L"");
							//DirectSendMessage(EM_REPLACESEL, TRUE, (LPARAM)L"");
							//this->SetSelection(EB_SEL_START_FROM_LRESULT(EB_GET_SEL()), EB_SEL_END_FROM_LRESULT(EB_GET_SEL()));
							//RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);

						}
						else
						{
							int shift = 1;
							while (m_EBText[m_SelEnd + shift] == '\r' || m_EBText[m_SelEnd + shift] == '\n') shift++;
							DirectSendMessage(EM_SETSEL, m_SelEnd, min(m_SelEnd + min(2, shift), this->GetTextLenght()));
							//DirectSendMessage(EM_REPLACESEL, TRUE, (LPARAM)L"");
							AddUndoableContent(L"");
							//this->SetSelection(EB_SEL_START_FROM_LRESULT(EB_GET_SEL()), EB_SEL_END_FROM_LRESULT(EB_GET_SEL()));
							//RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);

						}

					}
					else
					{
						EB_SET_SEL(m_SelStart, m_SelEnd);
						AddUndoableContent(L"");
						//DirectSendMessage(EM_REPLACESEL, TRUE, (LPARAM)L"");
						//this->SetSelection(EB_SEL_START_FROM_LRESULT(EB_GET_SEL()), EB_SEL_END_FROM_LRESULT(EB_GET_SEL()));
						//RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
					}
				}
				else if (wParam == VK_RETURN) {
					//DirectSendMessage(EM_REPLACESEL, TRUE, (LPARAM)L"\r\n");
					AddUndoableContent(L"\r\n");
					//this->SetSelection(EB_SEL_START_FROM_LRESULT(EB_GET_SEL()), EB_SEL_END_FROM_LRESULT(EB_GET_SEL()));

				}
				else if (wParam == VK_BACK) 
				{
					if (m_SelStart == m_SelEnd) {
						if ((m_EBText[max(0,m_SelEnd - 1)] == '\t') || (m_EBText[max(0, m_SelEnd - 1)] == ' '))
						{
							this->DoBackTab();
						}
						else
						{
							int shift = -1;
							while (m_EBText[m_SelEnd + shift] == '\r' || m_EBText[m_SelEnd + shift] == '\n') shift--;
							DirectSendMessage(EM_SETSEL, m_SelEnd, max(0, m_SelEnd + max(-2, shift)));
							//DirectSendMessage(EM_REPLACESEL, TRUE, (LPARAM)L"");
							AddUndoableContent(L"");
							//this->SetSelection(EB_SEL_START_FROM_LRESULT(EB_GET_SEL()), EB_SEL_END_FROM_LRESULT(EB_GET_SEL()));
							//RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
						}
					}
					else
					{
						EB_SET_SEL(m_SelStart, m_SelEnd);
						AddUndoableContent(L"");
						//DirectSendMessage(EM_REPLACESEL, TRUE, (LPARAM)L"");
						//this->SetSelection(EB_SEL_START_FROM_LRESULT(EB_GET_SEL()), EB_SEL_END_FROM_LRESULT(EB_GET_SEL()));
						//RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
					}
				}
				else if (wParam == VK_TAB) 
				{
					if (m_SelStart != m_SelEnd)
					{
						this->EditIndent(KEY_STATE_DOWN(VK_SHIFT));
						EB_SET_SEL(m_SelStart, m_SelEnd);
					}
					else
					{
						if (KEY_STATE_DOWN(VK_SHIFT))
						{
							this->DoBackTab();
						}
						else
						{
							//DirectSendMessage(EM_REPLACESEL, TRUE, (LPARAM)L"    ");
							AddUndoableContent(L"    ");
						}
						//this->SetSelection(EB_SEL_START_FROM_LRESULT(EB_GET_SEL()), EB_SEL_END_FROM_LRESULT(EB_GET_SEL()));
					}
				}
				else
				{
					//SendMessage(this->m_hWnd, WM_SETREDRAW, TRUE, 0);
					return 0l;
				}
				//SendMessage(this->m_hWnd, WM_SETREDRAW, TRUE, 0);
				//this->InvalidateSelectRegion();
				this->ScrollToCaret();
				RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
				return 0l;
			}
			case WM_CHAR:
			{
				if (InPopUpMenu) return 0l;
				SendMessage(this->m_hWnd, WM_SETREDRAW, FALSE, 0);
				//this->InvalidateSelectRegion();

				TCHARString InsertText = L"";
				bool Selection = m_SelStart != m_SelEnd;

				
				if (wParam >= 32) {
					DirectSendMessage(EM_SETSEL, m_SelStart, m_SelEnd);
					TCHAR CharSent[] = { (TCHAR)wParam, 0 };
					InsertText = CharSent;
					this->AddUndoableContent(InsertText);
					// DirectSendMessage(EM_REPLACESEL, TRUE, (LPARAM)InsertText.c_str());
					//this->SetSelection(EB_SEL_START_FROM_LRESULT(EB_GET_SEL()), EB_SEL_END_FROM_LRESULT(EB_GET_SEL()));

				}
				SendMessage(this->m_hWnd, WM_SETREDRAW, TRUE, 0);

				this->SetSelection(EB_SEL_START_FROM_LRESULT(EB_GET_SEL()), EB_SEL_END_FROM_LRESULT(EB_GET_SEL()));
				//this->CaretSetPosition(m_SelEnd);
				//this->InvalidateEditChar(m_SelStart);
				RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
				
				this->ScrollToCaret();
				return 0l;
			}
			case WM_LBUTTONDBLCLK:
			{
				m_WordSelectDrag = true;
				RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
			} //fall though to lbuttondown
			case WM_LBUTTONDOWN:
			{
				if (InPopUpMenu) return 0l;
				m_scrollHDirection = 0;
				m_scrollVDirection = 0;
				SetFocus(hwnd);
				SetCapture(hwnd);
				//this->InvalidateSelectRegion();
				CharFromPos charFromPos = this->GetCharFromPos(lParam);
				if (KEY_STATE_DOWN(VK_SHIFT)) {
					this->SetSelection(m_SelStart, charFromPos.CharPos);
				}
				else {
					this->CaretSetPosition(charFromPos.CharPos);
				}
				if (m_TimerID != -1)
				{
					KillTimer(hwnd, m_TimerID);
					m_TimerID = -1;
				}
				m_TimerID = SetTimer(hwnd, 6502, 100, 0);
				if (m_WordSelectDrag)
				{
					this->CaretMoveChar(-1, 1, 0);
					this->CaretMoveChar(1, 1, 1);
				}
				//this->InvalidateSelectRegion();
				RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
				
				return 0L;
			}
			
			case WM_LBUTTONUP:
			{
				if (InPopUpMenu) return 0l;
				m_scrollHDirection = 0;
				m_scrollVDirection = 0;
				if (m_TimerID != -1)
				{
					KillTimer(hwnd, m_TimerID);
					m_TimerID = -1;
				}
				ReleaseCapture();
				m_WordSelectDrag = false;
				return 0l;
			}
			case WM_TIMER:
			{
				if (wParam == m_TimerID)
				{
					if (InPopUpMenu) return 0l;

					int hscroll = 0;
					RECT rect = this->GetClientRect();
					if (m_scrollHDirection < 0) {
						hscroll = -1;
					}
					else if (m_scrollHDirection > 0) {
						hscroll = 1;
					}
					if (m_scrollVDirection < 0) {
						repeat(abs(m_scrollVDirection)) {
							DirectSendMessage(WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
						}
					}
					else if (m_scrollVDirection > 0)
					{
						repeat(abs(m_scrollVDirection)) {
							DirectSendMessage(WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
						}
					}
					if (m_scrollHDirection != 0 || m_scrollVDirection != 0)
					{
						CharFromPos charFromPos = this->GetCharFromPos(m_ScrollMouseX, m_ScrollMouseY);
						if (abs(hscroll)) {
							int OldPos = charFromPos.CharPos;

							repeat(abs(m_scrollHDirection)) {
								charFromPos.CharPos += hscroll;
								if (GetLineFromPos(charFromPos.CharPos) != GetLineFromPos(OldPos))
								{
									charFromPos.CharPos -= hscroll;
								}
							}
							charFromPos.CharPos = CAP_0_64k(charFromPos.CharPos);

						}
						//invalidate the selection for redrawing it
						int OldSelEnd = m_SelEnd;
						this->SetSelection(m_SelStart, charFromPos.CharPos);
						if (OldSelEnd != charFromPos.CharPos)
						{
							//InvalidateSelectRegion(OldSelEnd, charFromPos.CharPos);
							RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
						}
						this->ScrollToCaret();
					}
					return 0l;
				}
				else
				{
					return CallWindowProc(m_origWndProc, m_hWnd, message, wParam, lParam);
				}
			}
			case WM_MOUSEMOVE:
			{
				if (InPopUpMenu) return 0l;
				if ((wParam & MK_LBUTTON) == MK_LBUTTON)
				{
					short x = LOWORD(lParam);
					short y = HIWORD(lParam);

					RECT rect = this->GetClientRect();

					m_scrollHDirection = 0;
					m_scrollVDirection = 0;
					if (x < rect.left)
					{
						m_scrollHDirection = (x - rect.left);
					}
					else if (x > rect.right)
					{
						m_scrollHDirection = (x - rect.right);
					}
					if (y < rect.top)
					{
						m_scrollVDirection = (y - rect.top);
					}
					else if (y > rect.bottom)
					{
						m_scrollVDirection = (y - rect.bottom);
					}

					m_ScrollMouseX = max(rect.left, min(x, rect.right));
					m_ScrollMouseY = max(rect.top, min(y, rect.bottom));

					if (m_scrollHDirection == 0 && m_scrollVDirection == 0)
					{
						CharFromPos charFromPos = this->GetCharFromPos(m_ScrollMouseX, m_ScrollMouseY);
						//invalidate the selection for redrawing it
						int OldSelEnd = m_SelEnd;
						if (m_WordSelectDrag)
						{
							m_SelEnd = charFromPos.CharPos;
							int OldSelStart = m_SelStart;
							if (m_SelEnd < m_SelStart)
							{
								this->CaretMoveChar(1, 1, 0);
								this->CaretMoveChar(-1, 1, 0);
							}
							else
							{
								this->CaretMoveChar(-1, 1, 0);
								this->CaretMoveChar(1, 1, 0);
							}
							charFromPos.CharPos = m_SelEnd;
							m_SelStart = OldSelStart;
							

						}
						this->SetSelection(m_SelStart, charFromPos.CharPos);

						if (OldSelEnd != charFromPos.CharPos)
						{
							//InvalidateSelectRegion(OldSelEnd, charFromPos.CharPos);
							RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
						}
						this->ScrollToCaret();
					}
					m_scrollHDirection /= 20;
					m_scrollVDirection /= 10;


				}
				return 0L;
			}
			case WM_IME_NOTIFY:
			{
				return 0L;

				//case EM_SCROLL:
				//	CallWindowProc(m_origWndProc, m_hWnd, message, wParam, lParam);
				//	RedrawWindow(hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);
				//	return 0L;
			}
			case WM_SIZE:
			{
				RedrawWindow(GetParent(hwnd) , NULL, NULL, RDW_INVALIDATE);
				return CallWindowProc(m_origWndProc, hwnd, message, wParam, lParam);
			}

		}
	}
	return CallWindowProc(m_origWndProc, m_hWnd, message, wParam, lParam);
}