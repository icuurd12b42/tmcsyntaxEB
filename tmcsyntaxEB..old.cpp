
 // Dll1.cpp : Defines the exported functions for the DLL application.
 //
#include "stdafx.h"
#include <unordered_map>




#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
using namespace std;



#ifndef UNICODE  
typedef std::string TCHARString;
#else
typedef std::wstring TCHARString;
#endif

#define gmexport extern "C" __declspec (dllexport)

void debug(const char * t)
{

	OutputDebugStringA(t);
	/*
	stringstream ss;
	ss << "\r\nRect {" << rect.left << "," << rect.top << "," << rect.right << "," << rect.bottom << "}";
	ss << "\r\nPos {" << x << "," << y << "}";
	ss << "\r\nCharPos {" << charPos << "}";
	debug(ss.str().c_str());
	//MessageBoxA(GetActiveWindow(), ss.str().c_str(), "SetHilite", MB_ICONINFORMATION | MB_OK);
	*/
}
TCHARString stringToString(const std::string& s)
{
	TCHARString temp(s.length(), L' ');
	std::copy(s.begin(), s.end(), temp.begin());
	return temp;
}


std::string WStringToString(const std::wstring& s)
{
	std::string temp(s.length(), ' ');
	std::copy(s.begin(), s.end(), temp.begin());
	return temp;
}

COLORREF textCol = 0xff0000;
int byteAt = 0;
TCHAR lastString[65536];
int lastString_len = 0;
int maxbytes = 0;
TCHAR TextBuff[65536];


TCHARString ReplaceAll(TCHARString str, TCHARString& from, TCHARString& to) {
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return str;
}
set<TCHARString> KeyWords;
void InitKeywords()
{
	KeyWords.insert(L"break");
	KeyWords.insert(L"for");
	KeyWords.insert(L"new");
	KeyWords.insert(L"var");
	KeyWords.insert(L"continue");
	KeyWords.insert(L"function");
	KeyWords.insert(L"return");
	KeyWords.insert(L"void");
	KeyWords.insert(L"delete");
	KeyWords.insert(L"if");
	KeyWords.insert(L"this");
	KeyWords.insert(L"while");
	KeyWords.insert(L"else");
	KeyWords.insert(L"in");
	KeyWords.insert(L"typeof");
	KeyWords.insert(L"with");
	KeyWords.insert(L"case");
	KeyWords.insert(L"debugger");
	KeyWords.insert(L"export");
	KeyWords.insert(L"super");
	KeyWords.insert(L"catch");
	KeyWords.insert(L"default");
	KeyWords.insert(L"extends");
	KeyWords.insert(L"switch");
	KeyWords.insert(L"class");
	KeyWords.insert(L"do");
	KeyWords.insert(L"finally");
	KeyWords.insert(L"throw");
	KeyWords.insert(L"const");
	KeyWords.insert(L"enum");
	KeyWords.insert(L"import");
	KeyWords.insert(L"try");

}
bool IsKeyWord()
{
	TCHARString t = lastString;
	if (KeyWords.count(t))
	{
		return true;
	}
	return false;
}
set<TCHARString>Functions;
void InitFunctions()
{

	Functions.insert(L"BlockInput");
	Functions.insert(L"Send");
	Functions.insert(L"Sleep");
}
bool IsFunction()
{
	TCHARString t = lastString;
	if (Functions.count(t))
	{
		return true;
	}
	return false;
}
set<TCHARString>ClassFuncs;
void InitClassFuncs()
{

	ClassFuncs.insert(L"HKS");
	ClassFuncs.insert(L".Recognition");
	ClassFuncs.insert(L".StartDictating");
	ClassFuncs.insert(L".split");

}
bool IsClassFunc()
{
	TCHARString t = lastString;
	if (ClassFuncs.count(t))
	{
		return true;
	}
	return false;
}
set<TCHARString>Literals;
void InitLiterals()
{

	Literals.insert(L"true");
	Literals.insert(L"false");
	Literals.insert(L"undefined");
	Literals.insert(L"null");
	Literals.insert(L"infinity");
	Literals.insert(L"NaN");
}
bool IsLiteral()
{
	TCHARString t = lastString;
	if (Literals.count(t))
	{
		return true;
	}
	return false;
}
set<TCHARString>StaticClass;
void InitStaticClass()
{
	StaticClass.insert(L"HKS");
	StaticClass.insert(L"HKS.Action");
	StaticClass.insert(L"HKS.Action.Run");
}
bool IsStaticClass()
{
	TCHARString t = lastString;
	if (StaticClass.count(t))
	{
		return true;
	}
	return false;
}
char AlphaNums[] = "abcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
char Hexas[] = "0123456789abcdefABCDEF";
char whitespace[] = " \t\r\n";
char splitchars[] = "~`!@#$%^&*()-+={[}]|\\:;<,>?/";
char stopchars[] = "'~`!@#$%^&*()-+={[}]|\\:;<,>?/\t\r\n\" ";
bool IsStopChar(TCHAR c)
{
	for (int i = 0; i < 34; i++)
	{
		if (c == stopchars[i])
		{
			return true;
		}
	}
	return false;
}
bool IsSplitChar(TCHAR c)
{
	for (int i = 0; i < 28; i++)
	{
		if (c == splitchars[i])
		{
			return true;
		}
	}
	return false;
}
bool IsWhiteSpace(TCHAR c)
{
	for (int i = 0; i < 4; i++)
	{
		if (c == whitespace[i])
		{
			return true;
		}
	}
	return false;
}
bool IsHexadecimal(TCHAR c)
{
	for (int i = 0; i < 22; i++)
	{
		if (c == Hexas[i])
		{
			return true;
		}
	}
	return false;
}
bool IsAlphaNumeric(TCHAR c)
{
	for (int i = 0; i < 63; i++)
	{
		if (c == AlphaNums[i])
		{
			return true;
		}
	}
	return false;
}
bool IsAlpha(TCHAR c)
{
	for (int i = 0; i < 53; i++)
	{
		if (c == AlphaNums[i])
		{
			return true;
		}
	}
	return false;
}

bool IsNumeric(TCHAR c)
{
	for (int i = 53; i < 63; i++)
	{
		if (c == AlphaNums[i])
		{
			return true;
		}
	}
	return false;
}
bool IsSingleQuote(TCHAR c)
{
	return c == '\'';
}

bool IsDoubleQuote(TCHAR c)
{
	return c == '"';
}

TCHAR ReadByte()
{
	if (byteAt >= maxbytes) return 0;
	return TextBuff[byteAt++];
}
TCHAR ScanByte()
{
	if (byteAt >= maxbytes) return 0;
	return TextBuff[byteAt];
}
TCHAR ScanNextByte()
{
	if (byteAt >= maxbytes) return 0;
	return TextBuff[byteAt + 1];
}

bool ReadAlphaNumeric()
{
	while (IsAlphaNumeric(ScanByte()) && byteAt < maxbytes)
	{
		lastString[lastString_len++] = ReadByte();
		lastString[lastString_len] = 0;
	}
	return true;
}
bool ReadSplitChars()
{
	while (IsSplitChar(ScanByte()) && byteAt < maxbytes)
	{
		lastString[lastString_len++] = ReadByte();
		lastString[lastString_len] = 0;
	}
	return true;
}
bool ReadNonAlphaNumeric()
{
	while (!IsAlphaNumeric(ScanByte()) && byteAt < maxbytes)
	{
		lastString[lastString_len++] = ReadByte();
		lastString[lastString_len] = 0;
	}
	return true;
}
bool ReadNumeric()
{
	bool did_dot = false;
	bool did_e = false;
	bool did_a_num = false;
	if (ScanByte() == '0' && ScanNextByte() == 'x')
	{
		lastString[lastString_len++] = ReadByte(); // 0 of 0xffff
		lastString[lastString_len++] = ReadByte(); // x of 0xffff
		lastString[lastString_len] = 0;
		while (byteAt < maxbytes)
		{
			if (IsHexadecimal(ScanByte()))
			{
				lastString[lastString_len++] = ReadByte();
				lastString[lastString_len] = 0;
			}
			else
			{
				return true;
			}
		}
		return true;
	}

	while (byteAt < maxbytes)
	{
		if (IsNumeric(ScanByte()))
		{
			lastString[lastString_len++] = ReadByte();
			lastString[lastString_len] = 0;
			did_a_num = true;
		}
		else if (ScanByte() == '.' && did_dot == false && did_e == false)
		{

			lastString[lastString_len++] = ReadByte();
			lastString[lastString_len] = 0;
			did_dot = true;
			did_a_num = true;

		}
		else if (ScanByte() == 'e' && did_a_num == true && did_e == false)
		{
			TCHAR e = ReadByte();
			if (IsNumeric(ScanNextByte()) && (ScanByte() == '-' || ScanByte() == '+'))
			{
				lastString[lastString_len++] = e;
				lastString[lastString_len++] = ReadByte();
				lastString[lastString_len++] = ReadByte();
				lastString[lastString_len] = 0;
			}
			else if (!IsNumeric(ScanByte()))
			{
				return true;
			}
			else
			{
				lastString[lastString_len++] = e;
				lastString[lastString_len] = 0;
			}
			did_dot = true;
			did_e = true;
		}
		else
		{
			return true;
		}

	}
	return true;
}
bool HasTabs = false;
bool ReadWhiteSpace()
{
	HasTabs = false;
	while (IsWhiteSpace(ScanByte()) && byteAt < maxbytes)
	{
		TCHAR c = ReadByte();
		if (c == '\t')
		{
			HasTabs = true;
		}
		lastString[lastString_len++] = c;
		lastString[lastString_len] = 0;
	}
	return true;
}
bool InMultiLineComment = false;
bool ReadMultiLineComment()
{
	TCHAR c;
	if (InMultiLineComment == false)
	{
		lastString[lastString_len++] = ReadByte(); // /
		lastString[lastString_len++] = ReadByte(); // *
		lastString[lastString_len] = 0;
	}
	InMultiLineComment = true;
	while (byteAt < maxbytes)
	{
		c = ReadByte();
		lastString[lastString_len++] = c;
		lastString[lastString_len] = 0;
		if (c == '*' && ScanByte() == '/')
		{
			lastString[lastString_len++] = ReadByte();
			lastString[lastString_len] = 0;
			InMultiLineComment = false;
			return true;
		}

	}
	return true;
}
bool InSingleQuoteText = false;
bool ReadSingleQuotedText()
{
	TCHAR c;

	if (InSingleQuoteText == false)
	{
		lastString[lastString_len++] = ReadByte(); // '
		lastString[lastString_len] = 0;
	}
	InSingleQuoteText = true;
	while (byteAt < maxbytes)
	{
		c = ReadByte();
		lastString[lastString_len++] = c;
		lastString[lastString_len] = 0;
		if (c == '\\' && IsSingleQuote(ScanByte()))
		{
			lastString[lastString_len++] = ReadByte();
			lastString[lastString_len] = 0;
		}
		else if (IsSingleQuote(c))
		{
			lastString[lastString_len] = 0;
			InSingleQuoteText = false;
			return true;
		}
	}
	return true;
}
bool InDoubleQuoteText = false;
bool ReadDoubleQuotedText()
{
	TCHAR c;

	if (InDoubleQuoteText == false)
	{
		lastString[lastString_len++] = ReadByte(); // "
		lastString[lastString_len] = 0;
	}
	InDoubleQuoteText = true;
	while (byteAt < maxbytes)
	{
		c = ReadByte();
		lastString[lastString_len++] = c;
		lastString[lastString_len] = 0;
		if (c == '\\' && IsDoubleQuote(ScanByte()))
		{
			lastString[lastString_len++] = ReadByte();
			lastString[lastString_len] = 0;
		}
		else if (IsDoubleQuote(c))
		{
			lastString[lastString_len] = 0;
			InDoubleQuoteText = false;
			return true;
		}
	}
	return true;
}
bool ReadSingleLineComment()
{
	while (byteAt < maxbytes)
	{
		lastString[lastString_len++] = ReadByte();
		lastString[lastString_len] = 0;
	}
	return true;
}
void ProcessCodeStart()
{
	int byteAt = 0;
	lastString[0] = 0;
	lastString_len = 0;
	textCol = 0xFFFFFF;
	InMultiLineComment = false;
	InSingleQuoteText = false;
	InDoubleQuoteText = false;
}
void ProcessLineStart(int numbytes)
{
	byteAt = 0;
	lastString[0] = 0;
	lastString_len = 0;
	maxbytes = numbytes;
}

COLORREF QUOTECOLOR = 0x5ADBE6;
COLORREF COMMENTCOLOR = 0x65AD82;
COLORREF NUMBERCOLOR = 0xA8FFB5;
COLORREF UNKNOWNWORDCOLOR = 0xC8D4D4;
COLORREF SPLITCHARSCOLOR = 0xA6b2b2;
COLORREF BADCHARSCOLOR = 0xC8D4FF;
COLORREF KEYWORDCOLOR = 0x746498;
COLORREF FUNCTIONCOLOR = 0xA375CB;
COLORREF LITERALCOLOR = 0x1F58DD;
COLORREF CLASSFUNCCOLOR = 0xA375CB;
COLORREF STATICCLASSCOLOR = 0xA375CB;
COLORREF SELECTEDTEXTCOLOR = 0x784F26;
COLORREF USERFUNCTIONCOLOR = 0xA375CB;
COLORREF TEXTBACKCOLOR = 0x272727;
bool ProcessBuff()
{
	//return ReadSingleLineComment();
	while (byteAt < maxbytes)
	{
		if (InSingleQuoteText)
		{
			textCol = QUOTECOLOR;
			return ReadSingleQuotedText();
		}
		else if (InDoubleQuoteText)
		{
			textCol = QUOTECOLOR;
			return ReadDoubleQuotedText();
		}
		else if (InMultiLineComment)
		{
			textCol = COMMENTCOLOR;
			return ReadMultiLineComment();
		}
		else if (ScanByte() == '/' && ScanNextByte() == '*')
		{
			textCol = COMMENTCOLOR;
			return ReadMultiLineComment();

		}
		else if (IsSingleQuote(ScanByte()))
		{
			textCol = QUOTECOLOR;
			return ReadSingleQuotedText();
		}
		else if (IsDoubleQuote(ScanByte()))
		{
			textCol = QUOTECOLOR;
			return ReadDoubleQuotedText();
		}
		else if (ScanByte() == '/' && ScanNextByte() == '/')
		{
			textCol = COMMENTCOLOR;
			return ReadSingleLineComment();
		}
		else if (IsNumeric(ScanByte()) || (ScanByte() == '.' && IsNumeric(ScanNextByte())))
		{
			textCol = NUMBERCOLOR;
			return ReadNumeric();
		}
		else if (IsAlpha(ScanByte()) || (ScanByte() == '.' && IsAlpha(ScanNextByte())))
		{
			if (ScanByte() == '.')
			{
				lastString[lastString_len++] = ReadByte();
				lastString[lastString_len] = 0;
			}
			textCol = UNKNOWNWORDCOLOR;
			ReadAlphaNumeric();
			if (0)
			{

			}
			else if (IsLiteral())
			{
				textCol = LITERALCOLOR;
			}
			else if (IsKeyWord())
			{
				textCol = KEYWORDCOLOR;
			}
			else if (IsFunction())
			{
				textCol = FUNCTIONCOLOR;
			}
			else if (IsStaticClass())
			{
				textCol = STATICCLASSCOLOR;
				if (ScanByte() == '.')
				{
					lastString[lastString_len++] = ReadByte();
					lastString[lastString_len] = 0;
					return  ProcessBuff();
				}
			}
			else if (IsClassFunc())
			{
				textCol = CLASSFUNCCOLOR;
			}
			else if (ScanByte() == '(')
			{
				textCol = USERFUNCTIONCOLOR;
			}
			return true;
		}
		else if (IsSplitChar(ScanByte()))
		{
			textCol = SPLITCHARSCOLOR;
			return ReadSplitChars();
		}
		else if (IsWhiteSpace(ScanByte()))
		{
			//textCol = 0x8888FF;
			return ReadWhiteSpace();
		}
		else
		{
			textCol = BADCHARSCOLOR;
			lastString[lastString_len++] = ReadByte();
			lastString[lastString_len] = 0;
			return true;
		}

	}
	return false;
}


HINSTANCE ghInst;
WNDPROC wpOrigEditProc = NULL;
BOOL WINAPI DllMain(HINSTANCE hInst /* Library instance handle. */,
	DWORD reason /* Reason this function is being called. */,
	LPVOID reserved /* Not used. */)
{
	ghInst = hInst;
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		break;

	case DLL_PROCESS_DETACH:
		break;

	case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		break;
	}

	/* Returns TRUE on success, FALSE on failure */
	return TRUE;
}

void ReadWordSet(ifstream *inFile, set<TCHARString> *theset)
{

	string line;
	theset->clear();
	int lct = 0;
	int addct = 0;
	int potct = 0;
	while (!inFile->eof())
	{
		std::getline(*inFile, line);
		lct++;
		if (line.compare("}") == 0)
		{

			return;
		}
		if (line.length())
		{
			potct++;
			if (line.at(0) != ';')
			{
				theset->insert(stringToString(line));
				addct++;
			}
		}
	}
}
void ReadSyntax(LPSTR filename)
{
	ifstream inFile;
	inFile.open(filename);
	if (!inFile) return;
	string line;
	TCHARString wString;
	while (!inFile.eof())
	{
		std::getline(inFile, line);
		if (line.compare("QUOTECOLOR") == 0)
		{
			std::getline(inFile, line);
			std::istringstream converter(line);
			converter >> std::hex >> QUOTECOLOR;
		}
		else if (line.compare("TEXTBACKCOLOR") == 0)
		{
			std::getline(inFile, line);
			std::istringstream converter(line);
			converter >> std::hex >> TEXTBACKCOLOR;
		}
		else if (line.compare("COMMENTCOLOR") == 0)
		{
			std::getline(inFile, line);
			std::istringstream converter(line);
			converter >> std::hex >> COMMENTCOLOR;
		}
		else if (line.compare("NUMBERCOLOR") == 0)
		{
			std::getline(inFile, line);
			std::istringstream converter(line);
			converter >> std::hex >> NUMBERCOLOR;
		}
		else if (line.compare("UNKNOWNWORDCOLOR") == 0)
		{
			std::getline(inFile, line);
			std::istringstream converter(line);
			converter >> std::hex >> UNKNOWNWORDCOLOR;
		}
		else if (line.compare("SPLITCHARSCOLOR") == 0)
		{
			std::getline(inFile, line);
			std::istringstream converter(line);
			converter >> std::hex >> SPLITCHARSCOLOR;
		}
		else if (line.compare("BADCHARSCOLOR") == 0)
		{
			std::getline(inFile, line);
			std::istringstream converter(line);
			converter >> std::hex >> BADCHARSCOLOR;
		}
		else if (line.compare("KEYWORDCOLOR") == 0)
		{
			std::getline(inFile, line);
			std::istringstream converter(line);
			converter >> std::hex >> KEYWORDCOLOR;
		}
		else if (line.compare("FUNCTIONCOLOR") == 0)
		{
			std::getline(inFile, line);
			std::istringstream converter(line);
			converter >> std::hex >> FUNCTIONCOLOR;
		}
		else if (line.compare("LITERALCOLOR") == 0)
		{
			std::getline(inFile, line);
			std::istringstream converter(line);
			converter >> std::hex >> LITERALCOLOR;
		}
		else if (line.compare("CLASSFUNCCOLOR") == 0)
		{
			std::getline(inFile, line);
			std::istringstream converter(line);
			converter >> std::hex >> CLASSFUNCCOLOR;
		}
		else if (line.compare("SELECTEDTEXTCOLOR") == 0)
		{
			std::getline(inFile, line);
			std::istringstream converter(line);
			converter >> std::hex >> SELECTEDTEXTCOLOR;
		}
		else if (line.compare("STATICCLASSCOLOR") == 0)
		{
			std::getline(inFile, line);
			std::istringstream converter(line);
			converter >> std::hex >> STATICCLASSCOLOR;
		}
		else if (line.compare("USERFUNCTIONCOLOR") == 0)
		{
			std::getline(inFile, line);
			std::istringstream converter(line);
			converter >> std::hex >> USERFUNCTIONCOLOR;
		}

		else if (line.compare("KEYWORDS {") == 0)
		{
			ReadWordSet(&inFile, &KeyWords);
		}
		else if (line.compare("LITERALS {") == 0)
		{
			ReadWordSet(&inFile, &Literals);
		}
		else if (line.compare("FUNCTIONS {") == 0)
		{
			ReadWordSet(&inFile, &Functions);
		}
		else if (line.compare("CLASSFUNCS {") == 0)
		{
			ReadWordSet(&inFile, &ClassFuncs);
		}
		else if (line.compare("STATICCLASS {") == 0)
		{
			ReadWordSet(&inFile, &StaticClass);
		}

	}
	inFile.close();
}

int initialWordStartPos = 0;
int initialWordEndPos = 0;

//finds in TextBuff the next word stop starting at startpos moving towards endpos
template <typename T> int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}
//this function replaces SendMessage for asking THIS edit box for settings... so general queries dont go through our own message proc and cause needles confusion and lag
LRESULT DirectSend(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return CallWindowProc(wpOrigEditProc, hwnd, message, wParam, lParam);
}
void DoDraw(HWND hwnd)
{
	ProcessCodeStart();
	//Dc and Paint Struct, get the DC from paintstruct or client if not paintstruct data
	PAINTSTRUCT ps;
	int TerminatePaintMode = 0; //determines how we clean up the dc
								//get the dc
	HDC hdc = BeginPaint(hwnd, &ps);
	if (hdc == NULL)
	{
		//if failed, try window
		hdc = GetDC(hwnd);
		TerminatePaintMode = 1;
		if (hdc == NULL)
		{
			//failed, quit
			return;
		}
	}
	//normal text backcolor
	COLORREF NormalBackColor = TEXTBACKCOLOR;// GetBkColor(hdc);
	SetTextColor(hdc, textCol);

	//Font, as defined by the control settings
	HFONT hfont = (HFONT)DirectSend(hwnd, WM_GETFONT, NULL, NULL);
	HFONT hfOld = (HFONT)SelectObject(hdc, hfont);

	//Font size
	TEXTMETRIC textMetrics;
	GetTextMetrics(hdc, &textMetrics);

	//The region of the control
	RECT rect;
	DirectSend(hwnd, EM_GETRECT, 0, (LPARAM)&rect);
	//the draw text clipping region
	int TopCutoff = rect.top - 2;
	int BottomCutoff = rect.bottom + 2;
	int LeftCutoff = rect.left - 2;
	int RightCutoff = rect.right + 2;

	//set the port clipping region
	HRGN hrgn = CreateRectRgn(rect.left, rect.top, rect.right + 2, rect.bottom + 2);
	//Clip that port
	SelectClipRgn(hdc, hrgn);


	//the upper left corner of the text in pixels relative to client and scroll pos
	//this tells me the client relative postion of the draw
	LRESULT posStart = DirectSend(hwnd, EM_POSFROMCHAR, 0, 0);
	short posX = LOWORD(posStart);
	short posY = HIWORD(posStart);

	//Num lines in control
	int numLines = DirectSend(hwnd, EM_GETLINECOUNT, 0, 0);


	//the selection range for text selection
	int selRange = DirectSend(hwnd, EM_GETSEL, 0, 0);
	int selStart = LOWORD(selRange);
	int selEnd = HIWORD(selRange);
	int drawCharPos = 0;
	int virtualSpaceCount = 0;

	//draw each line
	TCHAR OneChar[1];
	bool OverlapSel;
	int w;
	SIZE sz;
	int y = posY;
	int xOff = 0;
	for (int i = 0; i < numLines; i++)
	{
		xOff = 0;
		//using the global allocated buffer, get the line
		WORD *FirstWord = (WORD *)TextBuff;
		*FirstWord = 65535; //set the first word of TextBuff
		int len = DirectSend(hwnd, EM_GETLINE, i, (LPARAM)TextBuff);
		TextBuff[len] = 0; //end of string null

						   //start parsing process
		ProcessLineStart(len);
		//the virtual character count includes tab to spaces
		virtualSpaceCount = 0;
		//Where the line needs processing
		while (ProcessBuff())
		{
			//for each word gnerated

			//set colors
			SetTextColor(hdc, textCol);
			SetBkColor(hdc, NormalBackColor);
			//if has tags to conver to space or is in selection range
			//overlap = x1 <= y2 && y1 <= x2
			OverlapSel = (selStart < (drawCharPos + lastString_len)) && drawCharPos < selEnd;
			if (HasTabs || (OverlapSel && (selStart != selEnd)))
			{
				//need to do one character at a time because of the selected text and the tabs
				for (int i = 0; i < lastString_len; i++)
				{
					//inc the drawn character count
					drawCharPos += 1;
					//switch from normal to selected text back color for the string
					if (drawCharPos > selStart && drawCharPos <= selEnd)
					{
						SetBkColor(hdc, SELECTEDTEXTCOLOR);
					}
					else
					{
						SetBkColor(hdc, NormalBackColor);
					}

					//get the single character
					OneChar[0] = lastString[i];
					if (OneChar[0] != '\t') //not an effing tab
					{
						//draw the character and add to the xoffset
						GetCharWidth32(hdc,
							OneChar[0],
							OneChar[0],
							&w);
						//draw only if inside the drawing port x1 <= y2 && y1 <= x2 
						if (y>TopCutoff && (posX + xOff <= RightCutoff && LeftCutoff <= posX + xOff + w))
							TextOut(hdc, posX + xOff, y, OneChar, 1);
						//add to xoffset and virtual char count
						xOff += w;// textMetrics.tmMaxCharWidth;
						virtualSpaceCount++;
					}
					else
					{

						//tab, convert to space
						GetCharWidth32(hdc,
							32,
							32,
							&w);
						//for each effing tabs
						for (int j = (4 - (virtualSpaceCount % 4)); j > 0; j--)
						{

							//draw only if inside the drawing port x1 <= y2 && y1 <= x2 
							if (y>TopCutoff && (posX + xOff <= RightCutoff && LeftCutoff <= posX + xOff + w))
								TextOut(hdc, posX + xOff, y, L" ", 1);
							//add to xoffset and virtual char count
							xOff += w;// textMetrics.tmMaxCharWidth;
							virtualSpaceCount++;
						}

					}

				}
				//reset the hastabs global flag, will ve set by the next white text parse code
				HasTabs = false;
				//////
			}
			else
			{
				//We can do whole words here
				GetTextExtentPoint32(hdc,
					lastString,
					lastString_len,
					&sz
				);
				//inc the drawn character count
				drawCharPos += lastString_len;

				//draw only if inside the drawing port x1 <= y2 && y1 <= x2 
				if (y>TopCutoff && (posX + xOff <= RightCutoff && LeftCutoff <= posX + xOff + sz.cx))
					TextOut(hdc, posX + xOff, y, lastString, lastString_len);
				//add to xoffset and virtual char count
				xOff += sz.cx;// lastString_len * textMetrics.tmAveCharWidth;
				virtualSpaceCount += lastString_len;
			}
			lastString_len = 0;
		}
		//add \r\n count to draw count
		drawCharPos += 2;
		//draw an extra space to nuke the caret footprint on scroll
		SetBkColor(hdc, NormalBackColor);
		if (y>TopCutoff)
			TextOut(hdc, posX + xOff, y, L" ", 1);

		//next line position
		y += textMetrics.tmHeight;

		//break if below the drawing port
		if (y > BottomCutoff)
			break;
	}
	//re-add the old font in the dc
	SelectObject(hdc, hfOld);
	//delete the region
	DeleteObject(hrgn);
	//done painting, releace dc
	if (TerminatePaintMode == 0)
	{
		EndPaint(hwnd, &ps);
	}
	else if (TerminatePaintMode == 1)
	{
		ReleaseDC(hwnd, hdc);
	}

}
void DoTab(HWND hwnd)
{
	SetFocus(hwnd);
	int selRange = DirectSend(hwnd, EM_GETSEL, 0, 0);
	int selStart = LOWORD(selRange);
	int selEnd = HIWORD(selRange);
	int selSize = selEnd - selStart;
	int selMode = 0; //0 craet no selection, selection in a single line, 2 selection in multiple line
	if (selSize != 0)
	{
		selMode = 1;
		int lineStart = DirectSend(hwnd, EM_LINEFROMCHAR, selStart, 0);
		int lineEnd = DirectSend(hwnd, EM_LINEFROMCHAR, selEnd, 0);
		selStart = DirectSend(hwnd, EM_LINEINDEX, lineStart, 0);
		if (lineStart != lineEnd)
		{
			selMode = 2;
			selEnd = DirectSend(hwnd, EM_LINEINDEX, lineEnd, 0) + DirectSend(hwnd, EM_LINELENGTH, selEnd, 0);
		}
		else
		{
			selEnd = selStart + DirectSend(hwnd, EM_LINELENGTH, selStart, 0);
		}
		DirectSend(hwnd, EM_SETSEL, selEnd, selStart);
	}

	if (GetAsyncKeyState(VK_SHIFT))
	{
		if (selMode == 0)
		{
			int len = DirectSend(hwnd, WM_GETTEXT, 65535, (LPARAM)TextBuff);
			TCHAR *tstart = TextBuff + max(0, selStart - 1);
			//MessageBox(GetActiveWindow(), tstart, L"BeforeTab", MB_ICONINFORMATION | MB_OK);
			if (tstart[0] == '\t')
			{
				DirectSend(hwnd, WM_CHAR, VK_BACK, 0);
			}
		}
		else
		{
			int len = DirectSend(hwnd, WM_GETTEXT, 65535, (LPARAM)TextBuff);
			TCHAR *tstart = TextBuff + selStart;
			TextBuff[selEnd] = 0;
			if (tstart[0] == '\t')
			{
				tstart++;
			}
			else if (tstart[0] == ' ' && tstart[1] == ' ' && tstart[2] == ' ' && tstart[3] == ' ')
			{
				tstart += 4;
			}
			TCHARString t = tstart;
			TCHARString Search = L"\r\n\t";
			TCHARString Replace = L"\r\n";
			t = ReplaceAll(t, Search, Replace);
			Search = L"\r\n    ";
			Replace = L"\r\n";
			t = ReplaceAll(t, Search, Replace);
			ShowWindow(hwnd, SW_HIDE);
			DirectSend(hwnd, EM_REPLACESEL, 1, (LPARAM)t.c_str());
			DirectSend(hwnd, EM_SETSEL, selStart + t.length(), selStart);
			ShowWindow(hwnd, SW_SHOW);
			SetFocus(hwnd);
		}
	}
	else
	{
		if (selMode == 0)
		{
			TCHAR tab[2];
			tab[0] = '\t';
			tab[1] = 0;
			DirectSend(hwnd, EM_REPLACESEL, 1, (LPARAM)tab);
		}
		else
		{
			int len = DirectSend(hwnd, WM_GETTEXT, 65535, (LPARAM)TextBuff);
			TCHAR *tstart = TextBuff + selStart;
			TextBuff[selEnd] = 0;
			TCHARString t = L"\t";
			t += tstart;
			TCHARString Search = L"\r\n";
			TCHARString Replace = L"\r\n\t";
			t = ReplaceAll(t, Search, Replace);
			ShowWindow(hwnd, SW_HIDE);
			DirectSend(hwnd, EM_REPLACESEL, 1, (LPARAM)t.c_str());
			DirectSend(hwnd, EM_SETSEL, selStart + t.length(), selStart);
			ShowWindow(hwnd, SW_SHOW);
			SetFocus(hwnd);
		}
	}
}
int FindWord(int startpos, int endpos)
{
	int direction = endpos - startpos;
	direction = sgn(direction);
	if (direction == 0)
		return startpos;
	int pos = startpos;
	TCHAR c;
	c = TextBuff[pos];
	if (!IsStopChar(c))
	{
		while (pos != endpos)
		{
			c = TextBuff[pos];
			if (!IsStopChar(c))
			{
				pos += direction;
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		while (pos != endpos)
		{
			c = TextBuff[pos];
			if (IsStopChar(c) && c != ' ' && c != '\t' && c != '\n' && c != '\r'  && c != '\"' && c != '\'')
			{
				pos += direction;
			}
			else
			{
				break;
			}
		}
	}
	return pos + ((direction == -1) && pos != 0);
}
void DoubleClickSelect(HWND hwnd, short x, short y)
{

	int len = DirectSend(hwnd, WM_GETTEXT, 65535, (LPARAM)TextBuff);
	LRESULT nearXY = DirectSend(hwnd, EM_CHARFROMPOS, 0, MAKELPARAM(x, y));
	int charPos = LOWORD(nearXY);



	if (charPos == 65535)
	{
		return;
	}
	initialWordStartPos = FindWord(charPos, 0);
	initialWordEndPos = FindWord(charPos, len);

	int selStart = initialWordStartPos,
		selEnd = initialWordEndPos;
	if (charPos < (initialWordStartPos + initialWordEndPos) / 2)
	{
		selStart = initialWordEndPos;
		selEnd = initialWordStartPos;
	}


	DirectSend(hwnd, EM_SETSEL, selStart, selEnd);
}

void MoveSelect(HWND hwnd, short x, short y, int rwidth, int rheight)
{

	int selRange = DirectSend(hwnd, EM_GETSEL, 0, 0);
	int selStart = LOWORD(selRange);
	int selEnd = HIWORD(selRange);
	int len = DirectSend(hwnd, WM_GETTEXT, 65535, (LPARAM)TextBuff);
	LRESULT nearXY = DirectSend(hwnd, EM_CHARFROMPOS, 0, MAKELPARAM(x, y));
	int charPos = LOWORD(nearXY);

	if (charPos == 65535)
	{
		return;
	}

	if (charPos < (initialWordStartPos + initialWordEndPos) / 2)
	{
		selStart = initialWordEndPos;
		selEnd = FindWord(charPos, 0);
	}
	else
	{
		selStart = initialWordStartPos;
		selEnd = FindWord(charPos, len);
	}

	DirectSend(hwnd, EM_SETSEL, selStart, selEnd);

}

//Tell the c++ compiler to switch to standard C compiling so exported function names as not mangled
extern "C" {


	//NeedUpdate detection flags
	LRESULT OldselRange = -1;
	LRESULT OldposStart = -1;
	LRESULT OldselSize = -1;
	LRESULT OldtextLen = -1;
	BOOL DidDblClick = FALSE;
	LRESULT CALLBACK WndEditProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		//before porsessing event, get the states needed for detecting if redrawing is required
		RECT rect;
		DirectSend(hwnd, EM_GETRECT, 0, (LPARAM)&rect);
		//these need to be here
		LRESULT selRange = DirectSend(hwnd, EM_GETSEL, 0, 0);
		int selStart = LOWORD(selRange);
		int selEnd = HIWORD(selRange);
		int selSize = selEnd - selStart;
		LRESULT posStart = DirectSend(hwnd, EM_POSFROMCHAR, 0, 0);

		//The force erase
		BOOL EraseBack = FALSE;

		//need update if there is a region
		int NeedUpdate = GetUpdateRect(hwnd, 0, 0) != 0;
		LRESULT r = 0;
		//pre processing tab

		if (message == WM_CHAR && wParam == VK_TAB)
		{
			//do the tab and force erase
			DoTab(hwnd);
			NeedUpdate = TRUE;
			EraseBack = TRUE;
		}
		else if (message == WM_LBUTTONDBLCLK)
		{

			//r = CallWindowProc(wpOrigEditProc, hwnd, message, wParam, lParam);
			DoubleClickSelect(hwnd, LOWORD(lParam), HIWORD(lParam));
			DidDblClick = TRUE;
			NeedUpdate = TRUE;
			EraseBack = TRUE;
			SetCapture(hwnd);

		}
		else if (message == WM_MOUSEMOVE && DidDblClick && ((wParam & MK_LBUTTON) == MK_LBUTTON))
		{
			if (GetCapture() == hwnd)
			{
				short x = LOWORD(lParam),
					y = HIWORD(lParam);
				LRESULT nearXY = DirectSend(hwnd, EM_CHARFROMPOS, 0, MAKELPARAM(x, y));
				int charPos = LOWORD(nearXY);
				x = max(rect.left, min(x, rect.right));
				y = max(rect.top, min(y, rect.bottom));



				MoveSelect(hwnd, x, y, rect.right - rect.left, rect.bottom - rect.top);
				if (charPos == 65535)
				{
					if (x == rect.left)
					{
						DirectSend(hwnd, WM_HSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
					}
					else if (x == rect.right)
					{
						DirectSend(hwnd, WM_HSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
					}
					if (y == rect.top)
					{
						DirectSend(hwnd, WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
					}
					else if (y == rect.bottom)
					{
						DirectSend(hwnd, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
					}
				}
				else
				{
					DirectSend(hwnd, EM_SCROLLCARET, 0, 0);
				}
			}
		}
		else if (DidDblClick && (message == WM_LBUTTONUP || message == WM_RBUTTONUP))
		{
			DidDblClick = FALSE;
			SetCapture(hwnd);
			if (GetCapture() == hwnd) ReleaseCapture();
			r = CallWindowProc(wpOrigEditProc, hwnd, message, wParam, lParam);
		}
		else if (message != WM_PAINT)
		{
			//Default Processing of all other events
			//let the default event do it's thing if not draw event
			r = CallWindowProc(wpOrigEditProc, hwnd, message, wParam, lParam);
		}

		//Post processing
		//the text length usually changed after CallWindowProc
		LRESULT textLen = DirectSend(hwnd, WM_GETTEXTLENGTH, 0, 0);

		//Need update if the message processing created a new region
		NeedUpdate = NeedUpdate || GetUpdateRect(hwnd, 0, 0);
		//need update if the selection is different location
		NeedUpdate = NeedUpdate || ((selRange != OldselRange) && (selSize != 0)); //caret selection moved? Exclude single caret movement, make sure there is a selection
																				  //need update if the selection size changed
		NeedUpdate = NeedUpdate || (selSize != OldselSize);
		//need update if the selection is different size
		NeedUpdate = NeedUpdate || (posStart != OldposStart); //scroll not at same location?
															  //need updating if the text length changed
		NeedUpdate = NeedUpdate || (textLen != OldtextLen); //scroll not at same location?
															//if draw event of need update
		if (NeedUpdate || message == WM_PAINT)
		{
			//if need update and not region set
			if (EraseBack || GetUpdateRect(hwnd, 0, 0) == 0)
			{
				//create region entire windows

				InvalidateRect(hwnd, &rect, EraseBack);
				//MessageBeep(MB_ICONINFORMATION);
			}

			//draw
			DoDraw(hwnd);

		}
		//set the old data for comparing next call
		OldselRange = selRange;
		OldposStart = posStart;
		OldselSize = selSize;
		OldtextLen = textLen;
		//return what the CallWindowProc returned or 0
		return r;
	}


	gmexport void SetHilite(HWND ebhWnd)
	{
		//MessageBoxA(GetActiveWindow(), "In SetHighlite", "SetHilite", MB_ICONINFORMATION | MB_OK);


		HDC hdc = GetDC(ebhWnd);
		//Font size
		TEXTMETRIC textMetrics;
		GetTextMetrics(hdc, &textMetrics);
		ReleaseDC(ebhWnd, hdc);
		int ts[1];
		ts[0] = 16;
		SendMessage(ebhWnd, EM_SETLIMITTEXT, 65535, 0);
		SendMessage(ebhWnd, EM_SETTABSTOPS, 1, (LPARAM)ts); //set the tabs to half, essentially 4 chars intead of 8

		WNDPROC zProc = (WNDPROC)GetWindowLong(ebhWnd, GWL_WNDPROC);
		if (zProc != WndEditProc)
		{
			wpOrigEditProc = (WNDPROC)GetWindowLong(ebhWnd, GWL_WNDPROC);

			SetWindowLong(ebhWnd, GWL_WNDPROC, (LONG_PTR)WndEditProc);
		}


	}
	//example passing nothing
	gmexport void Initialize(LPSTR settingsfile)
	{
		int t = 10;
		//Show a message. GetActiveWindow always returns GM's Main window.
		InitKeywords();
		InitFunctions();
		InitLiterals();
		InitClassFuncs();
		InitStaticClass();
		ReadSyntax(settingsfile);
		//std::string s = "Hello";
		//MessageBoxA(GetActiveWindow(), s.c_str(), "SampleFunction", MB_ICONINFORMATION | MB_OK);

	}

} //extern "C"


