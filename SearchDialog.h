#pragma once

class OwnerDrawButton
{
public:
	int State = 0;
	int MouseState = 0;
	
};

typedef struct SearchDlgPos {
	RECT m_oRect; //orig size
	RECT m_nRect; //new size
	LOGFONT m_oFontData; //orig font
	LOGFONT m_nFontData; //new font
	HFONT m_hFont; //font to change to
	int m_at = 0; //counter for enum
	RECT m_oCtrlRects[13]; //children orig position, num items should match the ctrl count in the dialog
} SEARCHDLGPOS;
class SearchDialog
{
public:
	HWND m_hDlg = NULL;
	HWND m_hParent = NULL;
	int m_CheckTimer = 0;
	HWND m_HasAnHover = NULL;
	OwnerDrawButton m_SearchDropDownBut;
	OwnerDrawButton m_SearchLeftBut;
	OwnerDrawButton m_SearchRightBut;
	OwnerDrawButton m_SearchCaseBut;
	OwnerDrawButton m_SearchWordBut;
	OwnerDrawButton m_CloseSearchBut;

	OwnerDrawButton m_ReplaceDropDownBut;
	OwnerDrawButton m_ReplaceLeftBut;
	OwnerDrawButton m_ReplaceRightBut;
	OwnerDrawButton m_ReplaceCurrentBut;
	OwnerDrawButton m_ReplaceAllBut;
	SEARCHDLGPOS m_PosData;

	
public:
	SearchDialog();
	~SearchDialog();
	void Init(HWND hParent);
	void Show(DWORD ShowStyle = SW_SHOW);
	void Hide();
	bool IsVisible();
	bool IsPtOverDlg(POINT pt);
	void Kill();
	void UpdatePosition();
	
	wstring GetSearchText();
	wstring GetReplaceText();
	void ForceRefresh();
	void ResizeToParentFont(HFONT hFnt);
	void GetInitialPositions();
};

