#pragma once
#include "Parser.h"
#define CAP_0_64k(n) max(0,min(n,65535))
#define CAP_0_EB_SIZE(n) max(0,min(n,DirectSendMessage(WM_GETTEXTLENGTH), 0, 0)))
#define CLAMP(n1,n2,n3) max(n1,min(n2,n3))
#define KEY_STATE_DOWN(n) (GetKeyState(n) & 0x8000)
#define EB_SET_SEL(s,e) DirectSendMessage(EM_SETSEL, s,e)
#define EB_SEL_START_FROM_LRESULT(n) LOWORD(n)
#define EB_SEL_END_FROM_LRESULT(n) HIWORD(n)
#define EB_GET_SEL() DirectSendMessage(EM_GETSEL, 0, 0)
class EditAction
{
public:
	int m_SelStart;
	TCHARString m_OrigSelText;
	TCHARString m_NewText;
	EditAction(int SelPos, TCHARString OldSelText, TCHARString NewText)
	{
		m_SelStart = SelPos;
		m_OrigSelText = OldSelText;
		m_NewText = NewText;
	}
};
typedef stack<EditAction*> EditActions;

class CharFromPos
{
public:
	int CharPos = 0;
	int LinePos = 0;
};
class PosFromChar
{
public:
	short x = 0;
	short y = 0;
};
class HookedCtrl
{
private:
	HWND m_hWnd = NULL;
	WNDPROC m_origWndProc = NULL;
	int m_SelStart = 0;
	int m_SelEnd = 0;
	short m_TabSize = 16;
	TCHAR m_EBText[66536]; //not a typo, allocate more to allow overun in the parser searching for text, 1000 more bytes should be enough
	TCHAR m_EBTextMirror[65536];
	TCHAR m_EBLine[65536];
	COLORREF m_CharColors[65536];
	int m_TextHeight = 10;
	UINT_PTR m_TimerID = -1;
	int m_scrollHDirection = 0;
	int m_scrollVDirection = 0;
	int m_ScrollMouseX = 0;
	int m_ScrollMouseY = 0;
	bool InPopUpMenu = false;
	bool m_SyntaxHighlighting = true;
	bool m_WordSelectDrag = false;
	EditActions UndoStack;
	EditActions RedoStack;
	Parser m_Parser;
	COLORREF m_LineNumberBackColor = 0x272727;
	COLORREF m_LineNumberColor =  0x4A4A4A;
	bool m_DoLineNumbers = true;
	bool m_lMouseIsDown = false;

public:
	Language * m_Language = NULL;

	HookedCtrl(HWND hwnd);
	~HookedCtrl();

	LRESULT DirectSendMessage(UINT message, WPARAM wParam, LPARAM lParam);

	LRESULT DoWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);


	LRESULT Do_WM_PAINT(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	WNDPROC GetOrigProc() { return m_origWndProc; }

	void SetOrigProc(WNDPROC origWndProc) { m_origWndProc = origWndProc; }

	void OnHooked(DWORD LineNumberBackColor, DWORD LineNumberColor);

	void OnUnHook();

	void SetLanguage(Language *Language) { m_Language = Language; };

	void InvalidateSelectRegion(int SelStart, int SelEnd);

	void InvalidateSelectRegion() { InvalidateSelectRegion(m_SelStart, m_SelEnd); };

	void SetSelection(int SelStart, int SelEnd) {
		m_SelStart = CAP_0_64k(SelStart);
		m_SelEnd = CAP_0_64k(SelEnd);
		DirectSendMessage(EM_SETSEL, m_SelEnd, m_SelEnd);
	};

	int GetLineCount() { return DirectSendMessage(EM_GETLINECOUNT, NULL, NULL); };

	int GetLineLengthFromPos(int CharPos) { return DirectSendMessage(EM_LINELENGTH, CharPos, NULL); };

	int GetLineFromPos(int CharPos) { return DirectSendMessage(EM_LINEFROMCHAR, CharPos, 0); };

	int GetFirstPosInLine(int LinePos) { return DirectSendMessage(EM_LINEINDEX, LinePos, 0); }

	int GetTextLenght() { return DirectSendMessage(WM_GETTEXTLENGTH, 0, 0); };

	int GetFirstVisibleLine() { return DirectSendMessage(EM_GETFIRSTVISIBLELINE, 0, 0); };

	RECT GetClientRect() { RECT rect; DirectSendMessage(EM_GETRECT, 0, (LPARAM)&rect); return rect; };

	void CaretSetPosition(int Pos, int Select = 0) {
		int OldSelStart = m_SelStart;
		SetSelection(Pos, Pos);
		if (Select) m_SelStart = OldSelStart;
	};
	bool CapSelEndToTextLimit()
	{
		int MaxSize = GetTextLenght();
		m_SelEnd = CLAMP(0, MaxSize, m_SelEnd);
		return (m_SelEnd == 0 || m_SelEnd == MaxSize);
	}
	bool CaretMoveChar(int direction, int Word = 0, int Select = 0) {
		int MaxSize = GetTextLenght();
		if (Word == 0)
		{
			m_SelEnd += direction;
			bool HitEnd = CapSelEndToTextLimit();
			if (m_EBText[m_SelEnd] == '\r' || m_EBText[m_SelEnd] == '\n') m_SelEnd += direction;
			CaretSetPosition(m_SelEnd, Select);
			return HitEnd;
		}
		else
		{
			int rightoff;
			int leftoff;
			bool LeftValid;
			bool rightvalid;
			bool HitEnd;
			ByteLookup LookUp = this->m_Language->SELECTBLOCKERS;
			rightoff = CLAMP(0, MaxSize, m_SelEnd + direction);
			leftoff = CLAMP(0, MaxSize, m_SelEnd);
			rightvalid = LookUp.Valid(m_EBText[rightoff]);
			LeftValid = LookUp.Valid(m_EBText[leftoff]);
			if (direction == 1 && m_EBText[m_SelEnd] == '\r') m_SelEnd += 1;
			if (direction == -1 && m_EBText[max(-0,m_SelEnd-1)] == '\n') m_SelEnd -= 1;
			if (LeftValid && rightvalid)
			{
				LookUp = this->m_Language->SELECTREVERSEBLOCKERS;
			}
			if(direction == 1) m_SelEnd = CLAMP(0, MaxSize, m_SelEnd-1);
			do {
				HitEnd = m_SelEnd += direction;
				HitEnd = CapSelEndToTextLimit();
				rightoff = CLAMP(0, MaxSize, m_SelEnd+direction);
				leftoff = CLAMP(0, MaxSize, m_SelEnd);
				rightvalid = LookUp.Valid(m_EBText[rightoff]);
				LeftValid = LookUp.Valid(m_EBText[leftoff]);
			} while (((m_EBText[rightoff] == m_EBText[leftoff]) || (!LeftValid && !rightvalid))  && !HitEnd);
			m_SelEnd += (direction == 1);
			HitEnd = CapSelEndToTextLimit();
			
			CaretSetPosition(m_SelEnd, Select);
			return HitEnd;
			
		}
		return true;
	};
	
	void CaretMoveLine(int direction, int Select = 0) {
								int LastLine = this->GetLineCount() - 1;
								int LineTo = max(0,min(LastLine, this->GetLineFromPos(m_SelEnd) + direction));
								int pos = this->GetFirstPosInLine(LineTo);
								PosFromChar caretCoord = this->GetPosFromChar(m_SelEnd);
								PosFromChar charCoord;
								do {
									charCoord = this->GetPosFromChar(pos);
									pos++;
								} while (charCoord.x < caretCoord.x && m_EBText[pos] != '\0' && m_EBText[pos] != '\n');
								this->CaretSetPosition(pos-1, Select);
	};
	void CaretMoveEndOfLine(int ControlEnd = 0, int Select = 0) {
								if (ControlEnd)	{
									this->CaretSetPosition(this->GetTextLenght(), Select);
								}
								else {
									this->CaretSetPosition(this->GetFirstPosInLine(this->GetLineFromPos(m_SelEnd)) + this->GetLineLengthFromPos(m_SelEnd), Select);
								}
	};
	void CaretMoveStartOfLine(int ControlHome = 0, int Select = 0) {
								if (ControlHome) {
									this->CaretSetPosition(0, Select);
								}
								else {
									this->CaretSetPosition(this->GetFirstPosInLine(this->GetLineFromPos(m_SelEnd)), Select);
								}
		
	};
	void PageScroll(int direction, int Control = 0,  int Select = 0) {
								int LineOffset = this->GetLineFromPos(m_SelEnd) - this->GetFirstVisibleLine();
								PosFromChar caretCoord = this->GetPosFromChar(m_SelEnd);
								if (direction == -1)
								{
									if (Control)
									{
										DirectSendMessage(WM_HSCROLL, MAKEWPARAM(SB_PAGEUP, 0), 0);
									}
									else
									{
										DirectSendMessage(WM_VSCROLL, MAKEWPARAM(SB_PAGEUP, 0), 0);
									}
								}
								else
								{
									if (Control)
									{
										DirectSendMessage(WM_HSCROLL, MAKEWPARAM(SB_PAGEDOWN, 0), 0);
									}
									else
									{
										DirectSendMessage(WM_VSCROLL, MAKEWPARAM(SB_PAGEDOWN, 0), 0);
									}
								}
								int NewFirstLine = this->GetFirstVisibleLine();

								int NewLine = this->GetFirstVisibleLine() + LineOffset;
								int LineTo = CLAMP(0,this->GetFirstVisibleLine() + LineOffset, this->GetLineCount()-1);
								int pos = this->GetFirstPosInLine(LineTo);
								PosFromChar charCoord;
								do {
									charCoord = this->GetPosFromChar(pos);
									pos++;
								} while (charCoord.x < caretCoord.x && m_EBText[pos] != '\0' && m_EBText[pos] != '\n');
								this->CaretSetPosition(pos - 1, Select);
	};
	void ScrollToCaret() { DirectSendMessage(EM_SCROLLCARET, 0, 0); };

	CharFromPos GetCharFromPos(LPARAM lParam){
								CharFromPos charFromPos;
								LRESULT lresult = DirectSendMessage(EM_CHARFROMPOS, 0, lParam);
								charFromPos.CharPos = LOWORD(lresult); charFromPos.LinePos = HIWORD(lresult);
								return charFromPos;
	};

	CharFromPos GetCharFromPos(int x, int y) { return GetCharFromPos(MAKELPARAM(x, y)); };

	PosFromChar GetPosFromChar(int CharOffset) {
								PosFromChar posFromChar;
								LRESULT lresult = DirectSendMessage(EM_POSFROMCHAR, CharOffset, 0);
								posFromChar.x = LOWORD(lresult); posFromChar.y = HIWORD(lresult);
								return posFromChar;
	};
	


	int GetEBLine(int LinePos) {
								WORD *FirstWord = (WORD *)m_EBLine;
								*FirstWord = 65535; //set the first word of TextBuff
								int len = DirectSendMessage(EM_GETLINE, LinePos, (LPARAM)m_EBLine);
								m_EBLine[len] = 0; //end of string null
								return len;
	};

	
	void InvalidateEditChar(int CharPos);


	void EditCopy() { 
								EB_SET_SEL(m_SelStart, m_SelEnd); 
								this->DirectSendMessage(WM_COPY, 0, 0); 
								//RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
	};
	void EditPaste() { 
								EB_SET_SEL(m_SelStart, m_SelEnd); 
								if (!IsClipboardFormatAvailable(CF_UNICODETEXT)) return;
								if (!OpenClipboard(m_hWnd)) return;
								HGLOBAL   hglb;
								TCHAR*    lptstr;
								hglb = GetClipboardData(CF_UNICODETEXT);
								if (hglb != NULL)
								{
									lptstr = (TCHAR*) GlobalLock(hglb);
									if (lptstr != NULL)
									{
										// Call the application-defined ReplaceSelection 
										// function to insert the text and repaint the 
										// window. 
										this->AddUndoableContent(lptstr);
										GlobalUnlock(hglb);
									}
								}
								CloseClipboard();
								//this->DirectSendMessage(WM_PASTE, 0, 0); 
								//this->SetSelection(EB_SEL_START_FROM_LRESULT(EB_GET_SEL()), EB_SEL_END_FROM_LRESULT(EB_GET_SEL()));
								//RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
	};
	void EditCut() { 
								EB_SET_SEL(m_SelStart, m_SelEnd); 
								//this->DirectSendMessage(WM_CUT, 0, 0); 
								TCHARString text = this->GetTextPart(m_SelStart, m_SelEnd);
								if (text == L"") return;
								if (!OpenClipboard(m_hWnd)) return;
								EmptyClipboard();
								
								HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE,
									(text.length() + 1) * sizeof(TCHAR));
								if (hglbCopy == NULL)
								{
									CloseClipboard();
									return;
								}


								TCHAR * lptstrCopy = (TCHAR *) GlobalLock(hglbCopy);
								memcpy(lptstrCopy, text.c_str(),
									text.length() * sizeof(TCHAR));
								lptstrCopy[text.length()+1] = (TCHAR)0;    // null character 
								GlobalUnlock(hglbCopy);

								// Place the handle on the clipboard. 

								SetClipboardData(CF_UNICODETEXT, hglbCopy);
								this->AddUndoableContent(L"");
								CloseClipboard();
								//this->SetSelection(EB_SEL_START_FROM_LRESULT(EB_GET_SEL()), EB_SEL_END_FROM_LRESULT(EB_GET_SEL()));
								//RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
	};
	TCHAR WhatBrace(TCHAR c)
	{
		TCHAR braceChars[] = {'{','}','[',']','(',')','<','>','"','\''};
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
			if (c == braceChars[i*2])
			{
				return braceChars[i * 2 + 1];
			}
		}
		return found;

	}
	void EditSelectAll() { CaretMoveStartOfLine(1, 0); CaretMoveEndOfLine(1, 1);};
	void EditMatchBrace(int shift = 0) {
		//find the neared brace on left or right and dertine the direction
		int dir = 0;
		int leftrightOfCarret = 1; //1 found on the right of caret, -1 found on the left
		TCHAR SearchStartChar = WhatBrace(m_EBText[m_SelEnd]); // |{brace}
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
			SearchStartChar = WhatBrace(m_EBText[max(0, m_SelEnd - 1)]); // {brace}|
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
		
		if (dir == 0) return;
		//setup the position for the start of search
		int pos = m_SelEnd;
		if (leftrightOfCarret == 1 && dir == -1) pos+=-1;
		if (leftrightOfCarret == 1 && dir == 1) pos+=1;
		if (leftrightOfCarret == -1 && dir == -1) pos+=-2;
		if (leftrightOfCarret == -1 && dir == 1) pos+=0;
		if (pos < 0) return;
		
		TCHAR TargetBrace = WhatTargetBrace(SearchStartChar);
		int foundct = 1;
		//find the next matching brace in direction, skipping over recursive items, like when |{ {{ }} }
		do {
			
			if (m_EBText[pos] == TargetBrace) foundct--;
			else if (m_EBText[pos] == SearchStartChar) foundct++;
			pos += dir;
		} while (pos >=0 && m_EBText[pos] != '\0' && foundct >0);
		//found it! set the cursor at the right spot
		if (foundct == 0)
		{
			if (dir == 1 && m_EBText[pos] == '\n') pos--;
			else if (dir == -1 && m_EBText[pos] == '\n') pos++;
			else if(dir == -1) pos++;
				
			

			m_SelEnd = pos;
			//make sure we did not fall between \r\n
			if (!shift) m_SelStart = m_SelEnd;
			this->SetSelection(m_SelStart, m_SelEnd);
			this->ScrollToCaret();
			
			//RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);

		}
		
		
		
	};

	TCHARString GetTextPart(int startpos, int endpos)
	{
		TCHARString ret = L"";
		TCHAR t[] = { 0,0 };
		for (int i = startpos; i < endpos; i++)
		{
			t[0] = m_EBText[i];
			ret += t;
		}
		return ret;
	}
	void EditFind(int matchcase = 0, int shift = 0) {
		int start = min(m_SelStart, m_SelEnd);
		int end = max(m_SelStart, m_SelEnd);

		TCHARString SearchFor = this->GetTextPart(start, end);
		if (SearchFor == L"") return;
		int EbLen = this->GetTextLenght();
		int slen = SearchFor.length();
		int dir = 1;
		if (shift) dir = -1;
		start += dir;
		end += dir;
		
		while (start >= 0 && end <= EbLen)
		{
			
			TCHARString Found = this->GetTextPart(start, end);
			if (matchcase)
			{
				
				if (Found == SearchFor)
				{
					this->SetSelection(start, end);
					this->ScrollToCaret();
					//RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
					return;
				}
			}
			else
			{
				if (_wcsnicmp(Found.c_str(), SearchFor.c_str(), slen) == 0)
				{
					this->SetSelection(start, end);
					this->ScrollToCaret();
					//RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
					return;
				}
			}
			start += dir;
			end += dir;
		}
	};
	void EditIndent(int UnIndent = 0)
	{
		int start = min(m_SelStart, m_SelEnd);
		int end = max(m_SelStart, m_SelEnd);
		int startline = this->GetLineFromPos(start);
		int endline = this->GetLineFromPos(end);
		int FirstPosLastLine = this->GetFirstPosInLine(endline);
		if (FirstPosLastLine == end) endline--;
		m_SelEnd = start;
		//SendMessage(this->m_hWnd, WM_SETREDRAW, FALSE, 0);
		for (int i = endline; i >= startline; i--)
		{
			
			//this->CaretMoveStartOfLine();
			this->CaretSetPosition(this->GetFirstPosInLine(i));
			TCHARString Text = this->GetTextPart(m_SelEnd, m_SelEnd+4);
			if (UnIndent)
			{
				if (m_EBText[m_SelEnd] == '\t')
				{
					EB_SET_SEL(m_SelEnd, m_SelEnd + 1);
					//DirectSendMessage(EM_REPLACESEL, TRUE, (LPARAM)L"");
					AddUndoableContent(L"");
					this->CaretSetPosition(this->GetFirstPosInLine(i));
				}
				else if (wcsncmp(Text.c_str(), L"    ", 4)== 0)
				{
					EB_SET_SEL(m_SelEnd, m_SelEnd + 4);
					//DirectSendMessage(EM_REPLACESEL, TRUE, (LPARAM)L"");
					AddUndoableContent(L"");
					this->CaretSetPosition(this->GetFirstPosInLine(i));
				}
				else if (wcsncmp(Text.c_str(), L"   ", 3) == 0)
				{
					EB_SET_SEL(m_SelEnd, m_SelEnd + 3);
					//DirectSendMessage(EM_REPLACESEL, TRUE, (LPARAM)L"");
					AddUndoableContent(L"");
					this->CaretSetPosition(this->GetFirstPosInLine(i));
				}
				else if (wcsncmp(Text.c_str(), L"  ", 2) == 0)
				{
					EB_SET_SEL(m_SelEnd, m_SelEnd + 2);
					//DirectSendMessage(EM_REPLACESEL, TRUE, (LPARAM)L"");
					AddUndoableContent(L"");
					this->CaretSetPosition(this->GetFirstPosInLine(i));
				}
				else if (wcsncmp(Text.c_str(), L" ", 1) == 0)
				{
					EB_SET_SEL(m_SelEnd, m_SelEnd + 1);
					//DirectSendMessage(EM_REPLACESEL, TRUE, (LPARAM)L"");
					AddUndoableContent(L"");
					this->CaretSetPosition(this->GetFirstPosInLine(i));
				}
			}
			else
			{
				//DirectSendMessage(EM_REPLACESEL, TRUE, (LPARAM)L"    ");
				AddUndoableContent(L"    ");
			}
						
		}
		m_SelEnd = start;
		this->CaretMoveStartOfLine();
		for (int i = startline; i <= endline; i++)
		{
			this->CaretMoveLine(1, 1);
		}
		//SendMessage(this->m_hWnd, WM_SETREDRAW, TRUE, 0);
		//RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
		
	};
	void EditComment(int UnComent = 0)
	{
		int start = min(m_SelStart, m_SelEnd);
		int end = max(m_SelStart, m_SelEnd);
		int startline = this->GetLineFromPos(start);
		int endline = this->GetLineFromPos(end);
		int FirstPosLastLine = this->GetFirstPosInLine(endline);
		if (FirstPosLastLine == end) endline--;
		m_SelEnd = start;
		//SendMessage(this->m_hWnd, WM_SETREDRAW, FALSE, 0);
		for (int i = endline; i >= startline; i--)
		{

			//this->CaretMoveStartOfLine();
			this->CaretSetPosition(this->GetFirstPosInLine(i));
			TCHARString Text = this->GetTextPart(m_SelEnd, m_SelEnd + 4);
			
			if (UnComent)
			{
				if (this->m_Language->SINGLELINECOMMENT.Valid((m_EBText + m_SelEnd)))
				{
					EB_SET_SEL(m_SelEnd, m_SelEnd + this->m_Language->SINGLELINECOMMENT.GetLength());
					//DirectSendMessage(EM_REPLACESEL, TRUE, (LPARAM)L"");
					AddUndoableContent(L"");
					this->CaretSetPosition(this->GetFirstPosInLine(i));
				}
			}
			else
			{
				DirectSendMessage(EM_REPLACESEL, TRUE, (LPARAM)this->m_Language->SINGLELINECOMMENT.GetValue());
			}

		}
		m_SelEnd = start;
		this->CaretMoveStartOfLine();
		for (int i = startline; i <= endline; i++)
		{
			this->CaretMoveLine(1, 1);
		}
		EB_SET_SEL(m_SelStart, m_SelEnd);
		//SendMessage(this->m_hWnd, WM_SETREDRAW, TRUE, 0);
		//RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);

	};

	void DoBackTab() {
		if (m_EBText[max(0, m_SelEnd - 1)] == '\t')
		{
			EB_SET_SEL(m_SelEnd, m_SelEnd - 1);
			AddUndoableContent(L"");
			//DirectSendMessage(EM_REPLACESEL, TRUE, (LPARAM)L"");
			//this->SetSelection(EB_SEL_START_FROM_LRESULT(EB_GET_SEL()), EB_SEL_END_FROM_LRESULT(EB_GET_SEL()));
		}
		else
		{
			if (this->GetTextPart(max(0, m_SelEnd - 4), max(0, m_SelEnd - 4 + 4)) == L"    ")
			{
				EB_SET_SEL(m_SelEnd, m_SelEnd - 4);
				//DirectSendMessage(EM_REPLACESEL, TRUE, (LPARAM)L"");
				AddUndoableContent(L"");
					
			}
			else if (this->GetTextPart(max(0, m_SelEnd - 3), max(0, m_SelEnd - 3 + 3)) == L"   ")
			{
				EB_SET_SEL(m_SelEnd, m_SelEnd - 3);
				//DirectSendMessage(EM_REPLACESEL, TRUE, (LPARAM)L"");
				AddUndoableContent(L"");
					
			}
			else if (this->GetTextPart(max(0, m_SelEnd - 2), max(0, m_SelEnd - 2 + 2)) == L"  ")
			{
				EB_SET_SEL(m_SelEnd, m_SelEnd - 2);
				//DirectSendMessage(EM_REPLACESEL, TRUE, (LPARAM)L"");
				AddUndoableContent(L"");
					
			}
			else  if (this->GetTextPart(max(0, m_SelEnd - 1), max(0, m_SelEnd - 1 + 1)) == L" ")
			{
				EB_SET_SEL(m_SelEnd, m_SelEnd - 1);
				//DirectSendMessage(EM_REPLACESEL, TRUE, (LPARAM)L"");
				AddUndoableContent(L"");
					
			}
			
		}
	};
	void AddUndoableContent(TCHARString Text)
	{
		///this->SetSelection(EB_SEL_START_FROM_LRESULT(EB_GET_SEL()), EB_SEL_END_FROM_LRESULT(EB_GET_SEL()));
		int start = min(EB_SEL_START_FROM_LRESULT(EB_GET_SEL()), EB_SEL_END_FROM_LRESULT(EB_GET_SEL()));
		int end = max(EB_SEL_START_FROM_LRESULT(EB_GET_SEL()), EB_SEL_END_FROM_LRESULT(EB_GET_SEL()));
		while (!RedoStack.empty())
		{
			EditAction *t = RedoStack.top();
			RedoStack.pop();
			delete t;
		}
		TCHARString OldText = this->GetTextPart(start,end);

		UndoStack.push(new EditAction(start, OldText, Text));
		DirectSendMessage(EM_REPLACESEL, TRUE, (LPARAM)Text.c_str());
		this->SetSelection(EB_SEL_START_FROM_LRESULT(EB_GET_SEL()), EB_SEL_END_FROM_LRESULT(EB_GET_SEL()));
	}
	void EditUndo() {
		if (!UndoStack.empty())
		{
			EditAction *t = UndoStack.top();
			UndoStack.pop();
			EB_SET_SEL(t->m_SelStart, t->m_SelStart + t->m_NewText.length());
			DirectSendMessage(EM_REPLACESEL, TRUE, (LPARAM)t->m_OrigSelText.c_str());
			this->SetSelection(EB_SEL_START_FROM_LRESULT(EB_GET_SEL()), EB_SEL_END_FROM_LRESULT(EB_GET_SEL()));


			RedoStack.push(t);
			this->ScrollToCaret();
		}
	};
	void EditRedo() {
		if (!RedoStack.empty())
		{
			EditAction *t = RedoStack.top();
			RedoStack.pop();

			EB_SET_SEL(t->m_SelStart, t->m_SelStart + t->m_OrigSelText.length());
			DirectSendMessage(EM_REPLACESEL, TRUE, (LPARAM)t->m_NewText.c_str());
			this->SetSelection(EB_SEL_START_FROM_LRESULT(EB_GET_SEL()), EB_SEL_END_FROM_LRESULT(EB_GET_SEL()));
			

			UndoStack.push(t);
			this->ScrollToCaret();
		}
	};

	void ToggleSyntax() { 
		m_SyntaxHighlighting = !m_SyntaxHighlighting; 
		COLORREF color = 0xAAAAAA;
		int ct = 0;
		for (int i = 0; i < 65536; i++) {
			
			m_CharColors[i] = color;

		}
		//RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE); 
	};


};

