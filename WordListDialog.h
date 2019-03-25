

#pragma once
typedef struct WordListDlgPos {
	RECT m_oRect; //orig size
	RECT m_nRect; //new size
	LOGFONT m_oFontData; //orig font
	LOGFONT m_nFontData; //new font
	HFONT m_hFont; //font to change to
	int m_at = 0; //counter for enum
	RECT m_oCtrlRects[1]; //children orig position, num items should match the ctrl count in the dialog
} WORDLISTDLGPOS;
class WordListDialog
{
public:
	HWND m_hDlg = NULL;
	HWND m_hParent = NULL;
	HWND m_hWndDispatch = NULL;
	DWORD m_IDCDispatch = NULL;
	WORDLISTDLGPOS m_PosData;
	WORDLIST m_WordList;
	bool m_Sort = false;
	bool m_Visible = false;
public:
	WordListDialog();
	~WordListDialog();
	void Init(HWND hParent);
	void Show(DWORD ShowStyle = SW_SHOW, short x = -9999, short y = -9999, short w = -9999, short h = -9999, short InvertedOffset = 0);
	void Hide();
	bool IsVisible(bool UseFlag = false);
	bool IsPtOverDlg(POINT pt);
	void Kill();
	void UpdatePosition();
	void ForceRefresh();
	void ResizeToParentFont(HFONT hFnt);
	void GetInitialPositions();
	void SetWordList(WORDLIST WordList);
	void RefreshListContent(wstring Word, int trim, wstring Alternative = L"");
	void SetDispatch(HWND hReceiver, DWORD ControlID);

};

