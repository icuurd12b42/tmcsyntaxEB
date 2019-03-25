
#pragma once
#include "Parser.h"
#include "resource.h"
#define CLAMPEB(v) max(0,min(m_TextLen,v))
#define CLAMPEB2(v) max(0,min(m_TextLen-1,v))
class EditAction
{
public:
	size_t m_OSelStart;
	size_t m_OSelEnd;
	wstring m_OrigSelText;
	size_t m_NSelStart;
	size_t m_NSelEnd;
	wstring m_NewText;
	bool m_RememberNewSize;
	EditAction(size_t OSelStart, size_t OSelEnd, wstring OldSelText, size_t NSelStart, size_t NSelEnd, wstring NewText, bool RememberNewSize)
	{
		m_OSelStart = OSelStart;
		m_OSelEnd = OSelEnd;
		m_OrigSelText = OldSelText;
		m_NSelStart = NSelStart;
		m_NSelEnd = NSelEnd;
		m_NewText = NewText;
		m_RememberNewSize = RememberNewSize;
	}
};
typedef stack<EditAction*> EditActions;
extern HINSTANCE ghDllHandle;
typedef struct EbSettings{ //edit box settings. some values default to my settings
	WNDPROC WndProc = NULL;
	short TabStop = 16;
	LPARAM Margins = 0;
	UINT BlinkTime = 0;
	HBRUSH BkBrush = 0;
	HFONT font = 0;
	int TextHeight = 0;
} EBSETTINGS;
typedef struct ColorSpecs
{
	COLORREF DlgBkColor = 0;
	HBRUSH DlgBkColorBrush = NULL;

	COLORREF CtrlTextColor = 0;
	COLORREF CtrlBkColor = 0;
	HBRUSH CtrlBkColorBrush = NULL;

	COLORREF CtrlButColor = 0;
	HBRUSH CtrlButColorBrush = NULL;
	COLORREF CtrlButTextColor = 0;
	HBRUSH CtrlButTextColorBrush = NULL;

	COLORREF CtrlButHoverColor = 0;
	HBRUSH CtrlButHoverColorBrush = NULL;
	COLORREF CtrlButDownColor = 0;
	HBRUSH CtrlButDownColorBrush = NULL;

} COLORSPECS;
class HookedCtrl
{
public: //yeah, I'm bad with privacy, this aint a corporate project
	HWND m_hWnd = NULL;
	EBSETTINGS m_OEBS;
	EBSETTINGS m_NEBS;

	Language *m_Language = NULL;
	ColorSpec *m_ColorSpec = NULL;
	bool m_DrawLineNumbers = true;
	EditActions m_UndoStack;
	EditActions m_RedoStack;
	int LineNoTextGap = 4;
	LONG m_SelStart = 0;
	LONG m_SelEnd = 0;
	int m_StopRedraw = 0;
	int m_CaretTimer = -1;
	bool CaretOn = false;
	int m_NumCaretTicks = 0;
	LONG m_TextLen = 0;
	TEXTMETRIC m_textMetrics;
	TCHAR m_CaretChar = 0;
	TCHAR m_CaretPrevChar = 0;
	bool m_IsWordWrap = false;
	short m_Margin = 0;
	HCURSOR m_RightArrow;
	HCURSOR m_Arrow;
	HCURSOR m_IBeam;
	HCURSOR m_CurrentCursor;
	bool m_ScrollCaretEnabled = true;
	LONG m_LineStartSelStart = 0;
	LONG m_LineStartSelEnd = 0;
	LPARAM OldMousePos = 0;
	bool m_WordSelectMode = false;
	LONG m_OldMouseChar = 0;
	LRESULT m_InsertMode = 1;
	bool m_IsReadOnly = false;
	bool m_DontHideSelection = false;
	SearchDialog m_SearchDlg;
	WordListDialog m_WordListDlg;
	COLORSPECS m_Colors;

	KERNINGPAIR*   m_KerningPairs = NULL;
	int m_NumKerningPairs = 0;

	WORDLIST m_WordList;
	HLOCAL m_hColorBytes = NULL;
	LONG m_ColorByteAllocSize = 0;
	
	Parser m_Parser;
	

	size_t m_WLWordStart = 0;
	size_t m_WLWordEnd = 0;
	size_t m_WLAltWordStart = 0;

public:

	HookedCtrl(HWND hWnd) {
		
		m_SearchDlg.Init(hWnd);
		m_WordListDlg.Init(hWnd);
		/*
		m_WordList.push_back(L"Word1");
		m_WordList.push_back(L"Word2");
		m_WordList.push_back(L"Word3");
		m_WordList.push_back(L"Word4");
		m_WordList.push_back(L"Word5");
		m_WordList.push_back(L"Word6");
		m_WordList.push_back(L"Word7");
		m_WordList.push_back(L"Word8");
		m_WordList.push_back(L"Word9");
		m_WordList.push_back(L"Word10");
		m_WordList.push_back(L"Word11");
		m_WordList.push_back(L"Word12");
		m_WordListDlg.SetWordList(m_WordList);
		*/
		m_Parser.SetHookedCtrl(this);
		m_ColorByteAllocSize = 65536;
		m_hColorBytes = LocalAlloc(LHND, sizeof(BYTE) * m_ColorByteAllocSize);

		m_RightArrow = LoadCursor(ghDllHandle, MAKEINTRESOURCE(IDC_RIGHTPOINTER));
		m_Arrow = LoadCursor(NULL, IDC_ARROW);
		m_IBeam = LoadCursor(NULL, IDC_IBEAM);
		m_CurrentCursor = m_IBeam;

		m_hWnd = hWnd;
	};

	~HookedCtrl() {
		OnUnHook();
		m_hColorBytes = LocalFree(m_hColorBytes);
	};

	LRESULT DoIndent(int Style)
	{
		wstring t = m_Parser.DoIndent((HLOCAL)DirectSend(EM_GETHANDLE, 0, 0), m_TextLen, Style);
		//SetWindowText(m_hWnd, t->c_str());
		LONG endsel = m_SelEnd;
		RedrawStop();
		m_ScrollCaretEnabled = false;
		SendMessage(m_hWnd, EM_SETSEL, 0, -1);
		AddUndoableContent(t);
		
		SendMessage(m_hWnd, EM_SETSEL, endsel, endsel);
		RedrawResume();
		m_ScrollCaretEnabled = true;
		CaretMoveEndOfLine(-1, 0, 0);
		return 0l;
	}
	//Color bytes manips
	void ResizeColorBytes(DWORD EBTextLen)
	{
		//this functions grows or shrinks the color bytes according to the passed edit box text size. a 64k buffer growth is used so to limit the amount of re-allocs
		DWORD AllocatedSize = m_ColorByteAllocSize;
		//shrink down if eb text is less than what is alocated
		while (AllocatedSize > EBTextLen) AllocatedSize -= 65536;
		//grow if eb text is more or equal to what is allocated, this allows for the extra 64k buffer
		while (AllocatedSize <= EBTextLen) AllocatedSize += 65536;
		//if the resulting amount is not what is currently allocated, grow or shrink
		if (AllocatedSize != m_ColorByteAllocSize)
		{
			if (NULL != LocalReAlloc(m_hColorBytes, sizeof(BYTE) * AllocatedSize, LHND))
			{
				//failed, memory was not re-alocated, stick to initial values
				m_ColorByteAllocSize = AllocatedSize;
			}
		}
	}
	
	//Main Window Procedure stuff
	void RestoreWindowProc() { SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, (LONG_PTR)m_OEBS.WndProc); };

	void StoreWindowProcs(WNDPROC origWndProc, WNDPROC newWndProc) { m_OEBS.WndProc = origWndProc; m_NEBS.WndProc = newWndProc; };

	LRESULT DirectSend(UINT message, WPARAM wParam, LPARAM lParam) { return CallWindowProc(m_OEBS.WndProc, m_hWnd, message, wParam, lParam); };

	LRESULT CallOrigWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) { return CallWindowProc(m_OEBS.WndProc, hWnd, message, wParam, lParam); };

	LRESULT DoWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	//Message Handlers
	LRESULT DoWM_PAINT(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT DoWM_ERASEBKGND(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT DoWM_USER(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT DoWM_CHAR(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT DoWM_SIZE(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT DoEM_SETSEL(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT DoEM_GETSEL(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT DoWM_PASTE(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT DoWM_LBUTTONDOWN(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT DoWM_LBUTTONUP(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT DoWM_MOUSEMOVE(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT DoWM_TIMER(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT DoWM_KILLFOCUS(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT DoWM_KEYDOWN(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT DoEM_SCROLLCARET(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT DoWM_SETFOCUS(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT DoWM_SCROLL(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT DoWM_SETCURSOR(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT DoWM_LBUTTONDBLCLK(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT DoWM_COPY(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT DoWM_CUT(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT DoWM_MOUSEWHEEL(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT DoWM_SETFONT(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	LRESULT DoEM_SETREADONLY(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		RedrawStop();
		LRESULT r = CallOrigWindowProc(hWnd, message, wParam, lParam);
		RedrawResume();
		m_IsReadOnly = (GetWindowLongPtr(hWnd, GWL_STYLE) & ES_READONLY) == ES_READONLY;
		RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
		return r;
	}
	LRESULT DoUM_SETNOHIDESEL(WPARAM wParam)
	{
		LONG_PTR s = GetWindowLongPtr(m_hWnd, GWL_STYLE);

		if (wParam)
		{
			s |= ES_NOHIDESEL;
		}
		else
		{
			s ^= ES_NOHIDESEL;
		}
	
		
		SetWindowLongPtr(m_hWnd, GWL_STYLE, (LONG_PTR)s);
		m_DontHideSelection = (GetWindowLongPtr(m_hWnd, GWL_STYLE) & ES_NOHIDESEL) == ES_NOHIDESEL;
		RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
		SetWindowPos(m_hWnd, 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE);
		return 0l;
	}
	LRESULT DoWM_SETTEXT(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		RedrawStop();
		LRESULT r = CallOrigWindowProc(hWnd, message, wParam, lParam);
		FreeUndoRedo();
		DirectSend(EM_GETSEL, (WPARAM)&m_SelStart, (LPARAM)&m_SelEnd);
		RedrawResume();
		RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
		return r;
	}
	LRESULT DoEM_REPLACESEL(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		
		this->AddUndoableContent((TCHAR*)lParam,true);
		if(wParam == 0) FreeUndoRedo();
		
		return 0L;
	}
	void FreeUndoRedo()
	{
		//clean up the redo and undo stack
		while (!m_RedoStack.empty())
		{
			EditAction *t = m_RedoStack.top();
			m_RedoStack.pop();
			delete t;
		}
		while (!m_UndoStack.empty())
		{
			EditAction *t = m_UndoStack.top();
			m_UndoStack.pop();
			delete t;
		}
	}
	LRESULT DoEM_EMPTYUNDOBUFFER(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		FreeUndoRedo();
		return CallOrigWindowProc(hWnd, message, wParam, lParam);
	}
	//Drawing
	void RedrawStop()
	{
		//Call to cancel drawing, added a stack type ability
		m_StopRedraw++;
		DirectSend(WM_SETREDRAW, m_StopRedraw == 0, 0);
	}

	void RedrawResume()
	{
		//Call to resume drawing, added a stack type ability
		m_StopRedraw = max(0, m_StopRedraw-1);
		DirectSend(WM_SETREDRAW, m_StopRedraw == 0, 0);
	}
	void Paint(HWND hWnd, HDC hDC);

	
	

	
	//Language
	void SetLanguage(Language *Language) { 
		m_Language = Language; 
		m_WordList.clear();
		if (m_Language != NULL)
		{
			for (const auto& elem : m_Language->KEYWORDS.m_Words) {
				wstringex *t = new wstringex();
				t->assign(elem);
				t->type = COLORDEREF::KEYWORDCOLOR;
				m_WordList.push_back(*t);
			}
			for (const auto& elem : m_Language->LITERALS.m_Words) {
				wstringex *t = new wstringex();
				t->assign(elem);
				t->type = COLORDEREF::LITERALCOLOR;
				m_WordList.push_back(*t);
			}
			for (const auto& elem : m_Language->FUNCTIONS.m_Words) {
				wstringex *t = new wstringex();
				t->assign(elem);
				t->type = COLORDEREF::FUNCTIONCOLOR;
				m_WordList.push_back(*t);
			}
			for (const auto& elem : m_Language->STATICCLASS.m_Words) {
				wstringex *t = new wstringex();
				t->assign(elem);
				t->type = COLORDEREF::STATICCLASSCOLOR;
				m_WordList.push_back(*t);
			}
			for (const auto& elem : m_Language->CLASSFUNCS.m_Words) {
				wstringex *t = new wstringex();
				t->assign(elem);
				t->type = COLORDEREF::CLASSFUNCCOLOR;
				m_WordList.push_back(*t);
			}
			
		}
	};

	Language * GetLanguage() { return m_Language; };

	//Color Specs
	void SetColorSpec(ColorSpec *ColSpec) {
		m_ColorSpec = ColSpec;
		
	};

	ColorSpec * GetColorSpec() { return m_ColorSpec; };
	
	//Tab Stops
	void SaveTabStops() { m_OEBS.TabStop = 32; };

	void SetTabStops() { int ts[1]; ts[0] = m_NEBS.TabStop; DirectSend(EM_SETTABSTOPS, 1, (LPARAM)ts); };

	void RestoreTabStops() { int ts[1]; ts[0] = m_OEBS.TabStop; DirectSend(EM_SETTABSTOPS, 1, (LPARAM)ts); };

	//Margins
	void SaveMargins() { m_OEBS.Margins = DirectSend(EM_GETMARGINS, 0, 0); };

	void SaveFont() { m_OEBS.font = (HFONT)DirectSend(WM_GETFONT, 0, 0); };
	void RestoreFont()
	{
		HFONT CurrentFont = (HFONT)DirectSend(WM_GETFONT, 0, 0);
		DirectSend(WM_SETFONT, (WPARAM)m_OEBS.font, 0);
		if (CurrentFont != m_OEBS.font) DeleteObject(CurrentFont);
	};

	//return true if changed so we know a redraw was sent so we can cancel drawing
	bool SetMargins(int left, int right) { m_NEBS.Margins = MAKELPARAM(left, right); if (DirectSend(EM_GETMARGINS, 0, 0) != m_NEBS.Margins) { DirectSend(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, m_NEBS.Margins); return true; } return false; };
	bool SetLeftMarginTextWidth(wstring MarginText, HDC hdc) {	SIZE sz; GetTextExtentPoint32(hdc,MarginText.c_str(),(int)MarginText.length(),&sz	);	return SetMargins(sz.cx + LineNoTextGap * 2, 0); };

	WORD GetLeftMargin() { return LOWORD(DirectSend(EM_GETMARGINS, 0, 0)); };

	void RestoreMargins() {	DirectSend(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, m_OEBS.Margins); };

	//Caret/ Cursor
	void SetTheCursor(HCURSOR cur)
	{
		
		//used to change the cursor
		if (GetCursor() != cur) //if already set, prevent cursor freakout, do not call
		{
			m_CurrentCursor = cur;
			SetCursor(m_CurrentCursor);
		}
	}
	void TurnCaretOn(int NumTicks = 1)
	{
		//turns the caret visiility on. NumTicks specifies how long it remain full on before blinking again
		m_NumCaretTicks = NumTicks;
		CaretOn = true;
		//ShowCaret(m_hWnd);
	}
	void SelectNextChar(int direction)
	{
		//this function safely selects one character for replacement. usually either 1 char or the \r\n EOL
		//correct any between \r\n issue the caret will appear at the end of the line still to the user, so got back before the \r\n
		if (direction == 1 && m_CaretChar == '\n')
			m_SelEnd -= 1;
		else if (direction == -1 && m_CaretPrevChar == '\r')
			m_SelEnd -= 1;

		m_SelStart = m_SelEnd;
		if (direction == 1 && m_CaretChar == '\r')
			m_SelEnd += 2;
		else if (direction == -1 && m_CaretPrevChar == '\n')
			m_SelEnd -= 2;
		else
			m_SelEnd += direction;
		SendMessage(m_hWnd, EM_SETSEL, (WPARAM)m_SelStart, (LPARAM)m_SelEnd);
	}

	void CleanWord(wstring &t)
	{
		t.erase(std::remove(t.begin(), t.end(), L'\n'), t.end());
		t.erase(std::remove(t.begin(), t.end(), L'\r'), t.end());
	}
	//Selection
	wstring GetSelText(LONG ss = -1, LONG se = -1)
	{
		//get the selection text
		wstring retString;
		DWORD SelStart = ss,
			SelEnd = se;
		if(ss == -1)
			DirectSend(EM_GETSEL, (WPARAM) &SelStart,  (LPARAM) &SelEnd);
		if (SelStart > SelEnd) swap(SelStart, SelEnd);
		//create a string directly from the memory handle
		HLOCAL hMem = (HLOCAL)DirectSend(EM_GETHANDLE, 0, 0);
		TCHAR *Text = NULL;
		if (hMem)
		{
			TCHAR *Text = (TCHAR*)LocalLock(hMem);
			if (Text)
			{
				//by assigning a rterminating null tempararely to the end of select
				TCHAR OldChar = Text[SelEnd];
				Text[SelEnd] = 0;
				//and relying on the wstring TCHAR* asign
				retString = (Text + SelStart);
				//place back the character that was there
				Text[SelEnd] = OldChar;
				LocalUnlock(hMem);
				
			}
		}
		return retString;
	}
	
	void AddUndoableContent(wstring Text, bool RememberNewSize = false, bool SelectNewText = false, UINT message = 0, WPARAM WMCHAR = 0, LPARAM WMCHARLPARAM = 0)
	{
		//all the message, wparam craziness is because the code used to blindly call REMPLACESEL which is very slow on huge code when editing at top. so instead, when 
		//possible, we send the WM_CHAR message and params, which the edit box is faster to process the inserts with. there is still some slowdown for enter and shift tab 
		//and backspse on the first character in the line which I cant resolve... the slowdowns are almost always the same as in the plain jane edit box...
		//...I spent about 3 weeks trying to resolve the slowdowns with huge code, we are talking about tens of thousand of lines of code here where the slowdown happens, 
		//a scenario that is unlikely, but being a massive anal retentive programmer, I was being thorough to have it working in that scenario.
		
		if (m_IsReadOnly) return;

		m_Parser.SetChangeRegion(m_SelStart, m_SelEnd);

		//this function is called to change the content of the text box. it also records the action for undo...
		m_ScrollCaretEnabled = false;
		RedrawStop();
		//clean up the redo stack
		while (!m_RedoStack.empty())
		{
			EditAction *t = m_RedoStack.top();
			m_RedoStack.pop();
			delete t;
		}
		//get what was selected
		wstring OldText = this->GetSelText();
		//get the selection range
		DWORD start = m_SelStart, end = m_SelEnd;
		//DirectSend(EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
		//add the info to the undo stack
		LONG NewStart = min(start, end);
		LONG NewEnd = NewStart + (LONG) Text.length();
		
		m_UndoStack.push(new EditAction(start, end, OldText, NewStart, NewEnd, Text, RememberNewSize));
		//perform the replace
		if (!message)
		{
			DirectSend(EM_REPLACESEL, FALSE, (LPARAM)Text.c_str());
		}
		else // if wm_char message is passed... 
		{
			//DirectSend(WM_KEYDOWN, WMCHAR, WMCHARLPARAM);
			DirectSend(message, WMCHAR, WMCHARLPARAM);
			//DirectSend(WM_KEYUP, WMCHAR, WMCHARLPARAM);
		}
		//move caret to the end of the new content
		if (start > end) swap(start, end);
		end = start + (DWORD)Text.length();
		if (!SelectNewText) start = end;
		SendMessage(m_hWnd, EM_SETSEL, (WPARAM)start, (LPARAM)end);
		
		RedrawResume();
		m_ScrollCaretEnabled = true;
		SendMessage(m_hWnd, EM_SCROLLCARET, 0, 0);
		TurnCaretOn();
		RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
		m_TextLen = (LONG)DirectSend(WM_GETTEXTLENGTH, 0, 0);


		m_Parser.SetChangeRegion(m_SelStart, m_SelEnd);


		if (m_WordListDlg.IsVisible() && m_WordListDlg.m_hParent == m_hWnd)
		{
			//if (Text != L" " && Text != L"\t")
			{
				DoWordList();
			}
			//else
			//{
			//	m_WordListDlg.Hide();
			//}
		}
		//UpdateWindow(m_hWnd);
		
	}
	bool CanUndo()
	{
		return (LRESULT)!m_UndoStack.empty();
	}
	bool EditUndo() {
		if (m_IsReadOnly) return false;
		//TODO, retest
		if (CanUndo())
		{
			TurnCaretOn();
			EditAction *t = m_UndoStack.top();
			RedrawStop();
			SendMessage(m_hWnd, EM_SETSEL, t->m_NSelStart, t->m_NSelEnd);
			m_Parser.SetChangeRegion(m_SelStart, m_SelEnd);

			if (SendMessage(m_hWnd, EM_SCROLLCARET, 0, 0)) //do not do it until we see it, if scrolled, bail
			{
				RedrawResume();
				RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
				return true;
			}
			DirectSend(EM_REPLACESEL, FALSE, (LPARAM)t->m_OrigSelText.c_str());
			SendMessage(m_hWnd, EM_SETSEL, t->m_OSelStart, t->m_OSelEnd);
			RedrawResume();
			m_Parser.SetChangeRegion(m_SelStart, m_SelEnd);
			SendMessage(m_hWnd, EM_SCROLLCARET, 0,0);
			m_UndoStack.pop();
			m_RedoStack.push(t);
			return true;
		}
		else
		{
			TurnCaretOn();
			SendMessage(m_hWnd, EM_SETSEL, m_SelEnd, m_SelEnd);
		}
		return false;
	};
	
	
	
	LRESULT GetLineMargin()
	{
		return (LRESULT)m_DrawLineNumbers;
	}
	LRESULT SetLineMargin(WPARAM mode)
	{
		LRESULT Old = m_DrawLineNumbers;
		m_DrawLineNumbers = mode;
		RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
		return Old;
	}
	LRESULT GetInsertMode()
	{
		return (LRESULT) m_InsertMode;
	}

	LRESULT SetInsertMode(WPARAM mode)
	{
		if (m_IsReadOnly) return -1l;
		LRESULT Old = m_InsertMode;
		TurnCaretOn();
		m_InsertMode = mode;
		RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
		return Old;
	}
	bool CanRedo()
	{
		return (LRESULT)!m_RedoStack.empty();
	}
	bool EditRedo() {
		if (m_IsReadOnly) return 0l;
		//TODO: retest
		if (CanRedo())
		{
			TurnCaretOn();
			EditAction *t = m_RedoStack.top();
			RedrawStop();
			SendMessage(m_hWnd, EM_SETSEL, t->m_OSelStart, t->m_OSelEnd);
			m_Parser.SetChangeRegion(m_SelStart, m_SelEnd);

			if (SendMessage(m_hWnd, EM_SCROLLCARET, 0, 0))  //do not do it until we see it, if scrolled, bail
			{
				RedrawResume();
				RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
				return true;
			}
			

			DirectSend(EM_REPLACESEL, FALSE, (LPARAM)t->m_NewText.c_str());
			
			if(t->m_RememberNewSize)
				SendMessage(m_hWnd, EM_SETSEL, t->m_NSelStart, t->m_NSelEnd);
			else
				SendMessage(m_hWnd, EM_SETSEL, t->m_NSelEnd, t->m_NSelEnd);

			RedrawResume();
			m_Parser.SetChangeRegion(m_SelStart, m_SelEnd);

			SendMessage(m_hWnd, EM_SCROLLCARET, 0, 0);
			m_RedoStack.pop();
			m_UndoStack.push(t);
			return true;
		}
		else
		{
			TurnCaretOn();
			SendMessage(m_hWnd, EM_SETSEL, m_SelEnd, m_SelEnd);
		}
		return false;
	};

	//Font based functions
	int GetFontSize(int pt);
	int GetFontHeightFromPt(int sz);
	int GetFontPtFromHeight(int sz);
	int SetFontSize(int pt, int sz);
	int ZoomFont(int direction, int rettype);
	//For fonts that are not monospace we need to deal with the kerning
	void FreeKerningPairs()
	{
		//Free the kerning array, this happenes when the font changes and when the controle is unhooked
		if (m_KerningPairs != NULL)
		{
			delete[] m_KerningPairs;
			m_KerningPairs = NULL;
			m_NumKerningPairs = 0;
		}
	}
	//Get the kerning reference array
	void AllocKerningPairs(HDC hDC)
	{
		if (m_KerningPairs == NULL)
		{
			//how many kerning pairs are there for the font?
			m_NumKerningPairs = GetKerningPairs(hDC, 0, NULL);
			if (m_NumKerningPairs > 0)
			{
				//alloc array and store kering pairs
				m_KerningPairs = new KERNINGPAIR[m_NumKerningPairs];
				GetKerningPairs(hDC, m_NumKerningPairs, m_KerningPairs);
			}
		}
	}
	//check is a character combination has kerning info
	int GetKerning(HDC hDC, TCHAR Char1, TCHAR Char2)
	{
		//look through the kerning reference array, find the mathing pair and return the kerning
		int ret = 0;
		
		if (m_NumKerningPairs > 0)
		{
			for (int i = 0; i < m_NumKerningPairs; i++)
			{
				if (((m_KerningPairs + i)->wFirst == Char1) && ((m_KerningPairs + i)->wSecond == Char2))
				{
					
					ret = (m_KerningPairs + i)->iKernAmount;
					
					break;
				}
			}
		}
		return ret;
	}
	

	//Keyboard Handling/Caret/Line Movement
	short GetNextXCoord(HDC DrawDC, short xCoord, LONG CharPos, TCHAR Char, TCHAR Char2)
	{
		//this function gets the next x based on the current x position and the character to jump over, and the next character to draw
		if (Char != '\t') //if the character is not a tab, calculate it's width
		{
			//get the character width
			int w1;
			GetCharWidth32(DrawDC, Char, Char, &w1);
			//add to the xpostion andsubtract/add the kerning space between the 2 characters if any
			return xCoord + w1 + GetKerning(DrawDC, Char, Char2);
		}
		else //MUCH slower, if it's a tab, get the position of the next character after it relying on pos from char which is super slow in a loop
		{
			//hack, if tab and end of file, add \r\n so we dont confuse the caret system and so EM_POSFROMCHAR will succeed
			if (CharPos == m_TextLen-1)
			{
				LONG OldSelStart = m_SelStart;
				LONG OldSelEnd = m_SelEnd;
				RedrawStop();
				m_ScrollCaretEnabled = false;
				SendMessage(m_hWnd, EM_SETSEL, CharPos + 1, CharPos + 1);
				DirectSend(EM_REPLACESEL, 0, (LPARAM)L"\r\n");
				SendMessage(m_hWnd, EM_SETSEL, OldSelStart, OldSelEnd);
				m_ScrollCaretEnabled = true;
				m_TextLen = (LONG)DirectSend(WM_GETTEXTLENGTH, 0, 0);
				RedrawResume();
			}
			return LOWORD(DirectSend(EM_POSFROMCHAR, CLAMPEB(CharPos+1), 0));
		}
	}
	
	LONG SafeCharFromPos(short x, short y)
	{
		LRESULT data = (DirectSend(EM_CHARFROMPOS, 0, MAKELPARAM(x, y)));
		LONG Char = LOWORD(data);
		LONG Line = HIWORD(data);
		LONG Limit = (LONG) DirectSend(EM_GETLIMITTEXT, 0, 0);
		if (Limit < 65051 && Limit > 0)
		{
			return Char; //safe until 64k
		}

		//if it failed the char will be smaller than the first char on the line (the line is valid for far longer)
		//but not more than 64k lines... so here I use top + y to get the line
		Line = (LONG)DirectSend(EM_GETFIRSTVISIBLELINE, 0, 0) + y/this->m_textMetrics.tmHeight;
		LONG CharAtLine = (LONG)DirectSend(EM_LINEINDEX, min(Line, max(0,DirectSend(EM_GETLINECOUNT,0,0)-1)), 0);
		if (CharAtLine <= Char)
		{
			return Char; //looks Ok
		}
		//so we need to loop until we find the character, from the first char on the line found
		Char = CharAtLine;
		HLOCAL hMem = (HLOCAL)DirectSend(EM_GETHANDLE, 0, 0);
		if (hMem == NULL) return Char;
		TCHAR *Text = (TCHAR*)LocalLock(hMem);
		if (Text == NULL) return Char;
		short thisX;
		LONG RetChar = Char;
		bool SafeToCheckAhead = false;
		short RetCharX = LOWORD(DirectSend(EM_POSFROMCHAR, Char, 0));
		//loop through line chars until the x position of the charcter crosses the x coord passed
		while(1)
		{
			
			thisX = LOWORD(DirectSend(EM_POSFROMCHAR, Char, 0));
			if (thisX > x)
			{
				SafeToCheckAhead = true;
				break;
			}
			else if (Text[Char] == '\r') //quit at newline
			{
				RetChar = Char;
				break;
			}
			else if (Text[Char] == 0) //quit at null
			{
				RetChar = Char;
				break;
			}
			else if (Char == Limit) //end of box
			{
				RetChar = Char;
				break;
			}
			else
			{
				RetCharX = thisX;
				RetChar = Char;
			}
			Char++;
		}
		//if we did not hit the end, we are between chars, see if the left char (RetChar as RetCharX) is closer than the one on the right Char as thisX
		if (SafeToCheckAhead)
		{
			if (abs(x - RetCharX) > abs(x - thisX))
			{
				RetChar = Char;
			}
		}
		LocalUnlock(hMem);
		return RetChar;

	}
	 
	LPARAM SafePosFromChar(LONG CharPos, short ExtraXOff = 0, int ReturnEOLPlusNL = 0)
	{
		if (m_TextLen == 0) return MAKELPARAM(m_Margin, 0);
		else if (CharPos < m_TextLen) return DirectSend(EM_POSFROMCHAR, CharPos, 0);
		else
		{
			LPARAM pos = DirectSend(EM_POSFROMCHAR, m_TextLen - 1, 0);
			short x = LOWORD(pos),
				y = HIWORD(pos);
			bool HasNL = false;
			if (ReturnEOLPlusNL == 1)
			{
				HLOCAL hMem = (HLOCAL)DirectSend(EM_GETHANDLE, 0, 0);
				if (hMem != NULL)
				{
					TCHAR *Text = (TCHAR*)LocalLock(hMem);
					if (Text != NULL)
					{
						if (Text[m_TextLen - 1] == '\n') HasNL = true;
						LocalUnlock(hMem);

					}
				}
			}
			if(!HasNL)
				return MAKELPARAM(x + m_textMetrics.tmAveCharWidth  + ExtraXOff, y);
			else
				return MAKELPARAM(m_Margin  + ExtraXOff, y + m_textMetrics.tmHeight);
		}
		
		
	}

	bool HitLimit(LONG Pos) { return (Pos == 0 || Pos == m_TextLen); };

	LRESULT CaretMoveWord(int direction, int Select, int whitecharonly = 0) {
		//Left right word select.
		TurnCaretOn();


		//sanity check on text data
		if (m_TextLen == 0) return 0l;
		if (direction == -1 && m_SelEnd == 0) return 0l;
		else if (direction == 1 && m_SelEnd == (m_TextLen)) return 0l;
		HLOCAL hMem = (HLOCAL)DirectSend(EM_GETHANDLE, 0, 0);
		if (hMem == NULL) return 0l;
		TCHAR *Text = (TCHAR*)LocalLock(hMem);
		if (Text == NULL) return 0l;

		//is it a new line? we need to skip over that
		if (direction == 1 && m_CaretChar == '\r')
			m_SelEnd += 2;
		else if (direction == -1 && m_CaretPrevChar == '\n')
			m_SelEnd -= 2;
		else //if(!HitLimit(m_SelEnd))
		{
			if (direction == 1 && m_CaretChar == '\n') //between \r\n
				m_SelEnd += 1;
			else if (direction == -1 && m_CaretPrevChar == '\r') //between \r\n
				m_SelEnd -= 1;
			//Depending on the type of character we are on, loop until it's not that type of character we are on.
			ByteLookup LookUp = this->m_Language->SELECTBLOCKERS; //default to selection blockers

			LONG peek = 0;
			if (direction == -1) peek = -1;
			TCHAR InitialChar = Text[m_SelEnd + peek];

			if (whitecharonly)
			{
				LookUp = this->m_Language->WORDLISTSPLIT;
			}
			else if (LookUp.Valid(InitialChar))
			{
				LookUp = this->m_Language->SELECTREVERSEBLOCKERS; //unless we are on selection blockers
			}
			//loop until we are not on the same type of character
			bool Done = false;
			TCHAR Char = 0;
			//We can skip the first char yeah?
			m_SelEnd = CLAMPEB(m_SelEnd + direction);
			do
			{
				//we hit the end, break
				if (HitLimit(m_SelEnd))
					break;
				//read the character
				Char = Text[m_SelEnd + peek];
				//we hit a char blocker, done
				if (LookUp.Valid(Char))
				{
					//but not if it's the same character, so to skip over tabtabtab or == or // or spacespacespace
					if (InitialChar != Char)
						break;
				}
				//next position
				m_SelEnd = CLAMPEB(m_SelEnd + direction);

				InitialChar = Char; //remeber last character
			} while (true);
			//bugfix
			//fix the selection that may include a half selected newline (\r\n), that is text\r or \ntext is selected when it should not include either newline chars
			if (whitecharonly)
			{
				wstring t = this->GetSelText(m_SelStart, m_SelEnd);
				if (t != L"")
				{
					if (t.c_str()[0] == '\n')
					{
						if (m_SelStart < m_SelEnd)
						{
							m_SelStart++;
						}
						else
						{
							m_SelEnd++;
						}
					}
					if (t.c_str()[t.length() - 1] == '\r')
					{
						if (m_SelStart > m_SelEnd)
						{
							m_SelStart--;
						}
						else
						{
							m_SelEnd--;
						}
					}

				}
			}

		}
		LocalUnlock(hMem);
		RedrawStop();
		bool DoWordListSelect = false;
		if (Select) SendMessage(m_hWnd, EM_SETSEL, m_SelStart, m_SelEnd);
		else
		{
			SendMessage(m_hWnd, EM_SETSEL, m_SelEnd, m_SelEnd);
			DoWordListSelect = true;
		}
	
		RedrawResume();
		//move to caret
		SendMessage(m_hWnd, EM_SCROLLCARET, 0, 0);
		RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
		if (DoWordListSelect)
		{
			if (m_WordListDlg.IsVisible(true) && whitecharonly == 0)
			{
				DoWordList();
			}
		}
		//UpdateWindow(m_hWnd);
		return 0l;
	}
	LRESULT CaretMoveChar(int direction, int control, int Select) {
		if (control) return CaretMoveWord(direction, Select);
		//move by one character left or right, skip over \r\n
		TurnCaretOn();
		int moveamount = direction;
		if (moveamount == 1) 
			moveamount *= (m_CaretChar == '\r') ? 2 : 1;
		else if (moveamount == -1)
			moveamount *= (m_CaretPrevChar == '\n') ? 2 : 1;
		m_SelEnd = CLAMPEB(m_SelEnd + moveamount);
		//Call set sel, no drawing
		RedrawStop();
		bool DoWordListSelect = false;
		if (Select) SendMessage(m_hWnd, EM_SETSEL, m_SelStart, m_SelEnd);
		else
		{
			SendMessage(m_hWnd, EM_SETSEL, m_SelEnd, m_SelEnd);
			DoWordListSelect = true;
		}
		RedrawResume();
		//move to caret
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
		return 0;
	}
	LRESULT CaretMoveLine(int direction, int control, int Select) {
		TurnCaretOn();
		LONG LineAt = (LONG)DirectSend(EM_LINEFROMCHAR, m_SelEnd, 0);
		short CaretStartX = LOWORD(SafePosFromChar(m_SelEnd,0,1));
		LONG LineNext = LineAt += direction;
		LONG LineCount = (LONG)DirectSend(EM_GETLINECOUNT, 0, 0);
		if (LineNext < LineCount && LineNext >= 0)
		{
			LONG LineNextFirstChar = (LONG)DirectSend(EM_LINEINDEX, LineNext, 0);
			LONG LineNextLastChar = LineNextFirstChar + (LONG)DirectSend(EM_LINELENGTH, LineNextFirstChar, 0);
			LONG posAt = LineNextFirstChar;
			short xAt;
			short OldxAt = -999;
			while (posAt < LineNextLastChar)
			{
				xAt = LOWORD(SafePosFromChar(posAt));
				if (xAt >= CaretStartX)
				{
					if (OldxAt != -999)
					{
						if (abs(CaretStartX - OldxAt) < abs(CaretStartX - xAt))
						{
							posAt--;
						}
					}
					break;
				}
				OldxAt = xAt;
				posAt++;
			}
			RedrawStop();
			if (direction == -1 && control != 0)
				DirectSend(WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
			if (direction == 1 && control != 0)
				DirectSend(WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
			if (Select) SendMessage(m_hWnd, EM_SETSEL, m_SelStart, posAt);
			else SendMessage(m_hWnd, EM_SETSEL, posAt, posAt);
			RedrawResume();
			//move to caret
			SendMessage(m_hWnd, EM_SCROLLCARET, 0, 0);
			RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
		}
		return 0l;
		
	}
	LRESULT VPageScroll(int direction, int Select) {
		//Page UpDown
		//Works like the CaretMoveLine except for the the move is over by more that one line
		TurnCaretOn();
		//calculate the caret line offset from the top line
		LONG FirstVisibleLine = (LONG) DirectSend(EM_GETFIRSTVISIBLELINE, 0, 0);
		LONG CursorLine = (LONG)DirectSend(EM_LINEFROMCHAR, m_SelEnd, 0);
		if (CursorLine == -1) return 0l;
		RECT ClientRect; GetClientRect(m_hWnd, &ClientRect);
		LONG NumVisibleLines = ClientRect.bottom / m_textMetrics.tmHeight;
		LONG LastVisibleLine = FirstVisibleLine + NumVisibleLines;
		//first move if the caret in not in view, scroll to it
		if (CursorLine<FirstVisibleLine || CursorLine>LastVisibleLine)
		{
			//move to caret and leave
			SendMessage(m_hWnd, EM_SCROLLCARET, 0, 0);
			RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
			//UpdateWindow(m_hWnd);
			return 0l;
		}
		//line count from the top visible
		LONG DeltaLines = CursorLine - FirstVisibleLine;
		RedrawStop();
		//scroll
		short xInitial = LOWORD(DirectSend(EM_POSFROMCHAR, m_SelEnd, 0));
		if (direction == -1)
		{
			DirectSend(WM_VSCROLL, MAKEWPARAM(SB_PAGEUP, 0), 0);
			
		}
		else if (direction == 1)
		{
			DirectSend(WM_VSCROLL, MAKEWPARAM(SB_PAGEDOWN, 0), 0);
		}
		//not get the line at the offset from new top line, maching the position when thenew caret line will be
		LONG NewFirstVisibleLine = (LONG)DirectSend(EM_GETFIRSTVISIBLELINE, 0, 0);
		LONG ScrolledLine = NewFirstVisibleLine + DeltaLines;

		//From that line get the character and fint line in the CaretMoveLine, the character with matching x coord
		
		LONG posStart = (LONG)DirectSend(EM_LINEINDEX, ScrolledLine, 0);
		LONG pos = posStart;
		short xAt = LOWORD(DirectSend(EM_POSFROMCHAR, pos, 0));
		LONG LineEnd = pos + (LONG)DirectSend(EM_LINELENGTH, pos, 0);
		while (xAt < xInitial && pos <= LineEnd)
		{
			xAt = LOWORD(DirectSend(EM_POSFROMCHAR, pos++, 0));
		}
		pos = CLAMPEB(pos - 1 * (xAt >= xInitial));
		pos = max(posStart, min(LineEnd, pos));
		
		if (Select) SendMessage(m_hWnd, EM_SETSEL, m_SelStart, pos);
		else SendMessage(m_hWnd, EM_SETSEL, pos, pos);
		//Page scroll beyond range? do ctlr+Home/End to go to start or end of document
		if (NewFirstVisibleLine == FirstVisibleLine)
		{
			CaretMoveEndOfLine(direction, 1, Select);
		}
		RedrawResume();
		//move to caret
		SendMessage(m_hWnd, EM_SCROLLCARET, 0, 0);
		RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
		//UpdateWindow(m_hWnd);
		return 0l;
	}
	LRESULT HPageScroll(int direction, int Select) {
		//Do Horisontal page up and down, as cheap and dirty as the original edit box does. except my code can come back with page up
		RedrawStop();
		if (direction == -1)
		{
			DirectSend(WM_HSCROLL, MAKEWPARAM(SB_PAGEUP, 0), 0);

		}
		else if (direction == 1)
		{
			DirectSend(WM_HSCROLL, MAKEWPARAM(SB_PAGEDOWN, 0), 0);
		}
		RedrawResume();
		RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
		//UpdateWindow(m_hWnd);
		return 0l;
	}
	LRESULT PageScroll(int direction, int control, int Select) {
		m_WordListDlg.Hide();
		//Do H or V Page up down
		if (control == 0) VPageScroll(direction, Select);
		else HPageScroll(direction, Select);

		return 0l;
	}
	LRESULT CaretMoveEndOfLine(int direction, int control, int Select) {
		//home,end actions
		TurnCaretOn();
		LONG pos = 0; //default start (ctrl home)
		if (direction == 1 && control != 0) 
			pos = m_TextLen; //ctrl end, set pos to end of document
		else if (direction == 1 && control == 0)
		{
			//End, set pos to character at the end of the current line
			LONG LineEnd = (LONG)DirectSend(EM_LINEINDEX, DirectSend(EM_LINEFROMCHAR, m_SelEnd, 0), 0) + (LONG)DirectSend(EM_LINELENGTH, m_SelEnd, 0);
			pos = LineEnd+1;
			pos = CLAMPEB(pos);

			
			
			//fiddle a bit the position if word wrap is on because some line dont have \r\n and the reported line length 
			//combined with the set sel ability may cause it to go a little more to the right, wrapping to the next line
			//which we can see if the ycoord on the final position is not the same as the original position
			if (m_IsWordWrap)
			{
				short yInitial = HIWORD(DirectSend(EM_POSFROMCHAR, m_SelEnd,0));
				short yNow = HIWORD(DirectSend(EM_POSFROMCHAR, pos,0));
				if (yInitial != yNow) pos-=2;
			}
			if (pos < LineEnd-1) pos = LineEnd;
			
			//Now make sure we arent between \r\n, an impercievable error that will cause trouble. so far only the "end" feature sometimes does that
			HLOCAL hMem = (HLOCAL)DirectSend(EM_GETHANDLE, 0, 0);
			if (hMem != NULL)
			{
				TCHAR *Text = (TCHAR*)LocalLock(hMem);
				if (Text != NULL)
				{
					if (Text[pos] == '\n') pos--;
					LocalUnlock(hMem);

				}
			}
		}
		else if (direction == -1 && control == 0) //Home, set pos to first character on the line
			pos = (LONG) DirectSend(EM_LINEINDEX, DirectSend(EM_LINEFROMCHAR, m_SelEnd, 0), 0);
		pos = CLAMPEB(pos);
		RedrawStop();
		if (Select) SendMessage(m_hWnd, EM_SETSEL, m_SelStart, pos);
		else SendMessage(m_hWnd, EM_SETSEL, pos, pos);
		//move to caret
		RedrawResume();
		SendMessage(m_hWnd, EM_SCROLLCARET, 0, 0); 
		RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
		//UpdateWindow(m_hWnd);
		return 0l;
	}
	//Editing Functions
	LRESULT DeleteChar(int direction, int control, UINT message = 0, WPARAM wParam = 0, LPARAM lParam = 0)
	{
		if (m_IsReadOnly) return 0l;
		if (m_SelStart != m_SelEnd) //has selection
			;
		else if (control) //delete word
			CaretMoveWord(direction, 1);
		else //delete character
			SelectNextChar(direction);
		//Do it, cause refresh
		this->AddUndoableContent(L"",0,0, message, wParam, lParam);
		
		return 0L;
	}
	LRESULT DoTabChar(int shift, UINT message = 0, WPARAM wParam = 0, LPARAM lParam = 0)
	{
		if (m_IsReadOnly) return 0l;
		if (DirectSend(EM_LINEFROMCHAR, m_SelStart, 0) != DirectSend(EM_LINEFROMCHAR, m_SelEnd, 0))
			SendMessage(m_hWnd, UM_BLOCKINDENT, (shift) ? 0 : 1, 0); //multi line selection indent in or out
		else if (shift)
		{
			if (m_SelStart == m_SelEnd)
			{
				if (m_CaretPrevChar == '\t') //previous is tab, delete it
					DeleteChar(-1, 0);
				else
				{
					//try the darn space used as tabs
					wstring prevtext;
					
					LONG SeekPos = m_SelEnd;
					repeat(4)
					{
						//accumulate in reverse the characters on the left
						prevtext += GetSelText(CLAMPEB(SeekPos), CLAMPEB(SeekPos-1));
						SeekPos--;
					}
					
					
					LONG len = (LONG) prevtext.length();
					if (len) //the line has length
					{
						const TCHAR *str = prevtext.c_str(); //get the buffer for easier check
						//The characters left to right should be what we got right to left
						//the string may hold 4 to 1 a characters at this point, see if they are spaces in a possible group of 4, 3, 2 or 1 spaces
						if (len > 3 && str[0] == ' ' && str[1] == ' ' && str[2] == ' ' && str[3] == ' ')
						{
							RedrawStop();
							SendMessage(m_hWnd, EM_SETSEL, m_SelEnd-4, m_SelEnd);
							RedrawResume();
							this->AddUndoableContent(L"");
						}
						else if (len > 2 && str[0] == ' ' && str[1] == ' ' && str[2] == ' ')
						{
							RedrawStop();
							SendMessage(m_hWnd, EM_SETSEL, m_SelEnd - 3, m_SelEnd);
							RedrawResume();
							this->AddUndoableContent(L"");
						}
						else if (len > 1 && str[0] == ' ' && str[1] == ' ')
						{
							RedrawStop();
							SendMessage(m_hWnd, EM_SETSEL, m_SelEnd - 2, m_SelEnd);
							RedrawResume();
							this->AddUndoableContent(L"");
						}
						else if (len > 0 && str[0] == ' ')
						{
							RedrawStop();
							SendMessage(m_hWnd, EM_SETSEL, m_SelEnd - 1, m_SelEnd);
							RedrawResume();
							this->AddUndoableContent(L"");
						}
					}
					
				}
			}
			else
			{
				this->AddUndoableContent(L""); //Shift tab on selected text, make it do backspace/delete
			}
		}
		else
			this->AddUndoableContent(L"\t",0,0, message, wParam, lParam); //do a tab
		return 0l;
	}
	TCHAR WhatBrace(TCHAR c)
	{
		TCHAR braceChars[] = { '{','}','[',']','(',')','<','>','"','\'' };
		TCHAR found = 0;

		for (int i = 0; i < 10; i++)
		{
			if (c == braceChars[i])
			{
				return  braceChars[i];
			}
		}
		return found;

	}
	TCHAR WhatTargetBrace(TCHAR c)
	{
		TCHAR braceChars[] = { '{','}',
			'}','{',
			'[',']',
			']','[',
			'(',')',
			')','(',
			'<','>',
			'>','<',
			'"','"',
			'\'','\'' };
		TCHAR found = 0;

		for (int i = 0; i < 10; i++)
		{
			if (c == braceChars[i * 2])
			{
				return braceChars[i * 2 + 1];
			}
		}
		return found;

	}

	
	LRESULT MatchBracket(int shift = 0) {
		//create a string directly from the memory handle
		HLOCAL hMem = (HLOCAL)DirectSend(EM_GETHANDLE, 0, 0);
		if (hMem == NULL) return -1l;
		TCHAR *Text = (TCHAR*)LocalLock(hMem);
		if (Text == NULL) return -1l;

		//find the neared brace on left or right and dertine the direction
		int dir = 0;
		int leftrightOfCarret = 1; //1 found on the right of caret, -1 found on the left
		TCHAR SearchStartChar = WhatBrace(Text[m_SelEnd]); // |{brace}
		if (SearchStartChar == '}')
			dir = -1;
		else if (SearchStartChar == ']')
			dir = -1;
		else if (SearchStartChar == ')')
			dir = -1;
		else if (SearchStartChar == '>')
			dir = -1;
		else if (SearchStartChar == '{')
			dir = 1;
		else if (SearchStartChar == '[')
			dir = 1;
		else if (SearchStartChar == '(')
			dir = 1;
		else if (SearchStartChar == '<')
			dir = 1;
		else if (SearchStartChar == '\'')
			dir = 1;
		else if (SearchStartChar == '"')
			dir = 1;
		if (dir == 0)
		{
			leftrightOfCarret = -1;
			SearchStartChar = WhatBrace(Text[max(0, m_SelEnd - 1)]); // {brace}|
			if (SearchStartChar == '}')
				dir = -1;
			else if (SearchStartChar == ']')
				dir = -1;
			else if (SearchStartChar == ')')
				dir = -1;
			else if (SearchStartChar == '>')
				dir = -1;
			else if (SearchStartChar == '{')
				dir = 1;
			else if (SearchStartChar == '[')
				dir = 1;
			else if (SearchStartChar == '(')
				dir = 1;
			else if (SearchStartChar == '<')
				dir = 1;
			else if (SearchStartChar == '\'')
				dir = -1;
			else if (SearchStartChar == '"')
				dir = -1;

		}

		if (dir == 0)
		{
			LocalUnlock(hMem);
			return -1L;
		}
		//setup the position for the start of search
		size_t pos = m_SelEnd;
		if (leftrightOfCarret == 1 && dir == -1) pos += -1;
		if (leftrightOfCarret == 1 && dir == 1) pos += 1;
		if (leftrightOfCarret == -1 && dir == -1) pos += -2;
		if (leftrightOfCarret == -1 && dir == 1) pos += 0;
		if (pos < 0)
		{
			LocalUnlock(hMem);
			return -1l;
		}

		TCHAR TargetBrace = WhatTargetBrace(SearchStartChar);
		int foundct = 1;
		//find the next matching brace in direction, skipping over recursive items, like when |{ {{ }} }
		do {

			if (Text[pos] == TargetBrace) foundct--;
			else if (Text[pos] == SearchStartChar) foundct++;
			pos += dir;
		} while (pos >= 0 && Text[pos] != '\0' && foundct >0);
		//found it! set the cursor at the right spot
		if (foundct == 0)
		{
			if (dir == 1 && Text[pos] == '\n') pos--;
			else if (dir == -1 && Text[pos] == '\n') pos++;
			else if (dir == -1) pos++;



			m_SelEnd = (LONG)pos;
			//make sure we did not fall between \r\n
			if (!shift) m_SelStart = m_SelEnd;
			TurnCaretOn();
			SendMessage(m_hWnd, EM_SETSEL, m_SelStart, m_SelEnd);
			//RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);


		}
		LocalUnlock(hMem);
		return m_SelEnd;

	};
	LRESULT BlockIndent(WPARAM indent)
	{
		if (m_IsReadOnly) return 0l;
		//indent unindent using tabs
		RedrawStop();
		m_ScrollCaretEnabled = false;
		wstring content;
		LONG SelStart = m_SelStart;
		LONG SelEnd = m_SelEnd;
		LONG TempSelStart = SelStart;
		LONG TempSelEnd = SelEnd;
		if (TempSelStart > TempSelEnd) swap(TempSelStart, TempSelEnd);

		LONG LineFirst = (LONG)DirectSend(EM_LINEFROMCHAR, TempSelStart, 0);
		LONG FirstLineFirstPos = (LONG)DirectSend(EM_LINEINDEX, LineFirst, 0);
		LONG LineLast = (LONG)DirectSend(EM_LINEFROMCHAR, TempSelEnd, 0);
		m_SelStart = FirstLineFirstPos;
		m_SelEnd = FirstLineFirstPos;
		//If indenting 
		if (indent)
		{
			//For each line get the line text and prefix tab to it. assemble a final content string
			for (LONG i = LineFirst; i <= LineLast; i++)
			{
				m_SelStart = (LONG)DirectSend(EM_LINEINDEX, i, 0);
				m_SelEnd = m_SelStart;
				CaretMoveEndOfLine(1, 0, 1);
				content += L"\t";
				content += GetSelText();
				if (i < LineLast)
				{
					content += L"\r\n";
				}
			}

		}
		else
		{
			//For each line get the line text and remove the tab (or space used as tabs) prefix from it. assemble a final content string
			wstring ThisLine;
			LONG len;
			for (LONG i = LineFirst; i <= LineLast; i++)
			{
				m_SelStart = (LONG)DirectSend(EM_LINEINDEX, i, 0);
				m_SelEnd = m_SelStart;
				CaretMoveEndOfLine(1, 0, 1);
				
				ThisLine = GetSelText();
				len = (LONG)ThisLine.length();
				if (len) //the line has length
				{
					const TCHAR *str = ThisLine.c_str(); //get the buffer for easier check
					if (str[0] == '\t') //is firs char in line a tab? remove it
					{
						ThisLine = ThisLine.erase(0, 1); //erase it
					} //and do the same in the case of seeing 4,3,2 or 1 spaces
					else if (len > 3 && str[0] == ' ' && str[1] == ' ' && str[2] == ' ' && str[3] == ' ')
					{
						ThisLine = ThisLine.erase(0, 4);
					}
					else if (len > 2 && str[0] == ' ' && str[1] == ' ' && str[2] == ' ')
					{
						ThisLine = ThisLine.erase(0, 3);
					}
					else if (len > 1 && str[0] == ' ' && str[1] == ' ')
					{
						ThisLine = ThisLine.erase(0, 2);
					}
					else if (len > 0 && str[0] == ' ')
					{
						ThisLine = ThisLine.erase(0, 1);
					}
				}
				content += ThisLine;
				if (i < LineLast)
				{
					content += L"\r\n";
				}
			}
		}
		//select the text from the start to the end, m_SelEnd was set by the last CaretMoveEndOfLine
		SendMessage(m_hWnd, EM_SETSEL, FirstLineFirstPos, m_SelEnd);
		this->AddUndoableContent(content, true);
		//Why is it working!!! Oh, AddUndoableContent sets m_SelEnd to the right spot after replacing. I had an offset variable and debugged this for an hour before the facepalm
		//so we have a valid start and end that hold the entire content replaced, flip it so caret is on left edge
		SendMessage(m_hWnd, EM_SETSEL, m_SelEnd, FirstLineFirstPos); 
		RedrawResume();
		//hug the rail man
		DirectSend(WM_HSCROLL, MAKEWPARAM(SB_LEFT, 0), 0);
		RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
		m_ScrollCaretEnabled = true;

		
		return 0l;
	}

	LRESULT BlockComment(WPARAM indent)
	{
		if (m_IsReadOnly) return 0l;
		//indent unindent using tabs
		RedrawStop();
		m_ScrollCaretEnabled = false;
		wstring content;
		LONG SelStart = m_SelStart;
		LONG SelEnd = m_SelEnd;
		LONG TempSelStart = SelStart;
		LONG TempSelEnd = SelEnd;
		if (TempSelStart > TempSelEnd) swap(TempSelStart, TempSelEnd);

		LONG LineFirst = (LONG)DirectSend(EM_LINEFROMCHAR, TempSelStart, 0);
		LONG FirstLineFirstPos = (LONG)DirectSend(EM_LINEINDEX, LineFirst, 0);
		LONG LineLast = (LONG)DirectSend(EM_LINEFROMCHAR, TempSelEnd, 0);
		m_SelStart = FirstLineFirstPos;
		m_SelEnd = FirstLineFirstPos;
		//If indenting 
		if (indent)
		{
			//For each line get the line text and prefix tab to it. assemble a final content string
			for (LONG i = LineFirst; i <= LineLast; i++)
			{
				m_SelStart = (LONG)DirectSend(EM_LINEINDEX, i, 0);
				m_SelEnd = m_SelStart;
				CaretMoveEndOfLine(1, 0, 1);
				content += m_Language->SINGLELINECOMMENT.GetValue();
				content += GetSelText();
				if (i < LineLast)
				{
					content += L"\r\n";
				}
			}

		}
		else
		{
			//For each line get the line text and remove the tab (or space used as tabs) prefix from it. assemble a final content string
			wstring ThisLine;
			LONG len;
			for (LONG i = LineFirst; i <= LineLast; i++)
			{
				m_SelStart = (LONG)DirectSend(EM_LINEINDEX, i, 0);
				m_SelEnd = m_SelStart;
				CaretMoveEndOfLine(1, 0, 1);

				ThisLine = GetSelText();
				len = (LONG)ThisLine.length();
				if (len) //the line has length
				{
					if(m_Language->SINGLELINECOMMENT.Valid((TCHAR*)ThisLine.c_str()))
					{
						ThisLine = ThisLine.erase(0, m_Language->SINGLELINECOMMENT.GetLength()); //erase it
					} //and do the same in the case of seeing 4,3,2 or 1 spaces
				}
				content += ThisLine;
				if (i < LineLast)
				{
					content += L"\r\n";
				}
			}
		}
		//select the text from the start to the end, m_SelEnd was set by the last CaretMoveEndOfLine
		SendMessage(m_hWnd, EM_SETSEL, FirstLineFirstPos, m_SelEnd);
		this->AddUndoableContent(content, true);
		//Why is it working!!! Oh, AddUndoableContent sets m_SelEnd to the right spot after replacing. I had an offset variable and debugged this for an hour before the facepalm
		//so we have a valid start and end that hold the entire content replaced, flip it so caret is on left edge
		SendMessage(m_hWnd, EM_SETSEL, m_SelEnd, FirstLineFirstPos);
		RedrawResume();
		//hug the rail man
		DirectSend(WM_HSCROLL, MAKEWPARAM(SB_LEFT, 0), 0);
		RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
		m_ScrollCaretEnabled = true;


		return 0l;
	}
	LRESULT DoBlockCommentKey(int control, int shift)
	{
		if (control == 0) return 1l; //not processed
		SendMessage(m_hWnd, UM_BLOCKCOMMENT, (shift) ? 0 : 1, 0); //multi line selection indent in or out
		return 0l;
	}
	
	LRESULT DoUndoKey(int control)
	{
		if (control == 0) return 1l; //not processed
		SendMessage(m_hWnd, WM_UNDO,0,0); //Send Undo Message
		return 0l;
	}
	LRESULT DoRedoKey(int control)
	{
		if (control == 0) return 1l; //not processed
		SendMessage(m_hWnd, UM_REDO, 0, 0); //Send Redo Message
		return 0l;
	}
	LRESULT DoCopyKey(int control)
	{
		if (control == 0) return 1l; //not processed
		SendMessage(m_hWnd, WM_COPY, 0, 0); //Send Copy Message
		return 0l;
	}
	LRESULT DoPasteKey(int control)
	{
		if (control == 0) return 1l; //not processed
		SendMessage(m_hWnd, WM_PASTE, 0, 0); //Send Past Message
		return 0l;
	}
	
	LRESULT DoInsertKey()
	{
		SendMessage(m_hWnd, UM_SETINSERTMODE, (m_InsertMode) ? 0 : 1, 0); //Send Past Message
		return 0l;
	}
	LRESULT DoCutKey(int control)
	{
		if (control == 0) return 1l; //not processed
		SendMessage(m_hWnd, WM_CUT, 0, 0); //Send Cut Message
		return 0l;
	}
	LRESULT DoWordList();
	//Code box specific features
	LRESULT CanSearchText()
	{
		return m_SelStart != m_SelEnd;
	}

	bool WordEdge(LONG WordEdgeCharPos, LONG CharPosNextToIt)
	{
		if (CharPosNextToIt <= 0) return true;
		if (CharPosNextToIt >= m_TextLen) return true;
		HLOCAL hMem = (HLOCAL)DirectSend(EM_GETHANDLE, 0, 0);
		if (hMem == NULL) return false;
		TCHAR *Text = (TCHAR*)LocalLock(hMem);
		if (Text == NULL) return false;

		//check the type of character at the edge
		ByteLookup LookUp = this->m_Language->SELECTBLOCKERS; //default to selection blockers

		TCHAR EdgeChar = Text[WordEdgeCharPos];
		if (LookUp.Valid(EdgeChar))
		{
			//swap to invert lookup type if it's a character that is word breaking
			//like the user is searching <tab>Something or == or something not normally a alphanumeric word
			LookUp = this->m_Language->SELECTREVERSEBLOCKERS; //unless we are on selection blockers
		}
		//the the character next to the edge of the word
		TCHAR NextToEdgeChar = Text[CharPosNextToIt];
		LocalUnlock(hMem);
		//ruturn true if it's a word breaker
		return LookUp.Valid(NextToEdgeChar);
	}
	bool SelectedTextIsSameAs(wstring SearchText)
	{
		if (m_SelStart == m_SelEnd) return false;
		wstring SelectedText = GetSelText();
		bool wholeword = m_SearchDlg.m_SearchWordBut.State;
		if (wholeword)
		{
			LONG start = m_SelStart;
			long end = m_SelEnd;
			if(start > end) swap(start, end);
			RedrawStop();
			m_ScrollCaretEnabled = false;
			bool ValidWord = WordEdge(start, start - 1) && WordEdge(end, end + 1);
			m_ScrollCaretEnabled = true;
			RedrawResume();
			if (!ValidWord) return false;
		}
		bool sensitive = m_SearchDlg.m_SearchCaseBut.State;
		if (sensitive)
			return SelectedText == SearchText;
		else
		{
			LONG slen = (LONG) SearchText.length();
			return ((SelectedText.length() == slen) && (_wcsnicmp(SelectedText.c_str(), SearchText.c_str(), slen) == 0));
		}

	}
	LRESULT SearchText(int direction, bool sensitive, bool wholeword, wstring text = L"")
	{
		if(!CanSearchText() && !text.length())
			return -1;
		LONG start = min(m_SelStart, m_SelEnd);
		LONG end = max(m_SelStart, m_SelEnd);
		LONG InitialSelStart = m_SelStart;
		LONG InitialSelEnd = m_SelEnd;
		wstring SearchFor = GetSelText(start, end);
		start += direction;
		end += direction;
		if (text.length())
		{
			SearchFor = text;
			if (start == end)
			{
				start -= direction;
				start = max(0,min(start, m_TextLen - (LONG)text.length()));
			}
			end = start + (LONG) SearchFor.length();
		}
		if (SearchFor == L"") return -1;
		LONG EbLen = m_TextLen;
		LONG slen = (long)SearchFor.length();
		m_ScrollCaretEnabled = false;
		bool foundit = false;
		RedrawStop();
		bool ValidWord;
		while (start >= 0 && end <= EbLen)
		{
			ValidWord = true;
			wstring Found = GetSelText(start, end);
			if (wholeword)
			{
				ValidWord = WordEdge(start, start - 1) && WordEdge(end, end + 1);
			}
			if (ValidWord)
			{
				if (sensitive)
				{

					if (Found == SearchFor)
					{
						foundit = true;
						SendMessage(m_hWnd, EM_SETSEL, start, end);
						break;
					}
				}
				else
				{
					if ((Found.length() == slen) && (_wcsnicmp(Found.c_str(), SearchFor.c_str(), slen) == 0))
					{
						foundit = true;
						SendMessage(m_hWnd, EM_SETSEL, start, end);
						break;
					}
				}
			}
			start += direction;
			end += direction;
		}
		if (!foundit)
		{
			SendMessage(m_hWnd, EM_SETSEL, InitialSelStart, InitialSelEnd);
		}
		else
		{
			SendMessage(m_hWnd, EM_SETSEL, start, end);
		}
		RedrawResume();
		
		TurnCaretOn();
		m_ScrollCaretEnabled = true;
		
		SendMessage(m_hWnd, EM_SCROLLCARET, 0, 0);
		RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
		if (foundit == true)
		{
			return start;
		}
		return -1;
	}
	void FreeColorSpecs()
	{
		if (m_Colors.DlgBkColorBrush)
		{
			DeleteObject(m_Colors.DlgBkColorBrush);
			m_Colors.DlgBkColorBrush = NULL;
		}
		m_Colors.DlgBkColor = 0;
		if (m_Colors.CtrlBkColorBrush)
		{
			DeleteObject(m_Colors.CtrlBkColorBrush);
			m_Colors.CtrlBkColorBrush = NULL;
		}
		m_Colors.CtrlTextColor = 0;
		m_Colors.CtrlBkColor = 0;

		if (m_Colors.CtrlButColorBrush)
		{
			DeleteObject(m_Colors.CtrlButColorBrush);
			m_Colors.CtrlButColorBrush = NULL;
		}
		m_Colors.CtrlButColor = 0;

		if (m_Colors.CtrlButTextColorBrush)
		{
			DeleteObject(m_Colors.CtrlButColorBrush);
			m_Colors.CtrlButTextColorBrush = NULL;
		}
		m_Colors.CtrlButTextColor = 0;

		if (m_Colors.CtrlButHoverColorBrush)
		{
			DeleteObject(m_Colors.CtrlButHoverColorBrush);
			m_Colors.CtrlButHoverColorBrush = NULL;
		}
		m_Colors.CtrlButHoverColor = 0;

		if (m_Colors.CtrlButDownColorBrush)
		{
			DeleteObject(m_Colors.CtrlButDownColorBrush);
			m_Colors.CtrlButDownColorBrush = NULL;
		}
		m_Colors.CtrlButDownColor = 0;
	}
	void InitColorSpecs()
	{
		FreeColorSpecs();
		m_Colors.DlgBkColor = 0x00302D2D;
		m_Colors.DlgBkColorBrush = CreateSolidBrush(m_Colors.DlgBkColor);

		m_Colors.CtrlTextColor = 0x00C8C8C8;
		m_Colors.CtrlBkColor = 0x00463F3F;
		m_Colors.CtrlBkColorBrush = CreateSolidBrush(m_Colors.CtrlBkColor);

		m_Colors.CtrlButColor = 0x00757575;
		m_Colors.CtrlButColorBrush = CreateSolidBrush(m_Colors.CtrlButColor);
		m_Colors.CtrlButTextColor = 0x00C8C8C8;
		m_Colors.CtrlButTextColorBrush = CreateSolidBrush(m_Colors.CtrlButTextColor);
		

		m_Colors.CtrlButHoverColor = 0x00463F3F;
		m_Colors.CtrlButHoverColorBrush = CreateSolidBrush(m_Colors.CtrlButHoverColor);

		m_Colors.CtrlButDownColor = 0x00FF9933;
		m_Colors.CtrlButDownColorBrush = CreateSolidBrush(m_Colors.CtrlButDownColor);
		
	}
	//Hooked Unhooked
	void OnHooked() {
		//Initial (re-)Configuration
		//remember stuff
		SaveTabStops();
		SaveMargins();
		SetTabStops();
		SaveFont();
		m_OEBS.BlinkTime = GetCaretBlinkTime();
		//caret timer
		m_CaretTimer = (int)SetTimer(m_hWnd, 6502, m_OEBS.BlinkTime, 0);
		//byebye stupid caret
		if (GetFocus() == m_hWnd) DestroyCaret();
		//get the selection for my variables
		DirectSend(EM_GETSEL, (WPARAM)&m_SelStart, (LPARAM)&m_SelEnd);
		//SetCaretBlinkTime(INFINITE);
		DWORD Styles = (DWORD) GetWindowLongPtr(m_hWnd, GWL_STYLE);
		m_IsWordWrap =  (Styles& ES_AUTOHSCROLL) != ES_AUTOHSCROLL;
		m_IsReadOnly = (Styles & ES_READONLY) == ES_READONLY;

		Styles |= WS_CLIPCHILDREN;
		SetWindowLongPtr(m_hWnd, GWL_STYLE, Styles);

		//no soft linebreaks
		DirectSend(EM_FMTLINES, FALSE, 0);
		InitColorSpecs();
		if (IsWindowVisible(m_hWnd))
		{
			SendMessage(m_hWnd, WM_SIZE, 0, 0);
			RedrawWindow(m_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);
			//UpdateWindow(m_hWnd);
		}
	}
	void OnUnHook()
	{
		//reset the edit box to initial state
		FreeUndoRedo();
		m_SearchDlg.Hide();
		m_SearchDlg.Kill();
		m_WordListDlg.Hide();
		m_WordListDlg.Kill();
		FreeColorSpecs();
		FreeKerningPairs();
		if (IsWindow(m_hWnd)) 
		{
			if (m_CaretTimer != -1)
			{
				KillTimer(m_hWnd, m_CaretTimer);
				m_CaretTimer = -1;
			}
			
			
			//SetCaretBlinkTime(m_OEBS.BlinkTime);
			RestoreMargins();
			RestoreTabStops();
			RestoreWindowProc();
			RestoreFont();
			//reset the stupid caret
			if (GetFocus() == m_hWnd)
			{
				//HFONT hFont = (HFONT)DirectSend(WM_GETFONT, 0, 0);
				//LOGFONT fontAttributes = { 0 };
				//GetObject(hFont, sizeof(fontAttributes), &fontAttributes);
				//CreateCaret(
				//	m_hWnd,
				//	NULL,
				//	2,
				//	fontAttributes.lfHeight
				//);
				//ShowCaret(m_hWnd);
				//even simpler
				SendMessage(m_hWnd, WM_SETFOCUS, 0, 0);
				//ShowCaret(m_hWnd);

			}
			
			
			if (IsWindowVisible(m_hWnd))
			{

				//SetWindowLong(m_hWnd, GWL_WNDPROC, (LONG_PTR)m_origWndProc);
				RedrawWindow(m_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);
				//UpdateWindow(m_hWnd);
			}
		}
	}
	LONG CountLinesOFW(const TCHAR *txt, LONG endPos, LONG *PosLast)
	{
		//returns the number of line from the start of the text to endPos (exclusive), return values are 1, obviously, to N
		LONG ct = 0;
		LONG StartOfLinePos = 0;
		//also return the character position of the first character after the \r\n so we can adjust were we start drawing from in paint code
		*PosLast = 0;
		for (LONG i = 0; i < endPos; i++)
		{
			if (txt[i] == '\r')
			{
				ct++;
				*PosLast = StartOfLinePos;
				StartOfLinePos = i + 2;
				
			}

		}
		return max(1,ct);
	}
};


