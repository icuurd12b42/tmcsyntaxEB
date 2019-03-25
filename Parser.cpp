#include "stdafx.h"
#include "HookedCtrl.h"
#include "Parser.h"


Parser::Parser()
{
}


Parser::~Parser()
{
}

//these tags are used to place character combination temporarely in the string to allow string replaces to take place
//all tags are never found in text content. used to replace '==' for example so we can do something with '=' without 
//affecting the compound operators
TCHAR tag1[] = { 1,0 };
TCHAR tag2[] = { 2,0 };
TCHAR tag3[] = { 3,0 };
TCHAR tag4[] = { 4,0 };
TCHAR tag5[] = { 5,0 };
TCHAR tag6[] = { 6,0 };
TCHAR tag7[] = { 7,0 };
TCHAR tag8[] = { 8,0 };  
//TCHAR tag9[] = { 9,0 }; //oops not this one
//TCHAR tag10[] = { 10,0 }; //oops not this one
TCHAR tag11[] = { 11,0 };
TCHAR tag12[] = { 12,0 };
//TCHAR tag13[] = { 13,0 }; //oops not this one
TCHAR tag14[] = { 14,0 };
TCHAR tag15[] = { 15,0 };
TCHAR tag16[] = { 16,0 };
TCHAR tag17[] = { 17,0 };
TCHAR tag18[] = { 18,0 };
TCHAR tag19[] = { 19,0 };
TCHAR tag20[] = { 20,0 };
TCHAR tag21[] = { 21,0 };
TCHAR tag22[] = { 22,0 };
TCHAR tag23[] = { 23,0 };
TCHAR tag24[] = { 24,0 };
TCHAR tag25[] = { 25,0 };
TCHAR tag26[] = { 26,0 };
TCHAR tag27[] = { 27,0 };

wstring& FormatLine(wstring &line)
{
	
	
	//first replace all multi chars operator with tags
	ReplaceAll(line, L"===", tag1);
	ReplaceAll(line, L"==", tag2);
	ReplaceAll(line, L"!=", tag3);
	ReplaceAll(line, L">=", tag4);
	ReplaceAll(line, L"<=", tag5);
	ReplaceAll(line, L"+=", tag6);
	ReplaceAll(line, L"-=", tag7);
	ReplaceAll(line, L"*=", tag8);
	ReplaceAll(line, L"/=", tag24);
	ReplaceAll(line, L"%=", tag25);
	ReplaceAll(line, L"&=", tag11);
	ReplaceAll(line, L"|=", tag12);
	ReplaceAll(line, L"^=", tag26);
	ReplaceAll(line, L"<<=", tag14);
	ReplaceAll(line, L">>=", tag15);
	ReplaceAll(line, L"->", tag16);
	ReplaceAll(line, L"++", tag17);
	ReplaceAll(line, L"--", tag18);
	ReplaceAll(line, L"&&", tag19);
	ReplaceAll(line, L"||", tag20);
	ReplaceAll(line, L"<<", tag21);
	ReplaceAll(line, L">>", tag22);
	ReplaceAll(line, L"::", tag23);
	ReplaceAll(line, L"=>", tag27);

	//now we can format the single byte operators
	//add a space in from and behind each operator
	ReplaceAll(line, L"=", L" = ");
	ReplaceAll(line, L"+", L" + ");
	ReplaceAll(line, L"-", L" - ");
	ReplaceAll(line, L"/", L" / ");
	ReplaceAll(line, L"\\", L" \\ ");
	ReplaceAll(line, L"%", L" % ");
	ReplaceAll(line, L"|", L" | ");

	//these 2 requie post prcessing later
	ReplaceAll(line, L"&", L" &");
	ReplaceAll(line, L"*", L" *");

	ReplaceAll(line, L"^", L" ^ ");
	ReplaceAll(line, L"?", L" ? ");
	//ReplaceAll(line, L":", L": ");
	

	//now place back the multi byte ops and do the spaces
	

	ReplaceAll(line, tag2, L"==");
	ReplaceAll(line, L"==", L" == ");

	ReplaceAll(line, tag1, L"===");
	ReplaceAll(line, L"===", L" === ");

	ReplaceAll(line, tag3, L"!=");
	ReplaceAll(line, L"!=", L" != ");

	ReplaceAll(line, tag4, L">=");
	ReplaceAll(line, L">=", L" >= ");

	ReplaceAll(line, tag5, L"<=");
	ReplaceAll(line, L"<=", L" <= ");

	ReplaceAll(line, tag6, L"+=");
	ReplaceAll(line, L"+=", L" += ");

	ReplaceAll(line, tag7, L"-=");
	ReplaceAll(line, L"-=", L" -= ");

	ReplaceAll(line, tag8, L"*=");
	ReplaceAll(line, L"*=", L" *= ");

	ReplaceAll(line, tag24, L"/=");
	ReplaceAll(line, L"/=", L" /= ");

	ReplaceAll(line, tag25, L"%=");
	ReplaceAll(line, L"%=", L" %= ");

	ReplaceAll(line, tag11, L"&=");
	ReplaceAll(line, L"&=", L" &= ");

	ReplaceAll(line, tag12, L"|=");
	ReplaceAll(line, L"|=", L" |= ");

	ReplaceAll(line, tag26, L"^=");
	ReplaceAll(line, L"^=", L" ^= ");

	//ReplaceAll(line, L"<<", tag21);
	ReplaceAll(line, tag21, L"<<");
	ReplaceAll(line, L"<<", L" << ");

	//ReplaceAll(line, L">>", tag22);
	ReplaceAll(line, tag22, L">>");
	ReplaceAll(line, L">>", L" >> ");

	//ReplaceAll(line, L"<<=", tag14);
	ReplaceAll(line, tag14, L"<<=");
	ReplaceAll(line, L"<<=", L" <<= ");

	//ReplaceAll(line, L">>=", tag15);
	ReplaceAll(line, tag15, L">>=");
	ReplaceAll(line, L">>=", L" >>= ");

	ReplaceAll(line, tag16, L"->");
	
	ReplaceAll(line, tag17, L"++");
	

	ReplaceAll(line, tag18, L"--");
	
	ReplaceAll(line, tag20, L"||");
	ReplaceAll(line, L"||", L" || ");

	ReplaceAll(line, tag23, L"::");
	
	ReplaceAll(line, tag19, L"&&");
	ReplaceAll(line, L"&&", L" && ");

	ReplaceAll(line, tag27, L"=>");
	ReplaceAll(line, L"=>", L" => ");

	//& and * can be pointer related... we'll only do the spacing for numbers
	ReplaceAll(line, L"&0", L"& 0");
	ReplaceAll(line, L"&1", L"& 1");
	ReplaceAll(line, L"&2", L"& 2");
	ReplaceAll(line, L"&3", L"& 3");
	ReplaceAll(line, L"&4", L"& 4");
	ReplaceAll(line, L"&5", L"& 5");
	ReplaceAll(line, L"&6", L"& 6");
	ReplaceAll(line, L"&7", L"& 7");
	ReplaceAll(line, L"&8", L"& 8");
	ReplaceAll(line, L"&9", L"& 9");

	ReplaceAll(line, L"*0", L"* 0");
	ReplaceAll(line, L"*1", L"* 1");
	ReplaceAll(line, L"*2", L"* 2");
	ReplaceAll(line, L"*3", L"* 3");
	ReplaceAll(line, L"*4", L"* 4");
	ReplaceAll(line, L"*5", L"* 5");
	ReplaceAll(line, L"*6", L"* 6");
	ReplaceAll(line, L"*7", L"* 7");
	ReplaceAll(line, L"*8", L"* 8");
	ReplaceAll(line, L"*9", L"* 9");

	

	//fix "( "
	while (IsInStr(line, L"( ")) ReplaceAll(line, L"( ", L"(");
	//now fix " ("
	while (IsInStr(line, L" (")) ReplaceAll(line, L" (", L"(");
	//now fix " )"
	while (IsInStr(line, L" )")) ReplaceAll(line, L" )", L")");
	//now fix "[ "
	while (IsInStr(line, L"[ ")) ReplaceAll(line, L"[ ", L"[");
	//now fix " ]"
	while (IsInStr(line, L" ]")) ReplaceAll(line, L" ]", L"]");
	//now fix " ,"
	while (IsInStr(line, L" ,")) ReplaceAll(line, L" ,", L",");
	//now fix ",nospace"
	ReplaceAll(line, L",", L", ");
	

	//Finaly fix all the double spaces we have created
	while (IsInStr(line, L"  ")) ReplaceAll(line, L"  ", L" ");
	return line;

}



void Parser::IndentKR()
{
	
	//working
	/*
	int MaxBlanks = 1; //the number of blank lines allowed
	bool debug = KEY_STATE_DOWN(VK_CONTROL);
	std::list<wstring>::iterator iLine;
	std::list<int>::iterator iType;
	iType = m_IndentLinesTypes.begin();
	int LastType = -1;
	size_t BraceStartPos = 0;
	int IndentLevel = 0;
	int NumBlanks = 0;
	wstring prefix;
	wstring line;
	wstring suffix;
	wstring IndentS = L"";
	//add to the numbers so it matches what expected above
	NumBlanks += 1;
	MaxBlanks += 1;
	bool IsNewLine = false;
	bool LastIsNewLine = false;
	int CaseIndentStart = 0;
	bool DidSomeCase = false;
	for (iLine = m_IndentLines.begin(); iLine != m_IndentLines.end(); ++iLine)
	{
		prefix = IndentS;
		line = iLine->c_str();
		suffix = L"";
		IsNewLine = *iType == LINETYPE::NEW_LINE || *iType == LINETYPE::COMMENT_NEW_LINE;
		if (IsNewLine)	NumBlanks++;
		else NumBlanks = 0;

		switch (*iType)
		{
		case (LINETYPE::INDENT_START):
			if (debug) line = L"INDENT_START" + line;
			if (CaseIndentStart != 0 && DidSomeCase == false && line == L"{")
			{
				CaseIndentStart = 0;
			}
			if (!LastIsNewLine)
			{
				prefix = L"";
			}
			else if ((CaseIndentStart != 0) && (IndentLevel >= CaseIndentStart))
			{
				prefix += L"\t";
			}
			if (BraceStartPos != 0 && line == L"{") //if { and we have a line to rewind to for it
			{
				prefix = L"";
				m_IndentedCode.erase(BraceStartPos, m_IndentedCode.length() - BraceStartPos);
			}

			IndentLevel++;	IndentS = L""; repeat(IndentLevel) IndentS += L"\t";
			break;
		case (LINETYPE::INDENT_END):
			if (debug) line = L"INDENT_END" + line;
			IndentLevel--;	IndentLevel = max(0, IndentLevel); IndentS = L""; repeat(IndentLevel) IndentS += L"\t";
			prefix = IndentS;
			if (!LastIsNewLine) prefix = L"";
			else if ((CaseIndentStart != 0) && (IndentLevel >= CaseIndentStart)) prefix += L"\t";
			break;
		case (LINETYPE::COMMENT_NEW_LINE):
			if (debug) line = L"COMMENT_NEWLINE" + line;
			prefix = L"";
			suffix = L"\r\n";
			break;
		case (LINETYPE::NEW_LINE):
			if (debug) line = L"NEW_LINE" + line;
			prefix = L"";
			suffix = L"\r\n";
			break;
		case (LINETYPE::CODE_LINE):
			FormatLine(line);
			if (debug) line = L"CODE_LINE" + line;

			if (!LastIsNewLine) prefix = L"";
			else
			{
				if (CaseIndentStart > IndentLevel) CaseIndentStart = 0;
				else if ((CaseIndentStart != 0) && (IndentLevel >= CaseIndentStart) && !IsPossibleInline(line)) //working some
				{
					DidSomeCase = true;
					prefix += L"\t";
				}
				else if (CaseIndentStart <= IndentLevel && IsPossibleInline(line))
				{
					CaseIndentStart = IndentLevel;
					DidSomeCase = false;
				}

			}
			break;
		case (LINETYPE::QUOTED_CONTENT):
			if (debug) line = L"QUOTED_CONTENT" + line;
			if (!LastIsNewLine) prefix = L"";
			else if ((CaseIndentStart != 0) && (IndentLevel >= CaseIndentStart)) prefix += L"\t";
			break;
		case (LINETYPE::COMMENT_START):
			if (debug) line = L"COMMENT_START" + line;
			if (!LastIsNewLine) prefix = L"";
			else if ((CaseIndentStart != 0) && (IndentLevel >= CaseIndentStart)) prefix += L"\t";
			break;
		case (LINETYPE::COMMENT_END):
			if (debug) line = L"COMMENT_END" + line;
			if (!LastIsNewLine) prefix = L"";
			else if ((CaseIndentStart != 0) && (IndentLevel >= CaseIndentStart)) prefix += L"\t";
			break;
		case (LINETYPE::COMMENT_CONTENT):
			if (debug) line = L"COMMENT_CONTENT" + line;
			if (!LastIsNewLine) prefix = L"";
			else if ((CaseIndentStart != 0) && (IndentLevel >= CaseIndentStart)) prefix += L"\t";
			break;
		case (LINETYPE::SINGLE_INDENT):
			if (debug) line = L"SINGLE_INDENT" + line;
			prefix += L"\t";
			if ((CaseIndentStart != 0) && (IndentLevel >= CaseIndentStart)) prefix += L"\t";
			break;

		}


		if (NumBlanks > MaxBlanks)
		{
			prefix = L"";
			line = L"";
			suffix = L"";
		}
		m_IndentedCode += prefix;
		m_IndentedCode += line;


		if (*iType == LINETYPE::CODE_LINE || (*iType == LINETYPE::NEW_LINE && BraceStartPos == 0)) BraceStartPos = m_IndentedCode.length();
		else if (*iType != LINETYPE::NEW_LINE) BraceStartPos = 0;

		//remember to remove this debug line
		BraceStartPos = 0;

		m_IndentedCode += suffix;
		LastType = *iType;
		LastIsNewLine = IsNewLine;
		iType++;

	}
	*/
}

void Parser::IndentAllman()
{
	//m_IndentedCode = L"Allman";
	
}

void Parser::IndentGNU()
{
	//m_IndentedCode = L"GNU";
	
}

void Parser::IndentWhitesmiths()
{
	//m_IndentedCode = L"Whitesmiths";
	
}


void Parser::IndentHorstmann()
{
	//m_IndentedCode = L"Horstmann";
	
}

void Parser::RecurseIndentReplaceType(CodeSection *Current, SECTIONTYPE search, SECTIONTYPE replace)
{
	

	if(Current == NULL) 
		return;
	if (Current->m_SectionType == search)
	{
		Current->m_SectionType = replace;
	}
	CodeSection *n = Current->m_FirstChild;
	while (n != NULL)
	{
		RecurseIndentReplaceType(n, search,  replace);
		n = n->m_NextSibling;
	}
}
long m_nodect = 0;
void Parser::RecurseIndentCode(CodeSection *Current)
{
	

	
	wstring tabs = L"";
	
	if (Current->m_SectionType == SECTIONTYPE::NEW_LINE)
	{
		m_IndentedCode += L"\r\n";
		m_IndentDidNewLine = true;
	}
	int oldlevel = m_IndentLevel;
	m_IndentLevel -= (int)(Current->m_SectionType == SECTIONTYPE::BLOCK_END);
	m_IndentLevel = max(0, m_IndentLevel);
	m_IndentLevel += (int)(Current->m_SectionType == SECTIONTYPE::BLOCK_START);
	if (oldlevel != m_IndentLevel)
	{
		m_IndentTabs = L"";
		repeat(m_IndentLevel) m_IndentTabs += L"\t";
	}
	if (m_IndentDidNewLine) tabs = m_IndentTabs;

	if (Current->m_CodeText.length())
	{
		m_IndentDidNewLine = false;
		m_IndentedCode += tabs + Current->m_CodeText;
	}

	//new 452
	//*
	if (Current->m_FirstChild)
	{
		m_nodect++;

		wstringstream ss;
		ss << "<" << m_nodect << "-";
		m_IndentedCode += ss.str().c_str();
		RecurseIndentCode(Current->m_FirstChild);
		ss.clear();
		ss << "-" << m_nodect << ">";
		m_IndentedCode += ss.str().c_str();
		m_nodect--;

	}
	//if (Current->m_NextSibling)
	//{
	//	RecurseIndentCode(Current->m_NextSibling);
	//}
	if (Current->m_PreviousSibling == NULL) //less recursion, if top of chain, delete siblings
	{
		CodeSection *n = Current->m_NextSibling;
		while (n != NULL)
		{
			RecurseIndentCode(n);
			n = n->m_NextSibling;
		}
	}

	//old 25
	/*/
	CodeSection *n = Current->m_FirstChild;
	while (n != NULL)
	{
		RecurseIndentCode(n);
		n = n->m_NextSibling;
	}
	//*/
	
	
}

CodeSection * Parser::RecurseIndent(CodeSection *Parent)
{
	/* example to test
	do
		Current = NULL;
	while (Current = NULL);
	*/




	CodeSection * Current = Parent->NewSection(SECTIONTYPE::STATEMENT);;
	wstring keyword = L"";
	size_t kwlen = 0;
	while (m_ReadAt < m_DataLen)
	{
		TCHAR c = DATA_CHAR(m_EBText, m_ReadAt);
		if (c == '\r')
		{
			Current = Parent->NewSection(SECTIONTYPE::NEW_LINE);
			m_ReadAt += 2;
			DoIndentLeadingWhitespace();
			Current = Parent->NewSection(SECTIONTYPE::STATEMENT);
			continue;
		}
		else if (c == '{')
		{
			m_ReadAt++;
			CodeSection *Block = Parent->NewSection(SECTIONTYPE::BLOCK);
			CodeSection *BlockStart = Block->NewSection(SECTIONTYPE::BLOCK_START);
			Block->m_CodeText += c;
			//Block->NewSection(SECTIONTYPE::STATEMENT);
			CodeSection *BlockEnd = RecurseIndent(Block);
			if (Parent->m_SectionType == SECTIONTYPE::IN_BRACE_OR_EOL_BLOCK)
			{
				break;
			}
			Current = Parent->NewSection(SECTIONTYPE::STATEMENT);
			continue;
		}
		else if (c == '}')
		{
			m_ReadAt++;
			(Current = Parent->NewSection(SECTIONTYPE::BLOCK_END))->m_CodeText += c;
			break;
		}
		else if (c == '(')
		{
			m_ReadAt++;
			CodeSection *Block = Parent->NewSection(SECTIONTYPE::BLOCK);
			CodeSection *BlockStart = Block->NewSection(SECTIONTYPE::BLOCK_START);
			Block->m_CodeText += c;
			//Block->NewSection(SECTIONTYPE::STATEMENT);
			CodeSection *BlockEnd = RecurseIndent(Block);
			if (Parent->m_SectionType == SECTIONTYPE::IN_PAREN_BLOCK)
			{
				break;
			}
			Current = Parent->NewSection(SECTIONTYPE::STATEMENT);
			continue;
		}
		else if (c == ')')
		{
			m_ReadAt++;
			(Current = Parent->NewSection(SECTIONTYPE::BLOCK_END))->m_CodeText += c;
			
			break;
		}
		else if (c == '[')
		{
			m_ReadAt++;
			CodeSection *Block = Parent->NewSection(SECTIONTYPE::BLOCK);
			CodeSection *BlockStart = Block->NewSection(SECTIONTYPE::BLOCK_START);
			Block->m_CodeText += c;
			//Block->NewSection(SECTIONTYPE::STATEMENT);
			CodeSection *BlockEnd = RecurseIndent(Block);
			
			Current = Parent->NewSection(SECTIONTYPE::STATEMENT);
			continue;
		}
		else if (c == ']')
		{
			m_ReadAt++;
			(Current = Parent->NewSection(SECTIONTYPE::BLOCK_END))->m_CodeText += c;
			break;
		}
		else if (c == ';')
		{
			m_ReadAt++;
			(Current = Parent->NewSection(SECTIONTYPE::EOS))->m_CodeText += c;

			if (Parent->m_SectionType == SECTIONTYPE::IN_BRACE_OR_EOL_BLOCK)
			{
				break;
			}
			Current = Parent->NewSection(SECTIONTYPE::STATEMENT);
			continue;
		}
		else if (m_Language->QUOTES.Valid(c))
		{
			(Current = Parent->NewSection(SECTIONTYPE::QUOTED_CONTENT))->m_CodeText = DoIndentQuotedText();
			Current = Parent->NewSection(SECTIONTYPE::STATEMENT);
			continue;
		}
		else if (m_Language->COMMENTCHARS.Valid(c))
		{
			if (m_Language->SINGLELINECOMMENT.Valid(DATA_PTR(m_EBText, m_ReadAt)))
			{
				(Current = Parent->NewSection(SECTIONTYPE::QUOTED_CONTENT))->m_CodeText = DoIndentSingleLineComment();
				(Current = Parent->NewSection(SECTIONTYPE::NEW_LINE));
				DoIndentLeadingWhitespace();
				Current = Parent->NewSection(SECTIONTYPE::STATEMENT);
				continue;
			}
			else if (m_Language->MULTILINECOMMENTSTART.Valid(DATA_PTR(m_EBText, m_ReadAt)))
			{
				bool done = false;
				//this gets the position of the /*
				size_t tcount = m_ReadAt - (size_t)SendMessage(m_Ctrl->m_hWnd, EM_LINEINDEX, SendMessage(m_Ctrl->m_hWnd, EM_LINEFROMCHAR, m_ReadAt, 0), 0);
				(Current = Parent->NewSection(SECTIONTYPE::QUOTED_CONTENT))->m_CodeText = DoIndentMultiLineComment(true, &done);
				while (!done)
				{
					(Current = Parent->NewSection(SECTIONTYPE::NEW_LINE));
					DoIndentLeadingWhitespace(tcount); //do not trim multiline quotes without knowing where it started from
					(Current = Parent->NewSection(SECTIONTYPE::QUOTED_CONTENT))->m_CodeText = DoIndentMultiLineComment(false, &done);
				}
				Current = Parent->NewSection(SECTIONTYPE::STATEMENT);
				continue;
			}
		}
		else if (m_Language->BRACELESSWORDSPRECHECK.Valid(c))
		{
			//if the accumulated string is a keyword like for, if, etc, need to handle bracelss contitionals
			kwlen = m_Language->BRACELESSWORDS.ValidTCHAR(DATA_PTR(m_EBText, m_ReadAt));
			if (kwlen)
			{
				repeat(kwlen)
				{
					Current->m_CodeText += DATA_CHAR(m_EBText, m_ReadAt++);
				}
				keyword = Current->m_CodeText;
				kwlen = keyword.length();
				
				CodeSection *Block = Parent->NewSection(SECTIONTYPE::IN_PAREN_BLOCK);
				CodeSection *BlockStart = Block->NewSection(SECTIONTYPE::STUB_BLOCK_START);
				CodeSection *BlockEnd = RecurseIndent(Block);
				Current = Block;
				Current = Parent->NewSection(SECTIONTYPE::STATEMENT);
				
				//if/while/for(contition) //see if "(" is the section
				if (Block->m_LastChild->m_CodeText == L"(")
				{
					//read ()
					//read {} if any or somthing();
					Block = Parent->NewSection(SECTIONTYPE::IN_BRACE_OR_EOL_BLOCK);
					BlockStart = Block->NewSection(SECTIONTYPE::STUB_BLOCK_START);
					BlockEnd = RecurseIndent(Block);
					Current = Block;
					Current = Parent->NewSection(SECTIONTYPE::STATEMENT);
				}
				if (Block->m_LastChild->m_CodeText == L"{")
				{
					Current = Parent->NewSection(SECTIONTYPE::STATEMENT);
				}
				else //if (Block->m_LastChild->m_CodeText == L";")
				{
					//we read up to the first ; encounter, it was a braceless if/for/while...
					BlockStart->m_SectionType = SECTIONTYPE::BLOCK_START;
					Current = Parent->NewSection(SECTIONTYPE::BLOCK_END);
					Current = Parent->NewSection(SECTIONTYPE::STATEMENT);
				}
				if (Parent->m_SectionType == SECTIONTYPE::IN_BRACE_OR_EOL_BLOCK)
				{
					Parent->m_FirstChild->m_SectionType = SECTIONTYPE::BLOCK_START;
					Current = Parent->NewSection(SECTIONTYPE::BLOCK_END);

					Current = Parent->NewSection(SECTIONTYPE::STATEMENT);
					break;
				}

				//Current = Parent->NewSection(SECTIONTYPE::STATEMENT);
				//if (Parent->m_SectionType == SECTIONTYPE::BRACELESS_BLOCK)
				//{
				//	Parent->m_FirstChild->m_SectionType = SECTIONTYPE::BLOCK_START;
				//	Current = Parent->NewSection(SECTIONTYPE::BLOCK_END);
				//	break;
				//}
				//break;
				continue;
			}
			
		}
		Current->m_CodeText += c;
		m_ReadAt++;
		
	}
	

	return Current;
}

wstring& Parser::DoIndent(HLOCAL hEBText, size_t TextLen, int Style)
{
	m_IndentedCode = L"";
	m_EBText = (TCHAR*)LocalLock(hEBText);
	if (m_EBText != NULL)
	{
		//initial settings
		m_ReadAt = 0;
		m_DataLen = TextLen;
		m_IndentLevel = 0;
		m_IndentDidNewLine = false;
		//language used
		m_Language = m_Ctrl->GetLanguage();
		CodeSection *TopNode = new CodeSection();
		while (m_ReadAt < m_DataLen)
		{
			RecurseIndent(TopNode);
		}


		RecurseIndentCode(TopNode);
		delete TopNode;
		LocalUnlock(hEBText);
	}

	return m_IndentedCode;
}

void Parser::DoColorize(HLOCAL hEBText, size_t TextLen, size_t startpos, size_t endpos, HLOCAL hColorBytes, size_t ColBufferSize) {
	//hEBText is the HLOCAL text of the edit box
	//TextLen is the number of bytes in the box
	//start pos is the first character of the first visible line
	//endpos is the last character visible in view
	//hColorBytes is the HLOCAL to the color memory buffer
	//ColBufferSize is the alocated size of the color buffer, which may be less that the text size if mem alloc faillure happened

	//The function is called every draw and will colorise the text (via color buffer)
	//it will process to the end of the region that needs colorising which includes the start of the last change up to the last character on screen
	//if will exit if it is taking too long and continue on next draw, tried doing it in a thread but there was issues with the edit box 
	//text memory access which could not be resolved.

	//if nothing to do
	if (m_ChangedStartPos > m_ChangedEndPos)
	{
		return;
	}
	//change end pos to eof because a change may affect everything to the end
	m_ChangedEndPos = TextLen - 1;

	m_EBText = (TCHAR*)LocalLock(hEBText);

	if (!m_EBText) return;

	m_CharColors = (BYTE*)LocalLock(hColorBytes);
	if (!m_CharColors) return;
	{
		LocalUnlock(hEBText);
	}

	//Start reading from the start of the change
	m_ReadAt = max(0, m_ChangedStartPos);// min(m_ChangedStartPos, max(0, startpos));
	//rewinded to the start of a line that is safe to start processing from...
	Rewind();
	//the end of the processing, is the first character beyound view range or eof or the size of the color buffer which may have failed to allocate therefore is smaller than the text size
	m_DataLen = min(ColBufferSize, min(endpos, TextLen));

	m_Language = m_Ctrl->GetLanguage();

	//set the timer, which indicates when to stop processing if we are taking too much time,
	//to half the caret blink time
	DWORD CarretBlinkTime = min(250,m_Ctrl->m_OEBS.BlinkTime/2);
	DWORD timeStop = GetTickCount() + CarretBlinkTime;

	//process to the last vosible character
	while (m_ReadAt < m_DataLen)
	{
		bool processed = false;
		TCHAR c = DATA_CHAR(m_EBText, m_ReadAt);
		if (!processed && c == '\r')
		{
			//new line
			processed = true;
			m_ReadAt += 2;
			//inc the StartPos every new line, a new line is a safe spot to continue
			m_ChangedStartPos = m_ReadAt;
			//If we took too long, exit, we will continue on the next draw, which happens every cursor blink at worst
			if (GetTickCount()>timeStop) return;
		}
		if (!processed && m_Language->COMMENTCHARS.Valid(c))
		{
			//maybe comment
			if (m_Language->MULTILINECOMMENTSTART.Valid(DATA_PTR(m_EBText, m_ReadAt)))
			{
				processed = DoMultiLineComment();
			}
			else if (m_Language->SINGLELINECOMMENT.Valid(DATA_PTR(m_EBText, m_ReadAt)))
			{
				processed = DoSingleLineComment();
				//inc the StartPos every new line
				m_ChangedStartPos = m_ReadAt;
				//If we took too long, exit, we will continue on the next draw, which happens every cursor blink at worst
				if (GetTickCount()>timeStop) return;
			}
		}
		if (!processed && m_Language->QUOTES.Valid(c))
		{
			//quote
			processed = DoQuotedText();
		}


		if (!processed && m_Language->WHITESPACE.Valid(c))
		{
			//whitespace
			processed = DoWhitespace();
		}
		if (!processed && m_Language->BINPRECHECK.Valid(c))
		{
			//binary number?
			processed = DoBinary();
		}
		if (!processed && m_Language->OCTPRECHECK.Valid(c))
		{
			//octal number?
			processed = DoOctal();
		}
		if (!processed && m_Language->HEXPRECHECK.Valid(c))
		{
			//hex number
			processed = DoHex();
		}
		if (!processed && m_Language->NUMPRECHECK.Valid(c))
		{
			//number
			processed = DoNumber();
		}
		if (!processed && m_Language->VALIDNAMESTART.Valid(c))
		{
			//valid var or func name start
			CompoundString();
			processed = true;
		}

		if (!processed && m_Language->COLORSET1CHARS.Valid(c))
		{
			//Byte of color set 1
			DATA_SET(m_CharColors, m_ReadAt, COLORDEREF::COLORSET1);
			processed = true;
			m_ReadAt++;
		}
		if (!processed && m_Language->COLORSET2CHARS.Valid(c))
		{
			//Byte of color set 2
			DATA_SET(m_CharColors, m_ReadAt, COLORDEREF::COLORSET2);
			processed = true;
			m_ReadAt++;
		}
		if (!processed && m_Language->COLORSET3CHARS.Valid(c))
		{
			//Byte of color set 3
			DATA_SET(m_CharColors, m_ReadAt, COLORDEREF::COLORSET3);
			processed = true;
			m_ReadAt++;
		}
		if (!processed && m_Language->COLORSET4CHARS.Valid(c))
		{
			//Byte of color set 4
			DATA_SET(m_CharColors, m_ReadAt, COLORDEREF::COLORSET4);
			processed = true;
			m_ReadAt++;
		}
		//last one bad char 
		if (!processed)
		{
			//DATA_SET(m_EBTextMirror, m_ReadAt, c);
			DATA_SET(m_CharColors, m_ReadAt, COLORDEREF::BADCHARSCOLOR);
			m_ReadAt++;
		}

		//finally the ).Function() or ].Property handling, another hack, to hell with the consequences for other languages
		if (m_ReadAt > 0) //because read at-1 used
		{
			//since the single char handler likely ate the ] or ) and it's likely right next to the current pos, unless someone made an odd language file...
			//get the last eaten character
			bool done = false;
			while (!done)
			{
				TCHAR c = DATA_CHAR(m_EBText, m_ReadAt - 1);
				// is ) or ]
				if ((c == ')' || c == ']' || c == '\'' || c == '\"'))
				{
					//are we on a . or ->?
					size_t WordSize = 0;
					if (m_Language->BINDERSPRECHECK.Valid(DATA_CHAR(m_EBText, m_ReadAt)))
					{
						//are you sure?
						WordSize = m_Language->WORDBINDERS.ValidTCHAR((m_EBText + m_ReadAt));
						if (WordSize)
						{
							//OK and is the char after that a valid word start?
							if (m_Language->VALIDNAMESTART.Valid(DATA_CHAR(m_EBText, m_ReadAt + 1 + WordSize)))
							{
								//eat the word binder
								repeat(WordSize) {
									BYTE col = COLORDEREF::WORDBINDERCOLOR;
									//DATA_SET(m_EBTextMirror, m_ReadAt, DATA_CHAR(m_EBText, m_ReadAt));
									DATA_SET(m_CharColors, m_ReadAt, col);
									m_ReadAt++;
								}
								CompoundString(true);
								processed = true;
							}
							else
							{
								done = true;
							}
						}
						else
						{
							done = true;
						}
					}
					else
					{
						done = true;
					}

				}
				else
				{
					done = true;
				}
			}
		}
	}

	//did the entire file
	if (m_ReadAt >= TextLen)
	{
		m_ChangedStartPos = m_ReadAt;
	}
	//cleanup the end of the buffer
	//memset(m_CharColors + TextLen, 0, sizeof(BYTE) * max(0,(ColBufferSize -TextLen)));
	LocalUnlock(hEBText);
	LocalUnlock(hColorBytes);

	
};

void Parser::CompoundString(bool ClassFuncsOnly)
{
	//For the record I'm very exited this works but not especially proud of the hackcoding
	bool done = false;
	int DidJoin = 0;
	bool IsFirstWord = true;
	bool IsStaticClass = false;
	bool IsClassFunc = false;
	bool ThereIsADot = false;
	size_t wordcount = 0;
	wstring fullname; //for the full
	size_t fullnamestart = m_ReadAt;
	wstring ThisName;
	//for this word and any subsequent . word like Something.Something.something
	do {
		size_t thisnamestart = m_ReadAt;
		//get the word
		wstring ThisName = DoValidName();
		//Just read [something]

		//check if joiner something[->]
		size_t WordSize = 0;
		if (m_Language->BINDERSPRECHECK.Valid(DATA_CHAR(m_EBText, m_ReadAt)))
		{

			WordSize = m_Language->WORDBINDERS.ValidTCHAR((m_EBText + m_ReadAt));
		}
		if (WordSize)
		{

			DidJoin++;
			//if Something[->] the left of the joiner must be a static class or a loose class
			fullname += ThisName;
			//static class is accumulated full word Something->Something->Something...
			wstring ClassName = fullname;


			fullname += L".";
			//Loose class is Either ClassName or .PossibleMember for the search
			wstring ClassFuncname;
			if (wordcount || ClassFuncsOnly) ClassFuncname += L"."; //set the prefix for class functions so search will do ".FuncName" search in ref table
			ClassFuncname += ThisName;
			//handle the [->] type joiner, flag and set color
			TCHAR s[] = { 0,0 };
			repeat(WordSize) {
				BYTE col = COLORDEREF::WORDBINDERCOLOR;
				//DATA_SET(m_EBTextMirror, m_ReadAt, DATA_CHAR(m_EBText, m_ReadAt));
				DATA_SET(m_CharColors, m_ReadAt, col);
				m_ReadAt++;
			}

			//color the table positions
			if ((IsFirstWord == true || IsStaticClass == true) && ClassFuncsOnly == false && m_Language->STATICCLASS.Valid(ClassName))
			{
				//if(Something.Something.Something. exactly)
				IsStaticClass = true;
				ColorizeWord(ThisName, thisnamestart, COLORDEREF::STATICCLASSCOLOR);
			}
			else if ((IsFirstWord == true || IsClassFunc == true || ClassFuncsOnly == true) && m_Language->CLASSFUNCS.Valid(ClassFuncname))
			{
				//If CallsName or .MemberName
				IsClassFunc = true;
				ColorizeWord(ThisName, thisnamestart, COLORDEREF::CLASSFUNCCOLOR);
			}
			else
			{
				//here we hit imvalid class name or member
				ColorizeWord(ThisName, thisnamestart, COLORDEREF::UNKNOWNWORDCOLOR);
			}

			//check if next is another valid word start
			//if something->[A]notherWord
			if (!m_Language->VALIDNAMESTART.Valid(DATA_CHAR(m_EBText, m_ReadAt)))
			{
				done = true; //nope stop
			}

		}
		else if (DidJoin)
		{

			//Done joining, this is the last word in the constructed this.that.shomething sequence
			//and can only be a class member word
			//if Something->SomeLastThing the right of the joiner must be a static class function or a loose clase function
			fullname += ThisName;
			//Static clase uses the full accumulated "this.that.shomething" string
			wstring ClassName = fullname;
			//color the table positions
			//Loose clase uses ".MemberName"
			wstring ClassFuncname;
			ClassFuncname += L".";
			ClassFuncname += ThisName;

			if (m_Language->STATICCLASS.Valid(ClassName))
			{
				ColorizeWord(ThisName, thisnamestart, COLORDEREF::STATICCLASSCOLOR);
			}
			else if (m_Language->CLASSFUNCS.Valid(ClassFuncname))
			{
				ColorizeWord(ThisName, thisnamestart, COLORDEREF::CLASSFUNCCOLOR);
			}
			else
			{
				ColorizeWord(ThisName, thisnamestart, COLORDEREF::UNKNOWNWORDCOLOR);
			}
			done = true;
		}
		else
		{
			//OK, we did not consruct a this.that.shomething string...
			//plain old [something]... single valid word, may be any of the words or any of the dictionaries
			BYTE col = COLORDEREF::UNKNOWNWORDCOLOR;
			wstring TheName;
			
			if (ClassFuncsOnly) TheName += L".";
			TheName += ThisName;

			if (ClassFuncsOnly == false && m_Language->STATICCLASS.Valid(TheName))
			{
				col = COLORDEREF::STATICCLASSCOLOR;
			}
			else if (m_Language->CLASSFUNCS.Valid(TheName))
			{
				col = COLORDEREF::CLASSFUNCCOLOR;
			}
			else if (ClassFuncsOnly == false && m_Language->FUNCTIONS.Valid(TheName))
			{
				col = COLORDEREF::FUNCTIONCOLOR;
			}
			else if (ClassFuncsOnly == false && m_Language->KEYWORDS.Valid(TheName))
			{
				col = COLORDEREF::KEYWORDCOLOR;
			}
			else if (ClassFuncsOnly == false && m_Language->LITERALS.Valid(TheName))
			{
				col = COLORDEREF::LITERALCOLOR;
			}
			else if (ClassFuncsOnly == false && DATA_CHAR(m_EBText,m_ReadAt) == '(')
			{
				//not found in dictionaries but ( at the end, assume user function
				col = COLORDEREF::USERFUNCTIONCOLOR;
			}
			ColorizeWord(ThisName, thisnamestart, col);
			done = true;
		}

		//if we read up and found another
		IsFirstWord = false;
		wordcount++;
	} while (!done);
	
}

