#include "stdafx.h"
#include "HookedCtrl.h"

const int NumFontItems = 36;
//                      0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30   31   32   33   34   35
int FontSizesPt[] =    {1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 34, 38, 42, 46, 50, 54, 60, 64, 68, 72,  76,  80,  86,  92,  96};
int FontSizesPx[] =    {1,  3,  4,  5,  7,  8,  9, 11, 12, 13, 15, 16, 19, 22, 24, 26, 29, 32, 35, 37, 40, 45, 51, 56, 61, 67, 72, 80, 85, 91, 96, 101, 107, 115, 123, 128};
HFONT FontHandles[] = { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,   0,   0,   0,   0 };
int FontAt = 11;

int HookedCtrl::GetFontPtFromHeight(int sz)
{
	int thesz = abs(sz);
	int besti = 0;
	int bestdist = 999999;
	for (int i = NumFontItems - 1; i >= 0; i--)
	{
		int dist = abs(FontSizesPx[i] - thesz);
		if (dist < bestdist)
		{
			besti = i;
			bestdist = dist;
		}
	}
	
	return FontSizesPt[besti];
}
int HookedCtrl::GetFontHeightFromPt(int sz)
{
	int thesz = abs(sz);
	int besti = 0;
	int bestdist = 999999;
	for (int i = NumFontItems - 1; i >= 0; i--)
	{
		int dist = abs(FontSizesPt[i] - thesz);
		if (dist < bestdist)
		{
			besti = i;
			bestdist = dist;
		}
	}

	return FontSizesPx[besti];
}
int HookedCtrl::GetFontSize(int pt)
{
	HFONT hFont = (HFONT)DirectSend(WM_GETFONT, 0, 0);
	LOGFONT fontAttributes = { 0 };
	GetObject(hFont, sizeof(fontAttributes), &fontAttributes);
	if (pt) return GetFontPtFromHeight(abs(fontAttributes.lfHeight));
	return abs(fontAttributes.lfHeight);
}
int HookedCtrl::SetFontSize(int pt, int sz)
{
	int Height = 0;
	if (pt)
	{
		Height = GetFontHeightFromPt(sz);
	}
	else
	{
		int ValPt = GetFontPtFromHeight(sz);
		Height = GetFontHeightFromPt(ValPt);
	}
	
	//Get the font
	HFONT hFont = (HFONT)DirectSend(WM_GETFONT, 0, 0);
	LOGFONT fontAttributes = { 0 };
	GetObject(hFont, sizeof(fontAttributes), &fontAttributes);
	//fontAttributes.lfWeight = FW_BOLD;
	fontAttributes.lfHeight = -Height;
	
	HFONT NewFont = CreateFontIndirect(&fontAttributes);
	if (NewFont)
	{
		//remember the default font, this new font is managed by the control
		HFONT OldEBSFont = m_OEBS.font;
		SendMessage(m_hWnd, WM_SETFONT, (WPARAM)NewFont, 1);
		//restore the old font
		m_OEBS.font = OldEBSFont;

	}
	if (pt) return GetFontPtFromHeight(Height);
	else return Height;
}
int HookedCtrl::ZoomFont(int direction, int rettype = 0)
{
	LOGFONT fontAttributes = { 0 };
	HFONT hFont = (HFONT)DirectSend(WM_GETFONT, 0, 0);
	GetObject(hFont, sizeof(fontAttributes), &fontAttributes);
	//fontAttributes.lfWeight = FW_BOLD;
	long Height = abs(fontAttributes.lfHeight);
	if (direction == 1)
	{
		for (int i = 0; i < NumFontItems; i++)
		{
			if (FontSizesPx[i] > Height || i == NumFontItems-1)
			{
				Height = FontSizesPx[i];
				break;
			}
		}
	}
	else
	{
		for (int i = NumFontItems-1; i >=0 ; i--)
		{
			if (FontSizesPx[i] < Height || i==0)
			{
				Height = FontSizesPx[i];
				break;
			}
		}
	}
	int ret = SetFontSize(0, Height);
	if (rettype) return GetFontPtFromHeight(ret);
	else return ret;
}

#define VK_A 0x41
#define VK_C 0x43
#define VK_F 0x46
#define VK_K 0x4B
#define VK_R 0x52
#define VK_V 0x56
#define VK_X 0x58
#define VK_Y 0x59
#define VK_Z 0x5A
#define VK_SQUAREBRACKET 0xDD

LRESULT HookedCtrl::DoWordList()
{
	//Find the word...
	HLOCAL hMem = (HLOCAL)DirectSend(EM_GETHANDLE, 0, 0);
	TCHAR *Text = NULL;
	if (hMem)
	{
		TCHAR *Text = (TCHAR*)LocalLock(hMem);
		if (Text)
		{
			//...at the cursor
			LONG CaretPos = m_SelEnd;
			LONG selstart = m_SelEnd;
			LONG selend = m_SelEnd;
			
			

			while (CaretPos-1 >=0 && !m_Language->WORDLISTSPLIT.Valid(Text[CaretPos-1]))
			{
				CaretPos--;
			}
			selstart = CaretPos;
			m_WLAltWordStart = CaretPos;
			wstring Alternative = L"";
			while (!m_Language->WORDLISTSPLIT.Valid(Text[CaretPos]))
			{
				if (Text[CaretPos] == '.')
				{
					Alternative = Text[CaretPos];
					m_WLAltWordStart = CaretPos;
				}
				else if (Alternative.length())
				{
					Alternative += Text[CaretPos];
				}
				CaretPos++;
			}
			selend = CaretPos;
			wstring text = GetSelText(selstart, selend);
			m_WLWordStart = selstart;
			m_WLWordEnd = selend;
			

			LRESULT Pos = SafePosFromChar(selstart, 0, 1);
			short PosX = LOWORD(Pos);
			short PosY = HIWORD(Pos);
			m_WordListDlg.SetWordList(m_WordList);
			m_WordListDlg.SetDispatch(m_hWnd, 0);
			RECT r; GetClientRect(m_hWnd, &r);

			short InvertedOffset = (short)r.bottom - PosY;
			if (m_WordListDlg.m_hDlg == NULL) //for some reason it wont show if it was never shown before, so call twice if the window handle is null (nevere created), too tired to figure it out
				m_WordListDlg.Show(SW_SHOWNA, PosX, PosY + (short)m_textMetrics.tmHeight, (short)m_textMetrics.tmAveCharWidth * 24, (short)m_textMetrics.tmHeight * 8, InvertedOffset);
			m_WordListDlg.Show(SW_SHOWNA, PosX, PosY + (short)m_textMetrics.tmHeight, (short)m_textMetrics.tmAveCharWidth * 24, (short)m_textMetrics.tmHeight * 8, InvertedOffset);
			m_WordListDlg.RefreshListContent(text.c_str(), 1, Alternative);

			
			LocalUnlock(hMem);
		}
	}

	return 0l;
	
}
LRESULT HookedCtrl::DoWM_KEYDOWN(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    //What key?
	switch (wParam)
	{
	case VK_LEFT: return CaretMoveChar(-1, KEY_STATE_DOWN(VK_CONTROL), KEY_STATE_DOWN(VK_SHIFT));
	case VK_RIGHT: return CaretMoveChar(1, KEY_STATE_DOWN(VK_CONTROL), KEY_STATE_DOWN(VK_SHIFT));
	case VK_UP: 
	{
		if (m_WordListDlg.IsVisible())
		{
			
			SetActiveWindow(m_WordListDlg.m_hDlg);
			HWND hList = GetDlgItem(m_WordListDlg.m_hDlg, IDC_WORDLIST);
			SetFocus(hList);
			PostMessage(hList, message, wParam, lParam);
			return 0l;
		}
		else
		{
			return CaretMoveLine(-1, KEY_STATE_DOWN(VK_CONTROL), KEY_STATE_DOWN(VK_SHIFT));
		}
		break;
	}
	case VK_DOWN: {
		if (m_WordListDlg.IsVisible())
		{
			
			SetActiveWindow(m_WordListDlg.m_hDlg);
			HWND hList = GetDlgItem(m_WordListDlg.m_hDlg, IDC_WORDLIST);
			SetFocus(hList);
			PostMessage(hList, message, wParam, lParam);
			return 0l;
		}
		else
		{
			return CaretMoveLine(1, KEY_STATE_DOWN(VK_CONTROL), KEY_STATE_DOWN(VK_SHIFT));
		}
		break;
	}
	case VK_RETURN:
	{
		if (m_WordListDlg.IsVisible())
		{
			SetActiveWindow(m_WordListDlg.m_hDlg);
			HWND hList = GetDlgItem(m_WordListDlg.m_hDlg, IDC_WORDLIST);
			SetFocus(hList);
			PostMessage(hList, message, wParam, lParam);
			return 0l;
		}
		break;
	}
	case VK_HOME: return CaretMoveEndOfLine(-1, KEY_STATE_DOWN(VK_CONTROL), KEY_STATE_DOWN(VK_SHIFT));
	case VK_END: return CaretMoveEndOfLine(1, KEY_STATE_DOWN(VK_CONTROL), KEY_STATE_DOWN(VK_SHIFT));
	case VK_PRIOR: return PageScroll(-1, KEY_STATE_DOWN(VK_CONTROL), KEY_STATE_DOWN(VK_SHIFT));
	case VK_NEXT: return PageScroll(1, KEY_STATE_DOWN(VK_CONTROL), KEY_STATE_DOWN(VK_SHIFT));
	case VK_DELETE: {
		if(KEY_STATE_DOWN(VK_SHIFT) && KEY_STATE_DOWN(VK_CONTROL) == 0)
			return DoCutKey(1);
		else
			return DeleteChar(1, KEY_STATE_DOWN(VK_CONTROL), message, wParam, lParam);
	}
	case VK_INSERT: {
		if (KEY_STATE_DOWN(VK_SHIFT) == 0 && KEY_STATE_DOWN(VK_CONTROL))
			return DoCopyKey(1);
		else if (KEY_STATE_DOWN(VK_SHIFT) && KEY_STATE_DOWN(VK_CONTROL) == 0)
			return DoPasteKey(1); 
		else
			return DoInsertKey();
	}
	case VK_BACK: return DeleteChar(-1, KEY_STATE_DOWN(VK_CONTROL), WM_CHAR, wParam, lParam);
	case VK_TAB: return DoTabChar(KEY_STATE_DOWN(VK_SHIFT), WM_CHAR, wParam, lParam);
	case VK_K: return DoBlockCommentKey(KEY_STATE_DOWN(VK_CONTROL), KEY_STATE_DOWN(VK_SHIFT));
	case VK_Z: return DoUndoKey(KEY_STATE_DOWN(VK_CONTROL));
	case VK_Y: return DoRedoKey(KEY_STATE_DOWN(VK_CONTROL));
	case VK_C: return DoCopyKey(KEY_STATE_DOWN(VK_CONTROL));
	case VK_V: return DoPasteKey(KEY_STATE_DOWN(VK_CONTROL));
	case VK_X: return DoCutKey(KEY_STATE_DOWN(VK_CONTROL));
	case VK_R:
	case VK_F:
	{
		if (KEY_STATE_DOWN(VK_CONTROL) == 0) return 1L;
		m_SearchDlg.Hide();
		m_SearchDlg.Show();
		
		m_WordListDlg.Hide();
		m_WordListDlg.Show();

		PostMessage(m_SearchDlg.m_hDlg, UM_INTERNAL_DOSEARCH, KEY_STATE_DOWN(VK_R), 0);
		
		break;
	}
	case VK_F3: 
	{
		wstring t;
		if(m_SearchDlg.IsVisible() || m_SelStart == m_SelEnd)
			t = m_SearchDlg.GetSearchText();
		
		
		SearchText((KEY_STATE_DOWN(VK_SHIFT)) ? -1 : 1,
			(m_SearchDlg.m_SearchCaseBut.State),
			(m_SearchDlg.m_SearchWordBut.State),
			t);
		m_SearchDlg.Show(SW_SHOWNA);
		break;
		//return SendMessage(hWnd, UM_SEARCHSEL, KEY_STATE_DOWN(VK_SHIFT), (m_SearchDlg.m_SearchCaseBut.State) | (m_SearchDlg.m_SearchWordBut.State * 2));
	}
	case VK_A: 
	{
		if (KEY_STATE_DOWN(VK_CONTROL) == 0) return 1L;
		SendMessage(hWnd, EM_SETSEL, 0, -1); return 0L;
	}
	case VK_ESCAPE:
	{
		if (m_WordListDlg.IsVisible())
		{
			m_WordListDlg.Hide();
		}
		else if (m_SearchDlg.IsVisible())
		{
			m_SearchDlg.Hide();
		}
		else
		{
			SendMessage(hWnd, EM_SETSEL, -1, 0);
		}
		return 0l;
	}
	case VK_SQUAREBRACKET:
	{
		if (KEY_STATE_DOWN(VK_CONTROL) == 0) return 1L;
		SendMessage(hWnd, UM_MATCHBRACKET, 0, -1); 
		return 0L;
	}
	case VK_SPACE:
	{
		if (KEY_STATE_DOWN(VK_CONTROL)) return DoWordList();
		return 0L;
	}
	}
	return 0l;
}
LRESULT HookedCtrl::DoWM_USER(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//User messages used to set and get setting and access feature for the control. 
	//The control sends these messages to itself as well when using these exposed features
	switch (message)
	{

		case UM_BLOCKINDENT: return BlockIndent(wParam);
		case UM_BLOCKCOMMENT: return BlockComment(wParam);
		case UM_REDO: return EditRedo();
		case UM_CANREDO: return CanRedo();
		case UM_SETINSERTMODE: return SetInsertMode(wParam);
		case UM_GETINSERTMODE: return GetInsertMode();
		case UM_CANSEARCHSEL: return CanSearchText();
		case UM_SEARCHSEL:	return SearchText((wParam==1) ? 1 : -1, 
								((lParam & UMSS_CS) == UMSS_CS) ? 1 : 0,
								((lParam & UMSS_WW) == UMSS_WW) ? 1 : 0);
		case UM_GETLINEMARGIN: return GetLineMargin();
		case UM_SETLINEMARGIN: return SetLineMargin(wParam);
		case UM_CHARFROMPOS64k: return SafeCharFromPos((short)wParam, (short)lParam);
		case UM_MATCHBRACKET: return MatchBracket(KEY_STATE_DOWN(VK_SHIFT));
		case UM_SETNOHIDESEL: return DoUM_SETNOHIDESEL(wParam);
		case UM_GETNOHIDESEL: return (GetWindowLongPtr(m_hWnd, GWL_STYLE) & ES_NOHIDESEL) == ES_NOHIDESEL;
		case UM_INTERNAL_GETEBHOOK: return (LRESULT)this;
		case UM_SETFONTSIZE: return (LRESULT)SetFontSize((int)wParam, (int)lParam);
		case UM_GETFONTSIZE: return GetFontSize((int)wParam);
		case UM_ZOOMFONT: return (LRESULT)ZoomFont((int)wParam, (int)lParam);
		case UM_PERFORMAUTOINDENT: return (LRESULT)DoIndent((int)wParam);
		case UM_SHOWSEARCHREPLACE: {
			if (lParam)
			{
				m_SearchDlg.Hide();
				m_SearchDlg.Show();
				PostMessage(m_SearchDlg.m_hDlg, UM_INTERNAL_DOSEARCH, wParam, 0);
			}
			else
			{
				m_SearchDlg.Hide();
			}
			return 1l;
		}
		case UM_GETSCROLLBAR:
		{
			if(wParam)
				return (GetWindowLongPtr(hWnd, GWL_STYLE) & (WS_HSCROLL | ES_AUTOHSCROLL)) == (WS_HSCROLL | ES_AUTOHSCROLL);
			else
				return (GetWindowLongPtr(hWnd, GWL_STYLE) & (WS_VSCROLL | ES_AUTOVSCROLL)) == (WS_VSCROLL | ES_AUTOVSCROLL);
		}
		case UM_SETSCROLLBAR:
		{
			
			if (wParam)
			{
				LONG_PTR s = GetWindowLongPtr(m_hWnd, GWL_STYLE);

				if (lParam)
					s |= (WS_HSCROLL | ES_AUTOHSCROLL);
				else
					s ^= (WS_HSCROLL | ES_AUTOHSCROLL);
				SetWindowLongPtr(m_hWnd, GWL_STYLE, (LONG_PTR)s);
				
			}
			else
			{
				LONG_PTR s = GetWindowLongPtr(m_hWnd, GWL_STYLE);

				if (lParam)
					s |= (WS_VSCROLL | ES_AUTOVSCROLL);
				else
					s ^= (WS_VSCROLL | ES_AUTOVSCROLL);
				SetWindowLongPtr(m_hWnd, GWL_STYLE, (LONG_PTR)s);
			}
			RedrawWindow(hWnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_ERASENOW);
			SetWindowPos(hWnd, 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE);
			return 0l;
		}
		case UM_GETWORDWRAP:
		{
			return (GetWindowLongPtr(hWnd, GWL_STYLE) & (ES_AUTOHSCROLL)) != (ES_AUTOHSCROLL);
		}
		case UM_SETWORDWRAP:
		{
			LONG_PTR s = GetWindowLongPtr(m_hWnd, GWL_STYLE);
			if (wParam==0)
				s |= (ES_AUTOHSCROLL);
			else
				s ^= (ES_AUTOHSCROLL);
			SetWindowLongPtr(m_hWnd, GWL_STYLE, (LONG_PTR)s);
			RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
			SetWindowPos(hWnd, 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE);
			return 0l;
		}
		case UM_INTERNAL_CANCELED:
		{
			SetFocus(m_hWnd); //refocus on the text box
			return 0l;
		}
		case UM_INTERNAL_WORDSELECTED:
		{
			//list box tells the dialog box that a selection was made. passing the list handle and the linked control idc
			HWND ListhWnd = (HWND)lParam;
			if (SendMessage(ListhWnd, LB_GETSELCOUNT, 0, 0))
			{

				LRESULT Sel = SendMessage(ListhWnd, LB_GETCURSEL, 0, 0);
				if (Sel != LB_ERR)
				{
					RedrawStop();
					m_ScrollCaretEnabled = false;
					
					wstringex *Word = (wstringex *)SendMessage(ListhWnd, LB_GETITEMDATA, Sel, 0);
					if (Word->length())
					{
						LONG selstart = (LONG)m_WLWordStart;
						if(Word->c_str()[0] == '.') selstart = (LONG)m_WLAltWordStart;
						SendMessage(hWnd, EM_SETSEL, selstart, (LONG)m_WLWordEnd);
						RedrawResume();
						m_ScrollCaretEnabled = true;


						AddUndoableContent(Word->c_str());
					}
				}
			}
			PostMessage(m_hWnd, UM_INTERNAL_REFOCUS, 0, 0);
			//SetFocus(m_hWnd); //refocus on the text box


			return 0l;
		}
		case UM_INTERNAL_REFOCUS:
		{
			SetFocus(m_hWnd);
			return 0l;
		}
	}
	//ok, possibly a edit box user message
	return CallOrigWindowProc(hWnd, message, wParam, lParam);
}

LRESULT HookedCtrl::DoWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		//caught functionality either add to the behaviour or are implmented to prevent flicker for the standard behaviour
	case WM_PAINT:		return DoWM_PAINT(hWnd, message, wParam, lParam);
	case WM_ERASEBKGND:	return DoWM_ERASEBKGND(hWnd, message, wParam, lParam);
	case WM_CHAR:		return DoWM_CHAR(hWnd, message, wParam, lParam);
	case WM_SIZE:		return DoWM_SIZE(hWnd, message, wParam, lParam);
	case WM_SIZING:		return DoWM_SIZE(hWnd, message, wParam, lParam);
	case EM_SETSEL:		return DoEM_SETSEL(hWnd, message, wParam, lParam);
	case EM_GETSEL:		return DoEM_GETSEL(hWnd, message, wParam, lParam);
	case WM_LBUTTONDOWN:return DoWM_LBUTTONDOWN(hWnd, message, wParam, lParam);
	case WM_LBUTTONUP:	return DoWM_LBUTTONUP(hWnd, message, wParam, lParam);
	case WM_MOUSEMOVE:	return DoWM_MOUSEMOVE(hWnd, message, wParam, lParam);
	case WM_TIMER:		return DoWM_TIMER(hWnd, message, wParam, lParam);
	case WM_KILLFOCUS:	return DoWM_KILLFOCUS(hWnd, message, wParam, lParam);
	case WM_KEYDOWN:	return DoWM_KEYDOWN(hWnd, message, wParam, lParam);
	case EM_SCROLLCARET:return DoEM_SCROLLCARET(hWnd, message, wParam, lParam);
	case WM_SETFOCUS:   return DoWM_SETFOCUS(hWnd, message, wParam, lParam);
	case EM_SCROLL: return DoWM_SCROLL(hWnd, message, wParam, lParam);
	case WM_VSCROLL: return DoWM_SCROLL(hWnd, message, wParam, lParam);
	case WM_HSCROLL: return DoWM_SCROLL(hWnd, message, wParam, lParam);
	case EM_LINESCROLL: return DoWM_SCROLL(hWnd, message, wParam, lParam);
	case WM_MOUSEWHEEL: return DoWM_MOUSEWHEEL(hWnd, message, wParam, lParam);

	case WM_SETCURSOR: return DoWM_SETCURSOR(hWnd, message, wParam, lParam);
	case WM_NCMOUSEMOVE: {this->SetTheCursor(m_Arrow); return 0l; }
	case WM_LBUTTONDBLCLK: return DoWM_LBUTTONDBLCLK(hWnd, message, wParam, lParam);
	case EM_UNDO:
	case WM_UNDO: return EditUndo();
	case EM_CANUNDO: return CanUndo();
	case WM_COPY:		return DoWM_COPY(hWnd, message, wParam, lParam);
	case WM_PASTE:		return DoWM_PASTE(hWnd, message, wParam, lParam);
	case WM_CUT:		return DoWM_CUT(hWnd, message, wParam, lParam);
	case WM_CLEAR: { AddUndoableContent(L""); return 0l; }
	case EM_SETREADONLY: return DoEM_SETREADONLY(hWnd, message, wParam, lParam);
	case EM_EMPTYUNDOBUFFER: return DoEM_EMPTYUNDOBUFFER(hWnd, message, wParam, lParam);
	case WM_SETTEXT: return DoWM_SETTEXT(hWnd, message, wParam, lParam);
	case EM_REPLACESEL: return DoEM_REPLACESEL(hWnd, message, wParam, lParam);
	case EM_FMTLINES: return 0l;
	case WM_SETFONT: return DoWM_SETFONT(hWnd, message, wParam, lParam);
	
	}
	//is it a user message used for specific features
	if (message >= WM_USER && message <= 0x7FFF) return DoWM_USER(hWnd, message, wParam, lParam);
	//default to original function
	return CallOrigWindowProc(hWnd, message, wParam, lParam);
};

LRESULT HookedCtrl::DoWM_CHAR(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (m_IsReadOnly) return 0l;
	//control + any key results in wierd character being caught. like ctlr + 1 sends char 0
	if (KEY_STATE_DOWN(VK_CONTROL)) return 0l;
	//Character input event, basically typing text
	if (wParam == 13)
	{

		if (!m_WordListDlg.IsVisible())
		{
			TCHAR CharSent[] = { 13, 10, 0 };
			this->AddUndoableContent(CharSent,0,0, message, wParam, lParam);
			
		}
	}
	else if (wParam >= 32) {
		TCHAR CharSent[] = { (TCHAR)wParam, 0 };
		if (m_InsertMode == 0 && m_SelEnd == m_SelStart)
		{
			m_ScrollCaretEnabled = false;
			RedrawStop();
			wstring t = GetSelText(m_SelStart, m_SelStart + 1);
			if (t.length())
			{
				if(t.c_str()[0] != '\r')
					SendMessage(hWnd, EM_SETSEL, m_SelStart, m_SelStart + 1);
			}
			RedrawResume();
			m_ScrollCaretEnabled = true;
		}
		this->AddUndoableContent(CharSent, 0, 0, message, wParam, lParam);
		
	}
	return 0l;
}
LRESULT HookedCtrl::DoWM_SIZE(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//On resize, prevent flicker. 
	
	RedrawStop();
	LRESULT r = CallOrigWindowProc(hWnd, message, wParam, lParam);
	RedrawResume();
	m_SearchDlg.UpdatePosition();
	m_WordListDlg.UpdatePosition();
	RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
	UpdateWindow(m_hWnd);
	
	return r;
	

};



LRESULT HookedCtrl::DoEM_SETSEL(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//On setsel, prevent flicker and remember the selection, so I always have a valid m_SelEnd, which will be the caret position
	RedrawStop();
	//this->InvalidateSelectRegion();
	LRESULT r = CallOrigWindowProc(hWnd, message, wParam, lParam);
	//remember
	auto oldstart = m_SelStart;
	auto oldend = m_SelEnd;
	//If the start is 0 and the end is –1, all the text in the edit control is selected.If the start is –1, any current selection is deselected.
	if ((LONG)wParam == -1 || (LONG)lParam == -1)
	{
		DirectSend(EM_GETSEL, (WPARAM)&m_SelStart, (LPARAM) &m_SelEnd);
	}
	else
	{
		m_SelStart = (LONG)wParam;
		m_SelEnd = (LONG)lParam;
	}
	//check if edit box is trying to swap sel end and sel start on us. so sel end stays the cursor
	if (oldstart == m_SelEnd && oldend == m_SelStart) swap(m_SelStart, m_SelEnd);
	RedrawResume();
	RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
	//UpdateWindow(m_hWnd);
	return r;
}
LRESULT HookedCtrl::DoEM_GETSEL(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//Initially used. May need it later, but now basically it's the original function
	return CallOrigWindowProc(hWnd, message, wParam, lParam);
}
LRESULT HookedCtrl::DoWM_KILLFOCUS(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//Kill focus, prevent flicker
	RedrawStop();
	CaretOn = false;
	LRESULT r = CallOrigWindowProc(hWnd, message, wParam, lParam);
	RedrawResume();
	RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
	//UpdateWindow(m_hWnd);
	return r;

}
LRESULT HookedCtrl::DoWM_CUT(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (m_IsReadOnly) return 0l;
	DirectSend(WM_COPY, 0, 0);
	this->AddUndoableContent(L"",true);
	return 0l;
}

LRESULT HookedCtrl::DoWM_COPY(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return DirectSend(WM_COPY, 0, 0);
}
LRESULT HookedCtrl::DoWM_PASTE(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (m_IsReadOnly) return 0l;
	//TODO, Complete implementation
	if (!IsClipboardFormatAvailable(CF_UNICODETEXT)) return 0l;
	if (!OpenClipboard(m_hWnd)) return 0l;
	HGLOBAL   hglb;
	TCHAR*    lptstr;
	
	hglb = GetClipboardData(CF_UNICODETEXT);
	if (hglb != NULL)
	{
		lptstr = (TCHAR*)GlobalLock(hglb);
		if (lptstr != NULL)
		{
			// Call the application-defined ReplaceSelection 
			// function to insert the text and repaint the 
			// window. 
			this->AddUndoableContent(lptstr, true);
			GlobalUnlock(hglb);
		}
	}
	CloseClipboard();
	
	return 0l;
	
}

LRESULT HookedCtrl::DoWM_LBUTTONDBLCLK(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	
	//short xPos = GET_X_LPARAM(lParam);
	//short yPos = GET_Y_LPARAM(lParam);

	if (m_CurrentCursor == m_IBeam)
	{
		SetFocus(hWnd);
		SetCapture(hWnd);
		TurnCaretOn();
		m_WordSelectMode = true;
		//Whole word double click. use the word select function for iitial word
		RedrawStop();
		m_ScrollCaretEnabled = false;
		//Dont grab the character, use the one we got on lbutton down
		//auto pos = SafeCharFromPos(LOWORD(lParam), HIWORD(lParam));
		//m_OldMouseChar = (LONG)pos;
		//m_SelEnd = pos;
		float mx = LOWORD(lParam);
		float cx = LOWORD(DirectSend(EM_POSFROMCHAR,m_SelEnd,0));
		//I spend a lot of tine in the little tweak that matter. 
		//choose the direction of the select word based on caret x vs mouse x, solving the select on the left bias
		if (mx >= cx)
		{
			CaretMoveWord(1, KEY_STATE_DOWN(VK_SHIFT));
			CaretMoveWord(-1, 1);
		}
		else
		{
			CaretMoveWord(-1, KEY_STATE_DOWN(VK_SHIFT));
			CaretMoveWord(1, 1);
		}
		if (KEY_STATE_DOWN(VK_SHIFT) == 0)
		{
			m_LineStartSelStart = m_SelStart;
			m_LineStartSelEnd = m_SelEnd;
		}
		RedrawResume();
		m_ScrollCaretEnabled = true;
		RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		UpdateWindow(m_hWnd);
	}

	return 0l;
}
LRESULT HookedCtrl::DoWM_LBUTTONDOWN(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	SetFocus(hWnd);
	SetCapture(hWnd);
	TurnCaretOn();
	//short xPos = GET_X_LPARAM(lParam);
	//short yPos = GET_Y_LPARAM(lParam);
	
	if (m_CurrentCursor == m_IBeam)
	{
		//Regular mode Select the clicked character for sel start and sel end
		RedrawStop();
		auto pos = SafeCharFromPos(LOWORD(lParam), HIWORD(lParam));
		m_OldMouseChar = (LONG)pos;
		bool DoWordListSelect = false;
		if (KEY_STATE_DOWN(VK_SHIFT))
			SendMessage(hWnd, EM_SETSEL, m_SelStart, pos);
		else
		{
			SendMessage(hWnd, EM_SETSEL, pos, pos);
			DoWordListSelect = true;
		}
		m_LineStartSelStart = m_SelEnd; //not a typo
		m_LineStartSelEnd = m_SelEnd;
		RedrawResume();
		SendMessage(m_hWnd, EM_SCROLLCARET, 0, 0);
		RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
		if (DoWordListSelect)
		{
			if (m_WordListDlg.IsVisible(true))
			{
				DoWordList();
			}
		}
		//UpdateWindow(m_hWnd);
	}
	else if (m_CurrentCursor == m_RightArrow)
	{
		//Line number clicked mode, select the line by using the Home/End code
		RedrawStop();
		m_ScrollCaretEnabled = false;
		auto pos = SafeCharFromPos(LOWORD(lParam), HIWORD(lParam));
		m_OldMouseChar = (LONG)pos;
		m_SelEnd = pos;
		CaretMoveEndOfLine(-1, 0, KEY_STATE_DOWN(VK_SHIFT));
		CaretMoveEndOfLine(1, 0, 1);
		if (KEY_STATE_DOWN(VK_SHIFT) == 0)
		{
			m_LineStartSelStart = m_SelStart;
			m_LineStartSelEnd = m_SelEnd;
		}
		RedrawResume();
		//scroll to left edge
		DirectSend(WM_HSCROLL, MAKEWPARAM(SB_LEFT, 0), 0);
		RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
		m_ScrollCaretEnabled =true;
		//UpdateWindow(m_hWnd);
	}
	
	return 0l;
}
LRESULT HookedCtrl::DoWM_LBUTTONUP(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//Done mode down action, release capture and refresh
	if (GetCapture() == hWnd) ReleaseCapture();
	RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
	m_WordSelectMode = false;
	//UpdateWindow(m_hWnd);
	return 0l;
}
LRESULT HookedCtrl::DoWM_SETCURSOR(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (m_SearchDlg.IsVisible()) return 0l;
	if (m_WordListDlg.IsVisible()) return 0l;
	//My set cursor, set the cursor to active one
	SetCursor(m_CurrentCursor);
	return 1L;
}
LRESULT HookedCtrl::DoWM_MOUSEMOVE(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//Mouse hover, set the mouse cursor

	if (GetCapture() != hWnd)
	{
		short x = LOWORD(lParam);
		short y = HIWORD(lParam);
		if(m_SearchDlg.IsPtOverDlg({x,y})) return 0l;
		if (m_WordListDlg.IsPtOverDlg({ x,y })) return 0l;
		
		
		RECT ClientRect; GetClientRect(hWnd, &ClientRect);
		if (x < m_Margin) //over line number margin, use my right arrow
			SetTheCursor(m_RightArrow);
		else if (PtInRect(&ClientRect, { x,y }))
			SetTheCursor(m_IBeam);
		else
			SetTheCursor(m_Arrow);
	}
	else if ((wParam & MK_LBUTTON) == MK_LBUTTON)
	{
		//TODO Left Right Scrolling
		//Dragging the mouse
		RECT ClientRect; GetClientRect(hWnd, &ClientRect);
		bool Scrolled = true;
		short xPos = GET_X_LPARAM(lParam);
		short yPos = GET_Y_LPARAM(lParam);
		RedrawStop();
		//scroll if dragging outsite by amount according to distance from edge and relative size of the text
		if (yPos < -m_textMetrics.tmHeight / 2)
		{
			LONG OffSet = max(1, -yPos / m_textMetrics.tmHeight); //scroll by distance from edge
			repeat(OffSet)
			{
				DirectSend(WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
			}
			Scrolled = true;
		}
		else if (yPos > ClientRect.bottom + m_textMetrics.tmHeight / 2) //scroll by n distance from edge
		{
			LONG OffSet = max(1, (yPos - ClientRect.bottom) / m_textMetrics.tmHeight);
			repeat(OffSet)
			{
				DirectSend(WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
			}
			Scrolled = true;
		}
		//cap the coord for character finding so it's always valid coord inside rect
		xPos = (short)max(m_Margin + 1, min(xPos, ClientRect.right - 1));
		yPos = (short)max(1, min(yPos, ClientRect.bottom - m_textMetrics.tmHeight)); // bottom may have half skipped lines, this ensures coord is on the last line
		//get the pos of the caharcter under mouse
		auto pos = SafeCharFromPos(xPos,yPos);
		//if it's not the same as initial or last move
		if ((LONG) pos != m_OldMouseChar)
		{
			if (m_CurrentCursor == m_IBeam && m_WordSelectMode == false) //is selecting from text window
			{
				//just set the end pos to character under mouse
				if (m_SelEnd != pos)
				{
					SendMessage(hWnd, EM_SETSEL, m_SelStart, pos);
					RedrawResume();
					RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
					//UpdateWindow(m_hWnd);
				}
			}
			else if (m_CurrentCursor == m_IBeam && m_WordSelectMode == true) //if word selectiong mode
			{
				//With the character under mouse, set as caret position select and perform home/end operation
				//to select the next word
				m_ScrollCaretEnabled = false; //disable the ScrollToCaret code because we will hug the left side
				
				//This crazy code looks at the 4 possible options for the next character selection, 
				//using the word search function from the mouse position and then looking back from the end of the word
				//and doing this in both directions results in 4 options possibile, even if in most cases some may be the same
				//Not going to explain it because that would only cause your brain to tilt. This code ivolved from the commented out
				//code still seen which worked most the time but was causing weirdism in cases with conflicting stop chars

				//////////////////////////
				//Options looking right and back
				m_SelEnd = pos;
				CaretMoveWord(1, 1);
				LONG Option1 = m_SelEnd;
				CaretMoveWord(-1, 1);
				LONG Option2 = m_SelEnd;
				//Options looking left and back
				m_SelEnd = pos;
				CaretMoveWord(-1, 1);
				LONG Option3 = m_SelEnd;
				CaretMoveWord(1, 1);
				LONG Option4 = m_SelEnd;
				//and a quick 4 option sort to figure out what option is closest to the character under the mouse
				if (abs(pos - Option3) > abs(pos - Option4)) //swap 4 and 4 if needed
				{
					Option3 = Option4;
				}
				if (abs(pos - Option1) > abs(pos - Option2)) //swap 1 and 2 if needed
				{
					Option1 = Option2;
				}
				if (abs(pos - Option1) > abs(pos - Option3)) //swap 1 and 3, resulting in 1 as closest option
				{
					Option1 = Option3;
				}
				m_SelEnd = Option1;
				////////////////////

				
				{
					//m_SelEnd = pos
					if (pos <= (m_LineStartSelStart + m_LineStartSelEnd) / 2) //home end operation differs if selecting upwards and downwards from initial position
					{

						m_SelStart = m_LineStartSelEnd;
						if (m_SelEnd == m_LineStartSelEnd) m_SelEnd = m_LineStartSelStart;
						//CaretMoveWord(1, 1);
						//CaretMoveWord(-1, 1);
					}
					else
					{

						m_SelStart = m_LineStartSelStart;
						if (m_SelEnd == m_LineStartSelStart) m_SelEnd = m_LineStartSelEnd;
						//CaretMoveWord(-1, 1);
						//CaretMoveWord(1, 1);
					}
					SendMessage(hWnd, EM_SETSEL, m_SelStart, m_SelEnd);
				}
				RedrawResume();
				RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
				m_ScrollCaretEnabled = true;

				//hack, if the prev char is a tab, force a OldChar to stay what it was because the select gets stuck a wee bit from it's position being too far
				//if (m_CaretChar != '\t' && m_CaretPrevChar == '\t') pos = m_OldMouseChar;

				//UpdateWindow(m_hWnd);
			}
			else if (m_CurrentCursor == m_RightArrow) //if selectiong from line number bar
			{
				//With the character under mouse, set as caret position select and perform home/end operation
				m_ScrollCaretEnabled = false; //disable the ScrollToCaret code because we will hug the left side

				m_SelEnd = pos;
				if (pos < m_LineStartSelStart) //home end operation differs if selecting upwards and downwards from initial position
				{
					m_SelStart = m_LineStartSelEnd;
					CaretMoveEndOfLine(1, 0, 1);
					CaretMoveEndOfLine(-1, 0, 1);
				}
				else
				{
					m_SelStart = m_LineStartSelStart;
					CaretMoveEndOfLine(-1, 0, 1);
					CaretMoveEndOfLine(1, 0, 1);
				}
				RedrawResume();
				DirectSend(WM_HSCROLL, MAKEWPARAM(SB_LEFT, 0), 0);
				RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
				m_ScrollCaretEnabled = true;
				//UpdateWindow(m_hWnd);
			}
		}
	
		m_OldMouseChar = (LONG) pos;
		RedrawResume(); //just in case we did not branch to a resume drawing branch
		if (Scrolled) //we did some scrolling
		{
			UpdateWindow(m_hWnd); //(a) when this happens the mouse move will keep posting...
		}
		if (Scrolled && OldMousePos == lParam) //if scrolling and not wiggling the mouse for faster scrolling
		{
			//delay
			Sleep(100); //(a)...which allows controlling the scroll speed without using sleep instead of a timer, weird hack discovered by accident
		}
		OldMousePos = lParam;
	}
	return 0;
}
LRESULT HookedCtrl::DoWM_TIMER(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//because the caret would stop blinking and disapear after 3 blinks... in case you are wondering why I took over this control
	if (wParam == m_CaretTimer)
	{
		if (GetFocus() == hWnd)
		{
			//Fucking caret fuck you I'll draw you myself 
			//Later: I decided to leave that comment in there :)
			//Turns out the default caret uses the back color of the control as caret color
			//and does xor with this color with the text box image... long story short, when the caret would be Off on the dark skin
			//it would appear ON on the dark skin... and when ON it would disapear.... 
			//... and the work around for this falls in the hands of the user of the edit box which is asking for trouble
			//so the cursor is now rendered in the draw, it costs a little more but I have full control of it, which is required
			//to match the color settingS defined in the language file
			//On Hook the caret is destroyed and on unhook it is recreated. carret functions still work though I replaced a few behaviors

			CaretOn = !CaretOn;
			if (m_NumCaretTicks>0) CaretOn = true;
			
			RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
			//UpdateWindow(m_hWnd);
			//if (CaretOn || m_NumCaretTicks >0)
			//	CaretOn
				//HideCaret(hWnd);
			//else
				//ShowCaret(hWnd);
			//SetCaretBlinkTime(500);
		}
		else if (CaretOn)
		{
			CaretOn = false;
			RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
			//UpdateWindow(m_hWnd);
		}
		m_NumCaretTicks = max(m_NumCaretTicks-1, 0);
		return 0l;
	}
	return CallOrigWindowProc(hWnd, message, wParam, lParam);
}


LRESULT HookedCtrl::DoEM_SCROLLCARET(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	
	//Replacement of the move to carret function so I can do it without the flicker
	if (m_ScrollCaretEnabled == false) return 0;
	LRESULT StartPos = DirectSend(EM_POSFROMCHAR, 0, 0);
	RedrawStop();
	RECT ClientRect; GetClientRect(hWnd, &ClientRect);
	
	LONG FirstVisibleLine = (LONG)DirectSend(EM_GETFIRSTVISIBLELINE,0,0);
	LONG CaretLine = (LONG)DirectSend(EM_LINEFROMCHAR, m_SelEnd, 0);
	LONG NumVisibleLines = (ClientRect.bottom + m_textMetrics.tmHeight) / m_textMetrics.tmHeight -1;
	LONG LastVisibleLine = FirstVisibleLine + NumVisibleLines;
	//scroll up or down by the exact amount of lines needed
	if (CaretLine < FirstVisibleLine)
	{
		m_WordListDlg.Hide();
		LONG OffSet = FirstVisibleLine - CaretLine;
		//DirectSend(EM_LINESCROLL, 0,-OffSet);
		repeat(OffSet)
		{
			DirectSend(WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
		}
	}
	if (CaretLine >= LastVisibleLine)
	{
		m_WordListDlg.Hide();
		LONG OffSet = CaretLine - LastVisibleLine;
		//DirectSend(EM_LINESCROLL, 0, OffSet);
		repeat(OffSet)
		{
			DirectSend(WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
		}
		if (HIWORD(DirectSend(EM_POSFROMCHAR, m_SelEnd, 0)) + m_textMetrics.tmHeight >  ClientRect.bottom)
		{
			DirectSend(WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
		}
	}
	//scroll left or right gradually untill in view
	short left = (short)ClientRect.left + m_Margin;
	int ct = 0;
	short xPos;
	int repval = max(1, ClientRect.right / m_textMetrics.tmAveCharWidth/3);
	do {
		xPos = LOWORD(SafePosFromChar(m_SelEnd, (short)m_textMetrics.tmAveCharWidth * 2));
		if (xPos < left)
		{
			repeat(repval)DirectSend(WM_HSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
			//DirectSend(EM_LINESCROLL, -repval ,0);
		}
		if (xPos > ClientRect.right)
		{
			repeat(repval)DirectSend(WM_HSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
			//DirectSend(EM_LINESCROLL, repval, 0);
		}
		
	} while (ct++ < 100 &&  xPos != -1 && (xPos<left || xPos>ClientRect.right));

	//LRESULT r = CallOrigWindowProc(hWnd, message, wParam, lParam);
	//if (GetFocus() == hWnd) DestroyCaret();
	RedrawResume();
	RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
	//returns true if scrolled
	return !(StartPos == DirectSend(EM_POSFROMCHAR, 0, 0));
}
LRESULT HookedCtrl::DoWM_SETFOCUS(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//to prevent flicker, do nothing, also no caret creating by default code
	//m_SearchDlg.Hide();
	TurnCaretOn();
	RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
	
	return 0l;
}
LRESULT HookedCtrl::DoWM_SCROLL(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	m_WordListDlg.Hide();
	//to prevent flicker
	RedrawStop();
	LRESULT r = CallOrigWindowProc(hWnd, message, wParam, lParam);
	RedrawResume();
	m_SearchDlg.UpdatePosition();
	m_WordListDlg.UpdatePosition();
	RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
	
	//UpdateWindow(m_hWnd);
	return r;

}


LRESULT HookedCtrl::DoWM_SETFONT(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RedrawStop();
	//Get the font about to be replaced
	HFONT hReplacedFont = (HFONT)DirectSend(WM_GETFONT, 0, 0);
	LRESULT r = CallOrigWindowProc(hWnd, message, wParam, lParam);
	//resend tab stops to update with the new font
	int ts[] = { m_NEBS.TabStop };
	DirectSend(EM_SETTABSTOPS, 1, (LPARAM)ts);
	HFONT hFont = (HFONT)wParam;
	//twas my mousewheel font?
	if (hReplacedFont != m_OEBS.font)
	{
		DeleteObject(hReplacedFont);
	}
	//set the "original" text box font to new one so I dont think this font is mine to free. 
	//the mouse wheel event with reset this if this was called from there
	m_OEBS.font = hFont;
	
	//Not you again? sod off
	if (GetFocus() == m_hWnd) DestroyCaret();
	//recalculate kerning
	FreeKerningPairs();
	RedrawResume();
	if (LOWORD(lParam))
	{
		RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
	}
	m_SearchDlg.ResizeToParentFont(hFont);
	m_WordListDlg.ResizeToParentFont(hFont);
	return r;
}
LRESULT HookedCtrl::DoWM_MOUSEWHEEL(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//to prevent flicker
	LRESULT r = 0;
	RedrawStop();
	if (KEY_STATE_DOWN(VK_CONTROL))
	{

		short Delta = GET_WHEEL_DELTA_WPARAM(wParam);
		int sz = m_textMetrics.tmHeight;
		

		if (Delta > 0) ZoomFont(1);
		else if (Delta < 0) ZoomFont(-1);
		

	}
	else if (KEY_STATE_DOWN(VK_SHIFT) && !m_IsWordWrap)
	{
		short Delta = GET_WHEEL_DELTA_WPARAM(wParam)/20;
		SendMessage(hWnd, EM_LINESCROLL, -Delta, 0);
	}
	else
	{
		r = CallOrigWindowProc(hWnd, message, wParam, lParam);
	}
	m_WordListDlg.Hide();

	RedrawResume();
	m_SearchDlg.UpdatePosition();
	m_WordListDlg.UpdatePosition();
	RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
	
	//UpdateWindow(m_hWnd);
	return r;

}
LRESULT HookedCtrl::DoWM_PAINT(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//My paint, do my paint
	PAINTSTRUCT ps;
	HDC hDC = BeginPaint(hWnd, &ps);
	Paint(hWnd, hDC);
	EndPaint(hWnd, &ps);
	return 0L;
};
LRESULT HookedCtrl::DoWM_ERASEBKGND(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//Why is the erase background different than a paint hmm? Not for me it aint
	Paint(hWnd, (HDC) wParam);
	m_SearchDlg.UpdatePosition();
	m_WordListDlg.UpdatePosition();
	return 1L;
};

void HookedCtrl::Paint(HWND hWnd, HDC hDC)
{
	SetTextCharacterExtra(hDC, 0);
	//Draw the control
	m_CaretChar = 0;
	m_CaretPrevChar = 0;
	m_TextLen = (LONG) DirectSend(WM_GETTEXTLENGTH, 0, 0);
	
	
	m_IsWordWrap = (GetWindowLongPtr(hWnd, GWL_STYLE) & ES_AUTOHSCROLL) != ES_AUTOHSCROLL;
	m_IsReadOnly = (GetWindowLongPtr(hWnd, GWL_STYLE) & ES_READONLY) == ES_READONLY;
	m_DontHideSelection = (GetWindowLongPtr(m_hWnd, GWL_STYLE) & ES_NOHIDESEL) == ES_NOHIDESEL;
	if (m_Language == NULL) return;
	//Set the margins if doing line numbers or not
	if (m_DrawLineNumbers)
	{
		//Calculated margin based on digits to draw and the font of the text
		HFONT hOldFont = (HFONT)SelectObject(hDC, (HFONT)DirectSend(WM_GETFONT, NULL, NULL));
		wstring format = L"0";
		size_t numlines = DirectSend(EM_GETLINECOUNT, 0, 0);
		while (numlines>=10)
		{
			format += L"0";
			numlines /= 10;
		}
		//set margins, if not already set, to the space the numbers would take
		bool NeedRedraw = SetLeftMarginTextWidth(format, hDC);
		SelectObject(hDC, hOldFont);
		if (NeedRedraw) //is it different than last?
		{
			//Bail. changing margin causes a redraw to use new setting
			return;
		}
	}
	else
	{
		//set margins to none, if not already set, if changed margins bail
		if (SetMargins(0, 0))
		{
			//Bail. changing margin causes a redraw to use new setting
			return;
		}
	}
	//Get the margin
	int MarginWidth = GetLeftMargin();
	m_Margin = MarginWidth;
	//Set the drawing DC to passed DC, will later be the mem DC is successful
	HDC DrawDC = hDC;

	//create a memory DC to draw on
	HDC hMemDC = CreateCompatibleDC(hDC);
	RECT ClientRect; GetClientRect(hWnd, &ClientRect);
	HBITMAP hOldBMP = NULL;
	if (hMemDC != NULL)
	{
		
		//Creat a hBitmap 
		HBITMAP hBmp = CreateCompatibleBitmap(hDC, ClientRect.right, ClientRect.bottom);
		if (hBmp)
		{
			//and size the DC with it
			hOldBMP = (HBITMAP)SelectObject(hMemDC, hBmp);
			DrawDC = hMemDC;
		}
	}
	
	//Now we draw the content, could be mem dc or, if mem dc failed, the window dc, it's the same code
	//Set the font
	HFONT hOldFont = (HFONT)SelectObject(DrawDC, (HFONT)DirectSend(WM_GETFONT, NULL, NULL));
	AllocKerningPairs(DrawDC);
	GetTextMetrics(DrawDC, &m_textMetrics);
	
	SetMapMode(DrawDC, MM_TEXT);
	//set the mode, this should be opaque on default
	SetBkMode(DrawDC, OPAQUE);
	//Erase the bacground with the text back color
	DrawFilledRect(DrawDC, { ClientRect.left,ClientRect.top,ClientRect.right,ClientRect.bottom}, m_ColorSpec->TEXTBACKCOLOR);
	//Do the margin if there is one, with the line number back color
	if(MarginWidth !=0) DrawFilledRect(DrawDC, { ClientRect.left,ClientRect.top,MarginWidth, ClientRect.bottom }, m_ColorSpec->LINENOBACKCOLOR);

	
	//grab the text memory block to draw it
	HLOCAL hMem = (HLOCAL) DirectSend(EM_GETHANDLE, 0, 0);

	//Get the first and last char to colorcode
	LONG FirstLine = (LONG)DirectSend(EM_GETFIRSTVISIBLELINE, 0, 0);
	size_t firstchar = DirectSend(EM_LINEINDEX, FirstLine, 0);
	LONG LastLine = (LONG)min(FirstLine + ClientRect.bottom / m_textMetrics.tmHeight, max(0,DirectSend(EM_GETLINECOUNT, 0, 0)-1));
	size_t lastchar = DirectSend(EM_LINEINDEX, LastLine, 0);
	lastchar += DirectSend(EM_LINELENGTH, lastchar, 0);
	ResizeColorBytes(m_TextLen);
	m_Parser.DoColorize(hMem, m_TextLen, firstchar, lastchar, m_hColorBytes, m_ColorByteAllocSize);

	TCHAR *Text = NULL;
	if (hMem)
	{
		
		TCHAR *Text = (TCHAR*)LocalLock(hMem);
		//check if we are doing word wrap, yes I gots word wrap support
		
		if (Text)
		{
			bool DoColoring = false;
			BYTE *ColorRefs = (BYTE*)LocalLock(m_hColorBytes);
			if (ColorRefs)
			{
				DoColoring = true;
			}
			if (m_TextLen)
			{
				//remember the neighbour character for navigation functions
				m_CaretChar = Text[CLAMPEB(m_SelEnd)];
				m_CaretPrevChar = Text[CLAMPEB(m_SelEnd-1)];
			}
			
			//If Selection
			LONG start = m_SelStart,
				 end = m_SelEnd;
			if (start > end) swap(start, end);
			bool HasSelection = (start != end);
			if ((start != end) && (m_DontHideSelection || (GetFocus() == hWnd)))
			{
				//From first visible to last visible
				LONG FirstLine = (LONG)DirectSend(EM_GETFIRSTVISIBLELINE, 0, 0);
				LONG LastLine = FirstLine + ClientRect.bottom / m_textMetrics.tmHeight;
				//from first sel line to last sel line
				LONG SelLineStart = (LONG)DirectSend(EM_LINEFROMCHAR, start, 0);
				LONG SelLineEnd = (LONG)DirectSend(EM_LINEFROMCHAR, end, 0);
				//if sel lines in reange of visible lines
				bool InRange = false;
				InRange = !(SelLineStart > LastLine);
				InRange = InRange && !(SelLineEnd < FirstLine);
				if (InRange)
				{
					SetBkColor(DrawDC, m_ColorSpec->SELECTEDTEXTCOLOR);
					SetTextColor(DrawDC, m_ColorSpec->TEXTBACKCOLOR);
					SetBkMode(DrawDC, OPAQUE);
					//cap the draw to smalest number of lines
					FirstLine = max(FirstLine, SelLineStart);
					LastLine = min(LastLine, SelLineEnd);
					LRESULT CharCoord;
					short xCoord;
					short yCoord;
					for (LONG i = FirstLine; i <= LastLine; i++)
					{
						//Get the start of line data
						LONG CharPos = (LONG)DirectSend(EM_LINEINDEX, i, 0);
						if (CharPos == -1) break;
						LONG LineLength = (LONG)DirectSend(EM_LINELENGTH, CharPos, 0);
						LONG LineCharPos = 0;
						CharCoord = DirectSend(EM_POSFROMCHAR, CharPos, 0);;
						xCoord = LOWORD(CharCoord);
						yCoord = HIWORD(CharCoord);
						if (yCoord >= ClientRect.bottom) break;
						//move toward draw region, skipping over hidden, but tabs are an exception
						if (LineLength)
						{
							while (xCoord < MarginWidth && LineCharPos < LineLength && Text[CharPos] != '\t')
							{
								xCoord = GetNextXCoord(DrawDC, xCoord, CharPos, Text[CharPos], Text[CLAMPEB2(CharPos + 1)]);
								CharPos++;
								LineCharPos++;
							}
							//draw the characters, producing the selection since opaque
							while (xCoord < ClientRect.right && LineCharPos < LineLength)
							{
								if (CharPos >= start && CharPos < end)
								{
									if (Text[CharPos] != '\t')
										TextOut(DrawDC, xCoord, yCoord, (Text + CharPos), 1);
									else
									{   //for tabs get the next character position for extent of selection
										short xend = GetNextXCoord(DrawDC, xCoord, CharPos, Text[CharPos], Text[CLAMPEB2(CharPos + 1)]);
										DrawFilledRect(DrawDC, { xCoord,yCoord,xend,yCoord + m_textMetrics.tmHeight }, m_ColorSpec->SELECTEDTEXTCOLOR);
									}
								}
								xCoord = GetNextXCoord(DrawDC, xCoord, CharPos, Text[CharPos], Text[CLAMPEB2(CharPos + 1)]);
								CharPos++;
								LineCharPos++;
							}
						}
						else
						{
							DrawFilledRect(DrawDC, { xCoord,yCoord,xCoord+1,yCoord + m_textMetrics.tmHeight }, m_ColorSpec->SELECTEDTEXTCOLOR);
						}
					}
				}
			}
			else if(!m_IsReadOnly && GetFocus() == hWnd) //no selection, do active line bar
			{
				LONG FirstLine = (LONG)DirectSend(EM_GETFIRSTVISIBLELINE, 0, 0);
				LONG CursorLine = (LONG)DirectSend(EM_LINEFROMCHAR, -1, 0);
				short yStart = -1;
				if (CursorLine >= FirstLine)
				{
					yStart = (short)((CursorLine - FirstLine)*m_textMetrics.tmHeight);
					if ((yStart) < ClientRect.bottom)
					{
						RECT LineRect = { MarginWidth , yStart, ClientRect.right, yStart + m_textMetrics.tmHeight };
						if (LineRect.bottom > LineRect.top)
						{
							DrawFilledRect(DrawDC, LineRect, m_ColorSpec->ACTIVELINECOLOR);
							if (MarginWidth)
							{
								LineRect.left = 0;
								LineRect.right = MarginWidth;
								DrawFilledRect(DrawDC, LineRect, m_ColorSpec->ACTIVELINENOCOLOR);
							}
						}
					}
				}
			}
			//draw the text
			{
				SetBkMode(DrawDC, TRANSPARENT);
				//From first to last visible line
				LONG FirstLine = (LONG) DirectSend(EM_GETFIRSTVISIBLELINE,0,0);
				LONG LastLine = FirstLine + ClientRect.bottom / m_textMetrics.tmHeight;
				long LineNo = FirstLine + 1; //line number 
				
				LONG PosLast;
				//if it word wraps, manually count the lines and find the first line that would receive a line number before the first visible line
				if (m_IsWordWrap && MarginWidth)
				{
					LONG CharPos = (LONG)DirectSend(EM_LINEINDEX, FirstLine, 0);
					LineNo = CountLinesOFW(Text, CharPos, &PosLast);
					//this will cause the draw to start above the client region but it eases the line number handling
					FirstLine = (LONG)DirectSend(EM_LINEFROMCHAR, PosLast, 0);
					//plus we dont care that much doing extra drawing when cliping is on
					
				}
				wstring LineNoText;
				LRESULT CharCoord;
				short xCoord = 0;
				short yCoord = 0;
				short yLineNoCoord = 0;
				//lopp through visible lines
				for (LONG i = FirstLine; i <= LastLine; i++)
				{
					
					LONG CharPos = (LONG)DirectSend(EM_LINEINDEX, i, 0);
					//bail if done
					if (CharPos == -1)	break;
					//Get the start of line data
					LONG LineLength = (LONG)DirectSend(EM_LINELENGTH, CharPos, 0);
					LONG LineCharPos = 0;
					CharCoord = DirectSend(EM_POSFROMCHAR, CharPos, 0);;
					xCoord = LOWORD(CharCoord);
					yCoord = HIWORD(CharCoord);
					if (MarginWidth != 0) //line numbers are on
					{
						if (yCoord != -1) //if we have a valid coord, set the line number ypos
						{
							yLineNoCoord = yCoord;
						}
						bool DoLine = true; //default to valid line when not word wrapping
						if (m_IsWordWrap) //if we word wrap we need to only do the line number if the character is the first one or if the charcter previous is a newline
						{
							//result is 
							//1 A Line that is
							//  wrapped
							//2 Another line
							//result scrolled 1 line 
							//  wrapped
							//2 Another line
							if (CharPos != 0)
							{
								DoLine = (Text[CharPos - 1] == '\n'); //valid line
							}
						}
						if (DoLine) //if it's a valid line, inc line number and draw it in margin
						{
							if (yLineNoCoord < ClientRect.bottom)
							{
								LineNoText = to_wstring(LineNo++);
								SetTextAlign(DrawDC, TA_TOP | TA_RIGHT);
								SetTextColor(DrawDC, m_ColorSpec->LINENOCOLOR);
								TextOut(DrawDC, MarginWidth - LineNoTextGap, yLineNoCoord, LineNoText.c_str(), (UINT)LineNoText.length());
								SetTextAlign(DrawDC, TA_TOP | TA_LEFT);
							}
						}
						yLineNoCoord += (short)m_textMetrics.tmHeight;
					}
					//bail if out of space to draw
					if (yCoord >= ClientRect.bottom) break;
					
					//move towards the draw region if outside, to the left
					while (xCoord < MarginWidth && LineCharPos < LineLength)
					{
						xCoord = GetNextXCoord(DrawDC, xCoord, CharPos, Text[CharPos], Text[CLAMPEB2(CharPos+1)]);
						CharPos++;
						LineCharPos++;
					}
					//draw the characters
					while (xCoord < ClientRect.right && LineCharPos < LineLength)
					{
						SetTextColor(DrawDC, m_ColorSpec->COLORS[ColorRefs[min((CharPos),m_ColorByteAllocSize-1)]]);
						
						TextOut(DrawDC, xCoord, yCoord, (Text + CharPos), 1);
						xCoord = GetNextXCoord(DrawDC, xCoord, CharPos, Text[CharPos], Text[CLAMPEB2(CharPos + 1)]);
						CharPos++;
						LineCharPos++;
					}
					

				}
				//do the caret myself because it sucks to try to make the existing one work as it uses the wrong color and disapears
				if (CaretOn)
				{
					//figure out where I should draw the caret. special case for at end of line and at end of content or no content
					LONG CharPos = m_SelEnd;
					xCoord = MarginWidth;
					yCoord = 0; //default no content, top of box
					
					if (m_TextLen && m_SelEnd == m_TextLen) //end of content
					{
						if (Text[m_SelEnd - 1] == '\n') //end of content after a new line
						{
							CharCoord = DirectSend(EM_POSFROMCHAR, m_SelEnd - 1, 0);
							yCoord = HIWORD(CharCoord) + (short) m_textMetrics.tmHeight;
							
						}
						else //end of content after a regular character
						{
							CharCoord = DirectSend(EM_POSFROMCHAR, m_SelEnd - 1, 0);
							xCoord = LOWORD(CharCoord);
							yCoord = HIWORD(CharCoord);
							xCoord = GetNextXCoord(DrawDC, xCoord, CharPos - 1, Text[CharPos - 1], Text[CharPos]);
						}
					}
					else if (m_TextLen) //in between 0 and end content 
					{
						CharCoord = DirectSend(EM_POSFROMCHAR, m_SelEnd, 0);
						xCoord = LOWORD(CharCoord);
						yCoord = HIWORD(CharCoord);
						//xCoord = GetNextXCoord(DrawDC, xCoord, CharPos - 1, Text[m_SelEnd - 1], Text[m_SelEnd]);
					}
					if (xCoord != -1 && yCoord != -1 && (yCoord < ClientRect.bottom))
					{
						if (m_InsertMode == 1)
						{
							DrawFilledRect(DrawDC, { xCoord, yCoord, xCoord + 2, yCoord + m_textMetrics.tmHeight }, m_ColorSpec->CARETCOLOR);
						}
						else
						{
							if (m_CaretChar == 13 || m_SelEnd == m_TextLen)
							{
								DrawFilledRect(DrawDC, { xCoord, yCoord + m_textMetrics.tmHeight / 2, xCoord + 2, yCoord + m_textMetrics.tmHeight }, m_ColorSpec->CARETCOLOR);
							}
							else
							{
								//SetTextColor(DrawDC, m_Language->TEXTBACKCOLOR);
								//SetBkColor(DrawDC, m_Language->CARETCOLOR);
								//SetBkMode(DrawDC, OPAQUE);
								//TextOut(DrawDC, xCoord, yCoord, (Text + CharPos), 1);

								short x2 = GetNextXCoord(DrawDC, xCoord, CharPos, Text[CharPos], Text[CLAMPEB2(CharPos + 1)]);
								DrawFilledRect(DrawDC, { xCoord, yCoord + m_textMetrics.tmHeight - 2, x2, yCoord + m_textMetrics.tmHeight }, m_ColorSpec->CARETCOLOR);
							}
						}
					}
				}
				

			}
			
			//unlock the memory
			LocalUnlock(hMem);
			if (ColorRefs)
			{
				LocalUnlock(m_hColorBytes);
			}
		}
	}
	
	
	//Restore DC or original State
	SetBkMode(DrawDC, OPAQUE);
	SelectObject(hDC, hOldFont);

	
	//if we did it on the Mem DC
	if (hMemDC)
	{
		//Transfer mem DC to window DC
		BitBlt(
			hDC,//_In_ HDC   hdcDest,
			0,//_In_ int   nXDest,
			0,//_In_ int   nYDest,
			ClientRect.right, //_In_ int   nWidth,
			ClientRect.bottom, //_In_ int   nHeight,
			hMemDC,//_In_ HDC   hdcSrc,
			0,//_In_ int   nXSrc,
			0,//_In_ int   nYSrc,
			SRCCOPY//_In_ DWORD dwRop
		);

		//Cleanup
		//if we have the OldBmp, the new one is inside the DC. select the old delete the new
		if (hOldBMP)
		{
			DeleteObject(SelectObject(hMemDC, hOldBMP));
		}
		//Cleanup
		DeleteDC(hMemDC);
	}

};
