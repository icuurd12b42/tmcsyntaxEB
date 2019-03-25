#pragma once
class ByteLookup {
private:
	wstring m_RefString;
	const TCHAR *m_Chars;
	size_t m_LastByte;
public:
	ByteLookup(){
		SetData(L" ");
	}
	void SetData(wstring RefString)
	{
		m_RefString = RefString;
		m_Chars = m_RefString.c_str();
		m_LastByte = max(0,m_RefString.length()-2);
	}
	bool Valid(TCHAR val)
	{
		return m_Chars[min((unsigned short)val, m_LastByte)] != ' ';
	}
};
class WordLookup {
private:
	
	
public:
	unordered_set<wstring>m_Words;
	WordLookup() {}
	//void SetData(unordered_set<TCHARString>Words)
	//{
	//	m_Words = Words;
	//}
	bool Valid(wstring t)
	{
		return m_Words.count(t.c_str());
	}
	size_t ValidTCHAR(TCHAR* text)
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
	wstring m_Word;
	const TCHAR *m_Chars;
	size_t m_Len;
public:
	SingleWordLookup() {
		m_Word = L"";
		m_Chars = m_Word.c_str();
		m_Len = m_Word.length();
	}
	void SetData(wstring Word)
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
	size_t GetLength()
	{
		return m_Len;
	}
};

class Language
{
private:
	
	wstring m_LanguageFile;
public:
	wstring m_LanguageName;
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
	WordLookup BRACELESSWORDS;


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
	ByteLookup WORDLISTSPLIT;
	ByteLookup BRACELESSWORDSPRECHECK;



public:
	Language(const TCHAR* LanguageName, const TCHAR* LanguageFile);
	~Language();
	void LoadFile(LPCSTR filename);
};

