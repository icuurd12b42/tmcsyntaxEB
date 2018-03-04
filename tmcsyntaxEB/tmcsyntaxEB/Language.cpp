#include "stdafx.h"
#include "Language.h"


Language::Language(const TCHAR* LanguageName, const TCHAR* LanguageFile)
{
	m_LanguageName = TCHARString(LanguageName);
	m_LanguageFile = TCHARString(LanguageFile);
	LoadFile(TCHARStringToString(m_LanguageFile).c_str());
}


Language::~Language()
{
}

void ReadWordSet(ifstream *inFile, unordered_set<TCHARString> *theset)
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
				theset->insert(stringToTCHARString(line));
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
		TCHARString TCs = stringToTCHARString(t);
		MessageBox(GetActiveWindow(), TCs.c_str(), L"TMCSyntaxEB.dll", MB_ICONINFORMATION | MB_OK);
		return;
	}
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
		else if (line.compare("COLORSET1") == 0)
		{
			std::getline(inFile, line);
			std::istringstream converter(line);
			converter >> std::hex >> COLORSET1;
		}
		else if (line.compare("COLORSET2") == 0)
		{
			std::getline(inFile, line);
			std::istringstream converter(line);
			converter >> std::hex >> COLORSET2;
		}
		else if (line.compare("COLORSET3") == 0)
		{
			std::getline(inFile, line);
			std::istringstream converter(line);
			converter >> std::hex >> COLORSET3;
		}
		else if (line.compare("COLORSET4") == 0)
		{
			std::getline(inFile, line);
			std::istringstream converter(line);
			converter >> std::hex >> COLORSET4;
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
		else if (line.compare("WORDBINDERCOLOR") == 0)
		{
			std::getline(inFile, line);
			std::istringstream converter(line);
			converter >> std::hex >> WORDBINDERCOLOR;
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
		else if (line.compare("WHITESPACE") == 0)
		{
			std::getline(inFile, line);
			WHITESPACE.SetData(stringToTCHARString(line));
		}
		else if (line.compare("COMMENTCHARS") == 0)
		{
			std::getline(inFile, line);
			COMMENTCHARS.SetData(stringToTCHARString(line));
		}
		else if (line.compare("SINGLELINECOMMENT") == 0)
		{
			std::getline(inFile, line);
			SINGLELINECOMMENT.SetData(stringToTCHARString(line));
		}
		else if (line.compare("MULTILINECOMMENTSTART") == 0)
		{
			std::getline(inFile, line);
			MULTILINECOMMENTSTART.SetData(stringToTCHARString(line));
		}
		else if (line.compare("MULTILINECOMMENTEND") == 0)
		{
			std::getline(inFile, line);
			MULTILINECOMMENTEND.SetData(stringToTCHARString(line));
		}
		else if (line.compare("QUOTES") == 0)
		{
			std::getline(inFile, line);
			QUOTES.SetData(stringToTCHARString(line));
		}
		else if (line.compare("BINDERSPRECHECK") == 0)
		{
			std::getline(inFile, line);
			BINDERSPRECHECK.SetData(stringToTCHARString(line));
		}
		else if (line.compare("BINPRECHECK") == 0)
		{
			std::getline(inFile, line);
			BINPRECHECK.SetData(stringToTCHARString(line));
		}
		else if (line.compare("OCTPRECHECK") == 0)
		{
			std::getline(inFile, line);
			OCTPRECHECK.SetData(stringToTCHARString(line));
		}
		else if (line.compare("HEXPRECHECK") == 0)
		{
			std::getline(inFile, line);
			HEXPRECHECK.SetData(stringToTCHARString(line));
		}
		else if (line.compare("STRINGESCAPE") == 0)
		{
			std::getline(inFile, line);
			STRINGESCAPE.SetData(stringToTCHARString(line));
		}
		else if (line.compare("VALIDNAMESTART") == 0)
		{
			std::getline(inFile, line);
			VALIDNAMESTART.SetData(stringToTCHARString(line));
		}
		else if (line.compare("VALIDNAMEEND") == 0)
		{
			std::getline(inFile, line);
			VALIDNAMEEND.SetData(stringToTCHARString(line));
		}
		else if (line.compare("COLORSET1CHARS") == 0)
		{
			std::getline(inFile, line);
			COLORSET1CHARS.SetData(stringToTCHARString(line));
		}
		else if (line.compare("COLORSET2CHARS") == 0)
		{
			std::getline(inFile, line);
			COLORSET2CHARS.SetData(stringToTCHARString(line));
		}
		else if (line.compare("COLORSET3CHARS") == 0)
		{
			std::getline(inFile, line);
			COLORSET3CHARS.SetData(stringToTCHARString(line));
		}
		else if (line.compare("COLORSET4CHARS") == 0)
		{
			std::getline(inFile, line);
			COLORSET4CHARS.SetData(stringToTCHARString(line));
		}
		else if (line.compare("SELECTBLOCKERS") == 0)
		{
			std::getline(inFile, line);
			SELECTBLOCKERS.SetData(stringToTCHARString(line));
		}
		else if (line.compare("SELECTREVERSEBLOCKERS") == 0)
		{
			std::getline(inFile, line);
			SELECTREVERSEBLOCKERS.SetData(stringToTCHARString(line));
		}
		else if (line.compare("BASEBIN") == 0)
		{
			std::getline(inFile, line);
			BASEBIN.SetData(stringToTCHARString(line));
		}
		else if (line.compare("BASEOCT") == 0)
		{
			std::getline(inFile, line);
			BASEOCT.SetData(stringToTCHARString(line));
		}
		else if (line.compare("BASEDEC") == 0)
		{
			std::getline(inFile, line);
			BASEDEC.SetData(stringToTCHARString(line));
		}
		else if (line.compare("BASEHEX") == 0)
		{
			std::getline(inFile, line);
			BASEHEX.SetData(stringToTCHARString(line));
		}
		else if (line.compare("SPLITTERS") == 0)
		{
			std::getline(inFile, line);
			SPLITTERS.SetData(stringToTCHARString(line));
		}
		else if (line.compare("NUMPRECHECK") == 0)
		{
			std::getline(inFile, line);
			NUMPRECHECK.SetData(stringToTCHARString(line));
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
	}
	inFile.close();
}
