#include "stdafx.h"
#include "Language.h"


Language::Language(const TCHAR* LanguageName, const TCHAR* LanguageFile)
{
	m_LanguageName = wstring(LanguageName);
	m_LanguageFile = wstring(LanguageFile);
	LoadFile(WStringToString(m_LanguageFile).c_str());
}


Language::~Language()
{
}

void ReadWordSet(ifstream *inFile, unordered_set<wstring> *theset)
{
	;
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
				theset->insert(StringToWString(line));
				addct++;
			}
		}
	}
	
}
void Language::LoadFile(LPCSTR filename)
{
	ifstream inFile;
	inFile.open(filename);
	if (!inFile)
	{
		string t = "Ooops!\r\nA request to load a language file failed. Syntax Highlithing for the language may not work properly.";
		t += "\r\n";
		t += filename;
		wstring TCs = StringToWString(t);
		MessageBox(GetActiveWindow(), TCs.c_str(), L"TMCSyntaxEB.dll", MB_ICONINFORMATION | MB_OK);
		return;
	}
	string line;
	wstring wString;
	while (!inFile.eof())
	{
		std::getline(inFile, line);
		if (line.compare("WHITESPACE") == 0)
		{
			std::getline(inFile, line);
			WHITESPACE.SetData(StringToWString(line));
		}
		else if (line.compare("COMMENTCHARS") == 0)
		{
			std::getline(inFile, line);
			COMMENTCHARS.SetData(StringToWString(line));
		}
		else if (line.compare("SINGLELINECOMMENT") == 0)
		{
			std::getline(inFile, line);
			SINGLELINECOMMENT.SetData(StringToWString(line));
		}
		else if (line.compare("MULTILINECOMMENTSTART") == 0)
		{
			std::getline(inFile, line);
			MULTILINECOMMENTSTART.SetData(StringToWString(line));
		}
		else if (line.compare("MULTILINECOMMENTEND") == 0)
		{
			std::getline(inFile, line);
			MULTILINECOMMENTEND.SetData(StringToWString(line));
		}
		else if (line.compare("QUOTES") == 0)
		{
			std::getline(inFile, line);
			QUOTES.SetData(StringToWString(line));
		}
		else if (line.compare("BINDERSPRECHECK") == 0)
		{
			std::getline(inFile, line);
			BINDERSPRECHECK.SetData(StringToWString(line));
		}
		else if (line.compare("BINPRECHECK") == 0)
		{
			std::getline(inFile, line);
			BINPRECHECK.SetData(StringToWString(line));
		}
		else if (line.compare("OCTPRECHECK") == 0)
		{
			std::getline(inFile, line);
			OCTPRECHECK.SetData(StringToWString(line));
		}
		else if (line.compare("HEXPRECHECK") == 0)
		{
			std::getline(inFile, line);
			HEXPRECHECK.SetData(StringToWString(line));
		}
		else if (line.compare("STRINGESCAPE") == 0)
		{
			std::getline(inFile, line);
			STRINGESCAPE.SetData(StringToWString(line));
		}
		else if (line.compare("VALIDNAMESTART") == 0)
		{
			std::getline(inFile, line);
			VALIDNAMESTART.SetData(StringToWString(line));
		}
		else if (line.compare("VALIDNAMEEND") == 0)
		{
			std::getline(inFile, line);
			VALIDNAMEEND.SetData(StringToWString(line));
		}
		else if (line.compare("COLORSET1CHARS") == 0)
		{
			std::getline(inFile, line);
			COLORSET1CHARS.SetData(StringToWString(line));
		}
		else if (line.compare("COLORSET2CHARS") == 0)
		{
			std::getline(inFile, line);
			COLORSET2CHARS.SetData(StringToWString(line));
		}
		else if (line.compare("COLORSET3CHARS") == 0)
		{
			std::getline(inFile, line);
			COLORSET3CHARS.SetData(StringToWString(line));
		}
		else if (line.compare("COLORSET4CHARS") == 0)
		{
			std::getline(inFile, line);
			COLORSET4CHARS.SetData(StringToWString(line));
		}
		else if (line.compare("SELECTBLOCKERS") == 0)
		{
			std::getline(inFile, line);
			SELECTBLOCKERS.SetData(StringToWString(line));
		}
		else if (line.compare("SELECTREVERSEBLOCKERS") == 0)
		{
			std::getline(inFile, line);
			SELECTREVERSEBLOCKERS.SetData(StringToWString(line));
		}
		else if (line.compare("BASEBIN") == 0)
		{
			std::getline(inFile, line);
			BASEBIN.SetData(StringToWString(line));
		}
		else if (line.compare("BASEOCT") == 0)
		{
			std::getline(inFile, line);
			BASEOCT.SetData(StringToWString(line));
		}
		else if (line.compare("BASEDEC") == 0)
		{
			std::getline(inFile, line);
			BASEDEC.SetData(StringToWString(line));
		}
		else if (line.compare("BASEHEX") == 0)
		{
			std::getline(inFile, line);
			BASEHEX.SetData(StringToWString(line));
		}
		else if (line.compare("SPLITTERS") == 0)
		{
			std::getline(inFile, line);
			SPLITTERS.SetData(StringToWString(line));
		}
		else if (line.compare("NUMPRECHECK") == 0)
		{
			std::getline(inFile, line);
			NUMPRECHECK.SetData(StringToWString(line));
		}
		else if (line.compare("WORDLISTSPLIT") == 0)
		{
			std::getline(inFile, line);
			WORDLISTSPLIT.SetData(StringToWString(line));
		}
		else if (line.compare("BRACELESSWORDSPRECHECK") == 0)
		{
			std::getline(inFile, line);
			BRACELESSWORDSPRECHECK.SetData(StringToWString(line));
		}
		else if (line.compare("KEYWORDS {") == 0)
		{
			ReadWordSet(&inFile, &KEYWORDS.m_Words);
		}
		else if (line.compare("LITERALS {") == 0)
		{
			ReadWordSet(&inFile, &LITERALS.m_Words);
		}
		else if (line.compare("FUNCTIONS {") == 0)
		{
			ReadWordSet(&inFile, &FUNCTIONS.m_Words);
		}
		else if (line.compare("CLASSFUNCS {") == 0)
		{
			ReadWordSet(&inFile, &CLASSFUNCS.m_Words);
		}
		else if (line.compare("STATICCLASS {") == 0)
		{
			ReadWordSet(&inFile, &STATICCLASS.m_Words);
		}
		else if (line.compare("OCTPREFIXES {") == 0)
		{
			ReadWordSet(&inFile, &OCTPREFIXES.m_Words);
		}
		else if (line.compare("HEXPREFIXES {") == 0)
		{
			ReadWordSet(&inFile, &HEXPREFIXES.m_Words);
		}
		else if (line.compare("BINPREFIXES {") == 0)
		{
			ReadWordSet(&inFile, &BINPREFIXES.m_Words);
		}
		else if (line.compare("NUMSUFFIXES {") == 0)
		{
			ReadWordSet(&inFile, &NUMSUFFIXES.m_Words);
		}
		else if (line.compare("WORDBINDERS {") == 0)
		{
			ReadWordSet(&inFile, &WORDBINDERS.m_Words);
		}
		else if (line.compare("BRACELESSWORDS {") == 0)
		{
			ReadWordSet(&inFile, &BRACELESSWORDS.m_Words);
		}
	}
	inFile.close();
}
