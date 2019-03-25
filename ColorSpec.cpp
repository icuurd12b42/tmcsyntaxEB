#include "stdafx.h"
#include "ColorSpec.h"


ColorSpec::ColorSpec(const TCHAR* ColorSpecName, const TCHAR* ColorSpecFile)
{
	m_ColorSpecName = wstring(ColorSpecName);
	m_ColorSpecFile = wstring(ColorSpecFile);
	LoadFile(WStringToString(m_ColorSpecFile).c_str());
	ColorsToArray();
}


ColorSpec::~ColorSpec()
{
}

void ColorSpec::LoadFile(LPCSTR filename)
{
	ifstream inFile;
	inFile.open(filename);
	if (!inFile)
	{
		string t = "Ooops!\r\nA request to load a color specification file failed. Syntax Highlithing for the language may not work properly.";
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
		if (line.compare("QUOTECOLOR") == 0)
		{
			std::getline(inFile, line);
			std::istringstream converter(line);
			converter >> std::hex >> QUOTECOLOR;
		}
		else if (line.compare("LINENOBACKCOLOR") == 0)
		{
			std::getline(inFile, line);
			std::istringstream converter(line);
			converter >> std::hex >> LINENOBACKCOLOR;
		}
		else if (line.compare("CARETCOLOR") == 0)
		{
			std::getline(inFile, line);
			std::istringstream converter(line);
			converter >> std::hex >> CARETCOLOR;
		}
		else if (line.compare("LINENOCOLOR") == 0)
		{
			std::getline(inFile, line);
			std::istringstream converter(line);
			converter >> std::hex >> LINENOCOLOR;
		}
		else if (line.compare("TEXTBACKCOLOR") == 0)
		{
			std::getline(inFile, line);
			std::istringstream converter(line);
			converter >> std::hex >> TEXTBACKCOLOR;
		}
		else if (line.compare("ACTIVELINECOLOR") == 0)
		{
			std::getline(inFile, line);
			std::istringstream converter(line);
			converter >> std::hex >> ACTIVELINECOLOR;
		}
		else if (line.compare("ACTIVELINENOCOLOR") == 0)
		{
			std::getline(inFile, line);
			std::istringstream converter(line);
			converter >> std::hex >> ACTIVELINENOCOLOR;
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
		
	}
	inFile.close();
}