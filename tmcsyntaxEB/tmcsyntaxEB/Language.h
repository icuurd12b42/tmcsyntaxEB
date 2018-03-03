#pragma once
class ByteLookup {
private:
	TCHARString m_RefString;
	const TCHAR *m_Chars;
	int m_LastByte;
public:
	ByteLookup(){
		SetData(L" ");
	}
	void SetData(TCHARString RefString) 
	{
		m_RefString = RefString;
		m_Chars = m_RefString.c_str();
		m_LastByte = max(0,m_RefString.length()-2);
	}
	bool Valid(TCHAR val)
	{
		return m_Chars[min((short)val, m_LastByte)] != ' ';
	}
};
class WordLookup {
private:
	
	
public:
	unordered_set<TCHARString>m_Words;
	WordLookup() {}
	//void SetData(unordered_set<TCHARString>Words)
	//{
	//	m_Words = Words;
	//}
	bool Valid(TCHARString t)
	{
		return m_Words.count(t.c_str());
	}
	int ValidTCHAR(TCHAR* text)
	{
		//TCHARString ItemName;
		
		for (auto item : m_Words) {
			if (_wcsnicmp(item.c_str(), text, item.length()) == 0)
				return item.length();
		}
		return 0;
		
	}
	
};

class SingleWordLookup {
private:
	TCHARString m_Word;
	const TCHAR *m_Chars;
	int m_Len;
public:
	SingleWordLookup() {
		m_Word = L"";
		m_Chars = m_Word.c_str();
		m_Len = m_Word.length();
	}
	void SetData(TCHARString Word)
	{
		m_Word = Word;
		m_Chars = m_Word.c_str();
		m_Len = m_Word.length();
	}
	bool Valid(TCHAR *t)
	{
		return _wcsnicmp(t, m_Chars, m_Len) == 0;
	}
	const TCHAR* GetValue()
	{
		return m_Chars;
	}
	int GetLength()
	{
		return m_Len;
	}
};

class Language
{
private:
	
	TCHARString m_LanguageFile;
public:
	TCHARString m_LanguageName;
	WordLookup KEYWORDS;
	WordLookup LITERALS;
	WordLookup FUNCTIONS;
	WordLookup STATICCLASS;
	WordLookup CLASSFUNCS;
	WordLookup OCTPREFIXES;
	WordLookup HEXPREFIXES;
	WordLookup BINPREFIXES;
	WordLookup NUMSUFFIXES;
	WordLookup WORDBINDERS;


	SingleWordLookup SINGLELINECOMMENT;
	SingleWordLookup MULTILINECOMMENTSTART;
	SingleWordLookup MULTILINECOMMENTEND;

	ByteLookup WHITESPACE;
	ByteLookup QUOTES;
	ByteLookup STRINGESCAPE;
	ByteLookup VALIDNAMESTART;
	ByteLookup VALIDNAMEEND;
	ByteLookup COLORSET1CHARS;
	ByteLookup COLORSET2CHARS;
	ByteLookup COLORSET3CHARS;
	ByteLookup COLORSET4CHARS;
	ByteLookup SELECTBLOCKERS;
	ByteLookup SELECTREVERSEBLOCKERS;
	ByteLookup BASEBIN;
	ByteLookup BASEOCT;
	ByteLookup BASEDEC;
	ByteLookup BASEHEX;
	ByteLookup COMMENTCHARS;
	ByteLookup BINDERSPRECHECK;
	ByteLookup BINPRECHECK;
	ByteLookup OCTPRECHECK;
	ByteLookup HEXPRECHECK;
	ByteLookup SPLITTERS;
	ByteLookup NUMPRECHECK;

	COLORREF QUOTECOLOR = 0x5ADBE6;
	COLORREF COMMENTCOLOR = 0x65AD82;
	COLORREF NUMBERCOLOR = 0xA8FFB5;
	COLORREF UNKNOWNWORDCOLOR = 0xC8D4D4;
	COLORREF COLORSET1 = 0xA6b2b2;
	COLORREF COLORSET2 = 0xA6b2b2;
	COLORREF COLORSET3 = 0xA6b2b2;
	COLORREF COLORSET4 = 0xA6b2b2;
	COLORREF BADCHARSCOLOR = 0xC8D4FF;
	COLORREF KEYWORDCOLOR = 0x746498;
	COLORREF FUNCTIONCOLOR = 0xA375CB;
	COLORREF LITERALCOLOR = 0x1F58DD;
	COLORREF CLASSFUNCCOLOR = 0xA375CB;
	COLORREF STATICCLASSCOLOR = 0xA375CB;
	COLORREF SELECTEDTEXTCOLOR = 0x784F26;
	COLORREF USERFUNCTIONCOLOR = 0xA375CB;
	COLORREF TEXTBACKCOLOR = 0x272727;
	COLORREF WORDBINDERCOLOR = 0x1F58DD;

public:
	Language(LPCSTR LanguageName, LPCSTR LanguageFile);
	~Language();
	void LoadFile(LPCSTR filename);
};

