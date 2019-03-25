#pragma once
#define DATA_PTR(ptr,offset) (TCHAR*)(ptr + offset)
#define DATA_CHAR(ptr,offset) (TCHAR)*(ptr + offset)
#define DATA_SET(ptr,offset,value) *(ptr + offset) = value
#define COLOR_GET(offset) (m_CharColors[offset])

typedef std::list<wstring> CODLINES;
typedef std::list<int> CODLINESTYPES;

enum class SECTIONTYPE {
	NOT_SET,
	STATEMENT,
	BLOCK,
	BLOCK_START,
	BLOCK_END,
	NEW_LINE,
	IN_PAREN_BLOCK,
	IN_BRACE_OR_EOL_BLOCK,
	STUB_BLOCK_START,
	EOS,
	//to review
	SECTION_START,
	SECTION_END,

	CODE_LINE,
	QUOTED_CONTENT,
	COMMENT_START,
	COMMENT_END,
	COMMENT_CONTENT,
	COMMENT_NEW_LINE,
	SINGLE_INDENT,
	_last
};

//container for indenting logic
class CodeSection
{
public:
	wstring m_CodeText;
	SECTIONTYPE m_SectionType = SECTIONTYPE::NOT_SET;
	CodeSection * m_Parent = NULL;
	CodeSection * m_FirstChild = NULL;
	CodeSection * m_LastChild = NULL;
	CodeSection * m_PreviousSibling = NULL;
	CodeSection * m_NextSibling = NULL;
	CodeSection(SECTIONTYPE SectionType = SECTIONTYPE::NOT_SET, CodeSection *Parent = NULL, CodeSection *PreviousSibling = NULL) {
		m_SectionType = SectionType;
		m_Parent = Parent;
		m_PreviousSibling = PreviousSibling;
	};
	~CodeSection() {
		if (m_FirstChild != NULL) delete m_FirstChild; //simply, this will delete the child, which will delete its child... and so on
		//if (m_NextSibling != NULL) delete m_NextSibling; //same concept with with next.
		if(m_PreviousSibling == NULL) //less recursion, if top of chain, delete siblings
		{
			CodeSection *n = m_NextSibling;
			while (n != NULL)
			{
				CodeSection *thisOne = n;
				n = thisOne->m_NextSibling;
				if (thisOne != NULL)
				{
					delete thisOne;
				}
			}
		}
	};

	CodeSection *NewSection(SECTIONTYPE SectionType = SECTIONTYPE::NOT_SET)
	{
		CodeSection *SubSection = new CodeSection(SectionType, this, m_LastChild);
		if (m_FirstChild == NULL)
		{
			m_FirstChild = SubSection;
		}
		if (m_LastChild != NULL) m_LastChild->m_NextSibling = SubSection;
		m_LastChild = SubSection;
		return SubSection;
	}
};
class Parser
{
private:
	TCHAR *m_EBText = NULL;
	//TCHAR *m_EBTextMirror = NULL;
	//COLORREF *m_CharColors = NULL;
	BYTE *m_CharColors = NULL;
	HookedCtrl *m_Ctrl = NULL;
	Language *m_Language = NULL;
	//ColorSpec *m_ColorSpec = NULL;
	size_t m_DataLen = 65535;
	size_t m_ReadAt = 0;
	wstring m_IndentedCode;
	wstring m_IndentedLine;
	wstring m_TempBuff;
	size_t m_ChangedStartPos = 0;
	size_t m_ChangedEndPos = 0;

	//CODLINES m_IndentLines;
	//CODLINESTYPES m_IndentLinesTypes;
	int m_IndentLevel = 0;
	wstring m_IndentTabs = L"";
	bool m_IndentDidNewLine = false;
public:
	Parser();
	~Parser();
	void IndentKR();
	void IndentAllman();
	void IndentGNU();
	void IndentWhitesmiths();
	void IndentHorstmann();

	void RecurseIndentReplaceType(CodeSection *Current, SECTIONTYPE search, SECTIONTYPE replace);
	void RecurseIndentCode(CodeSection *Current);
	CodeSection * RecurseIndent(CodeSection *Current );
	wstring& DoIndent(HLOCAL hEBText, size_t TextLen, int Style);

	void SetChangeRegion(size_t StarPos, size_t EndPos)
	{
		//remember the range when changes were made, 
		//keep the smaller of the value, including stored one
		m_ChangedStartPos = min(m_ChangedStartPos,min(StarPos, EndPos));
		//keep the larger of the value, including stored one
		m_ChangedEndPos = max(m_ChangedEndPos,max(StarPos, EndPos));
	}
	void SetHookedCtrl(HookedCtrl *Ctrl)
	{
		m_Ctrl = Ctrl;
	}
	void DoColorize(HLOCAL hEBText, size_t TextLen, size_t startpos, size_t endpos, HLOCAL hColorBytes, size_t ColBufferSize);

	void RewindQuote(char qt)
	{
		while (m_ReadAt > 0) {
			if (qt == DATA_CHAR(m_EBText, m_ReadAt) && !m_Language->STRINGESCAPE.Valid(DATA_CHAR(m_EBText, max(0, m_ReadAt - 1))))
			{
				break;
			}
			m_ReadAt--;
		}
	}
	void GoToBOL()
	{
		//Go to start of line
		while (m_ReadAt > 0)
		{
			if (DATA_CHAR(m_EBText, m_ReadAt - 1) == '\n')
			{
				break;
			}
			m_ReadAt--;
		}
	}
	void PrevLine()
	{
		GoToBOL();
		if (DATA_CHAR(m_EBText, max(0,m_ReadAt - 1)) == '\n')
		{
			m_ReadAt -= 2;
		}
		m_ReadAt = max(0, m_ReadAt);
		GoToBOL();
	}
	void Rewind()
	{
		GoToBOL();
		BYTE b = COLOR_GET(m_ReadAt);
		//Find the start of a line that is not a comment or a quote
		while (m_ReadAt > 0 && (b == COLORDEREF::DQUOTECOLOR || b == COLORDEREF::SQUOTECOLOR || b == COLORDEREF::MCOMMENTCOLOR))
		{
			PrevLine();
			b = COLOR_GET(m_ReadAt);
		}

		/*
		BYTE b = COLOR_GET(m_ReadAt);
		//Find the start of a line that is processed
		while (m_ReadAt > 0 && (b == COLORDEREF::NOTSET))
		{
			PrevLine();
			b = COLOR_GET(m_ReadAt);
		}

		b = COLOR_GET(m_ReadAt);
		//Find the start of a line that is not a comment or a quote
		while (m_ReadAt > 0 && (b == COLORDEREF::DQUOTECOLOR || b == COLORDEREF::SQUOTECOLOR || b == COLORDEREF::MCOMMENTCOLOR))
		{
			PrevLine();
			b = COLOR_GET(m_ReadAt);
		}
		*/
	}
	
	void CompoundString(bool ClassFuncsOnly = false);
	
	bool DoBinary()
	{
		size_t startpos = m_ReadAt;
		//are you sure?
		size_t WordSize = m_Language->BINPREFIXES.ValidTCHAR((m_EBText + m_ReadAt));
		if (WordSize)
		{
			BYTE col = COLORDEREF::NUMBERCOLOR;
			//flag prefix
			repeat(WordSize)
			{
				//DATA_SET(m_EBTextMirror, m_ReadAt, DATA_CHAR(m_EBText, m_ReadAt));
				DATA_SET(m_CharColors, m_ReadAt, col);
				m_ReadAt++;
			}
			//loop to the end of the number
			while (m_ReadAt < m_DataLen)
			{
				TCHAR c = DATA_CHAR(m_EBText, m_ReadAt);
				if (m_Language->BASEBIN.Valid(c))
				{
					//DATA_SET(m_EBTextMirror, m_ReadAt, c);
					DATA_SET(m_CharColors, m_ReadAt, col);
					m_ReadAt++;
				}
				else
				{
					if (m_Language->SPLITTERS.Valid(c))
					{
						return true;
					}
					else
					{
						m_ReadAt = startpos;
						return false;
					}

				}
			}
			return true;

		}
		return false;

	}
	bool DoOctal()
	{
		size_t startpos = m_ReadAt;
		//are you sure?
		size_t WordSize = m_Language->OCTPREFIXES.ValidTCHAR((m_EBText + m_ReadAt));
		if (WordSize)
		{
			BYTE col = COLORDEREF::NUMBERCOLOR;
			//flag prefix
			repeat(WordSize)
			{
				//DATA_SET(m_EBTextMirror, m_ReadAt, DATA_CHAR(m_EBText, m_ReadAt));
				DATA_SET(m_CharColors, m_ReadAt, col);
				m_ReadAt++;
			}
			//loop to the end of the number
			while (m_ReadAt < m_DataLen)
			{
				TCHAR c = DATA_CHAR(m_EBText, m_ReadAt);
				if (m_Language->BASEOCT.Valid(c))
				{
					//DATA_SET(m_EBTextMirror, m_ReadAt, c);
					DATA_SET(m_CharColors, m_ReadAt, col);
					m_ReadAt++;
				}
				else
				{
					if (m_Language->SPLITTERS.Valid(c))
					{
						return true;
					}
					else
					{
						m_ReadAt = startpos;
						return false;
					}

				}
			}
			return true;

		}
		return false;

	}
	bool DoHex()
	{
		size_t startpos = m_ReadAt;
		//are you sure?
		size_t WordSize = m_Language->HEXPREFIXES.ValidTCHAR((m_EBText + m_ReadAt));
		if (WordSize)
		{
			BYTE col = COLORDEREF::NUMBERCOLOR;
			//flag prefix
			repeat(WordSize)
			{
				//DATA_SET(m_EBTextMirror, m_ReadAt, DATA_CHAR(m_EBText, m_ReadAt));
				DATA_SET(m_CharColors, m_ReadAt, col);
				m_ReadAt++;
			}
			//loop to the end of the number
			while (m_ReadAt < m_DataLen)
			{
				TCHAR c = DATA_CHAR(m_EBText, m_ReadAt);
				if (m_Language->BASEHEX.Valid(c))
				{
					//DATA_SET(m_EBTextMirror, m_ReadAt, c);
					DATA_SET(m_CharColors, m_ReadAt, col);
					m_ReadAt++;
				}
				else
				{
					if (m_Language->SPLITTERS.Valid(c))
					{
						return true;
					}
					else
					{
						m_ReadAt = startpos;
						return false;
					}

				}
			}
			return true;

		}
		return false;

	}
	bool DoNumber()
	{
		bool did_dot = false;
		bool did_e = false;
		bool did_a_num = false;
		BYTE col = COLORDEREF::NUMBERCOLOR;
		size_t startpos = m_ReadAt;
		while (m_ReadAt < m_DataLen)
		{
			TCHAR c = DATA_CHAR(m_EBText, m_ReadAt);
			if (m_Language->BASEDEC.Valid(c))
			{
				//DATA_SET(m_EBTextMirror, m_ReadAt, c);
				DATA_SET(m_CharColors, m_ReadAt, col);
				did_a_num = true;
				m_ReadAt++;
			}
			else if (c == '.' && did_dot == false && did_e == false)
			{

				//DATA_SET(m_EBTextMirror, m_ReadAt, c);
				DATA_SET(m_CharColors, m_ReadAt, col);
				did_dot = true;
				did_a_num = true;
				m_ReadAt++;

			}
			else if (c == 'e' && did_a_num == true && did_e == false)
			{
				//DATA_SET(m_EBTextMirror, m_ReadAt, c);
				DATA_SET(m_CharColors, m_ReadAt, col);
				m_ReadAt++;
				if (DATA_CHAR(m_EBText, m_ReadAt) == '-' || DATA_CHAR(m_EBText, m_ReadAt) == '+')
				{
					//DATA_SET(m_EBTextMirror, m_ReadAt, c);
					DATA_SET(m_CharColors, m_ReadAt, col);
					m_ReadAt++;
				}
				did_dot = true;
				did_e = true;
			}
			else if (m_Language->SPLITTERS.Valid(c))
			{
				return true;
			}
			else
			{
				size_t WordSize = m_Language->NUMSUFFIXES.ValidTCHAR((m_EBText + m_ReadAt));
				if (WordSize>0)
				{
					if (m_Language->SPLITTERS.Valid(DATA_CHAR(m_EBText,m_ReadAt + WordSize)))
					{
						//flag prefix
						repeat(WordSize)
						{
							//DATA_SET(m_EBTextMirror, m_ReadAt, DATA_CHAR(m_EBText, m_ReadAt));
							DATA_SET(m_CharColors, m_ReadAt, col);
							m_ReadAt++;
						}
						return true;
					}
				}

				m_ReadAt = startpos;
				return false;
			}

		}
		return true;
	}
	
	bool DoMultiLineComment()
	{
		//Multiline comment start. mark from position to the end of the file as commented out
		size_t start = m_ReadAt;
		

		//read passed the start of multiline comment
		m_ReadAt += m_Language->MULTILINECOMMENTSTART.GetLength();

		//read until we find the end of multiline comment
		while (m_ReadAt < m_DataLen)
		{
			TCHAR c = DATA_CHAR(m_EBText, m_ReadAt);
			//Precheck with the fast system if the char is the comment char
			if (m_Language->COMMENTCHARS.Valid(c))
			{
				//and confirm it's the end comment
				if (m_Language->MULTILINECOMMENTEND.Valid(DATA_PTR(m_EBText, m_ReadAt)))
				{
					//consume the characters and done
					m_ReadAt += m_Language->MULTILINECOMMENTEND.GetLength();
					break;
				}
			}
			//keep reading
			m_ReadAt++;
		}
		//mark the region as processed
		memset(m_CharColors + start, COLORDEREF::MCOMMENTCOLOR, sizeof(BYTE) * (m_ReadAt - start));
		
		
		return true;
	}
	
	
	bool DoSingleLineComment()
	{
		//Remember where the single line comment started at
		size_t start = m_ReadAt;
		//read passed the start of single line comment
		m_ReadAt += m_Language->SINGLELINECOMMENT.GetLength();
		
		while (m_ReadAt < m_DataLen)
		{
			TCHAR c = DATA_CHAR(m_EBText, m_ReadAt);
			//carriage return?
			if (c == '\r')
			{
				//consume \r\n and done
				m_ReadAt += 2;
				break;
			}
			//keep reading
			m_ReadAt++;
		}
		//done the line
		//now mark them bytes as single line coments
		memset(m_CharColors + start, COLORDEREF::SCOMMENTCOLOR, sizeof(BYTE) * (m_ReadAt- start));
		return true;
	}
	
	bool DoQuotedText()
	{
		//Remember where the quote
		size_t start = m_ReadAt;
		//reads what is after " or '
		BYTE col = COLORDEREF::SQUOTECOLOR;
		//Was is " or '?
		TCHAR StartQuote = DATA_CHAR(m_EBText, m_ReadAt);
		if(StartQuote == '"') col = COLORDEREF::DQUOTECOLOR;
		//Quoted text can be multiline. mark from position to the end of the file as quoted text
		//read passed the quote
		{
			m_ReadAt++;
		}
		//read until end of quote
		while (m_ReadAt < m_DataLen)
		{
			TCHAR c = DATA_CHAR(m_EBText, m_ReadAt);
			//hit end quote? and not escape quote
			if (c == StartQuote && !m_Language->STRINGESCAPE.Valid(DATA_CHAR(m_EBText, m_ReadAt - 1)))
			{
				//done, consume this char and leave
				m_ReadAt++;
				break;
			}
			//not done continue
			m_ReadAt++;
		}
		//now mark them bytes as quoted text 
		memset(m_CharColors + start, col, sizeof(BYTE) * (m_ReadAt - start));
		return true;
	}
	

	void ColorizeWord(wstring word, size_t wordStart, BYTE col)
	{
		size_t wordend = wordStart + word.length();
		for (size_t i = wordStart; i < wordend; i++)
		{
			DATA_SET(m_CharColors, i, col);
		}
	};
	void SetWordColor(size_t wordStart, wstring word)
	{

		size_t wordend = wordStart + word.length();
		//match word to one of the dictionary to set the color of the word
		BYTE col = COLORDEREF::UNKNOWNWORDCOLOR;
		if (0)
		{

		}
		else if (m_Language->STATICCLASS.Valid(word))
		{
			col = COLORDEREF::STATICCLASSCOLOR;
		}
		else if (m_Language->FUNCTIONS.Valid(word))
		{
			col = COLORDEREF::FUNCTIONCOLOR;
		}
		else if (m_Language->KEYWORDS.Valid(word))
		{
			col = COLORDEREF::KEYWORDCOLOR;
		}
		else if (m_Language->LITERALS.Valid(word))
		{
			col = COLORDEREF::LITERALCOLOR;
		}
		else if (m_Language->STATICCLASS.Valid(word))
		{
			col = COLORDEREF::STATICCLASSCOLOR;
		}
		else if (m_Language->CLASSFUNCS.Valid(word))
		{
			col = COLORDEREF::CLASSFUNCCOLOR;
		}

		for (size_t i = wordStart; i < wordend; i++)
		{
			DATA_SET(m_CharColors, i, col);
		}
	};
	wstring DoValidName()
	{
		//reads what is after a proper function start
		wstring word;
		
		TCHAR s[] = { 0,0 };
		
		//flag the character as read accumulate word
		{
			TCHAR c = DATA_CHAR(m_EBText, m_ReadAt);
			s[0] = c;
			word += s;
			//DATA_SET(m_EBTextMirror, m_ReadAt, c);
			
			m_ReadAt++;
		}
		//read until now a function name character
		bool IsStillIt = true;
		while (m_ReadAt < m_DataLen)
		{
			//flag the character as read accumulate word
			TCHAR c = DATA_CHAR(m_EBText, m_ReadAt);
			
			//flag the characters as read and mark
			if (!m_Language->VALIDNAMEEND.Valid(c))
			{
				IsStillIt = false;
			}
			//Done?
			if (IsStillIt)
			{
				//mark as read and set color map
				s[0] = c;
				word += s;
				
				//DATA_SET(m_EBTextMirror, m_ReadAt, c);
				
			}
			else
			{
				break;
			}
			
			m_ReadAt++;
		}
		
		return word;
	}
	bool DoWhitespace()
	{
		BYTE col = COLORDEREF::TEXTBACKCOLOR;
		//reads all whitespace 'till no more
		//flag the first whitespace
		{
			//DATA_SET(m_EBTextMirror, m_ReadAt, DATA_CHAR(m_EBText, m_ReadAt));
			DATA_SET(m_CharColors, m_ReadAt, col);
			m_ReadAt++;
		}
		//read until end of whitespaces
		while (m_ReadAt < m_DataLen)
		{
			TCHAR c = DATA_CHAR(m_EBText, m_ReadAt);
			//is it still a white space?
			if (!m_Language->WHITESPACE.Valid(c))
			{
				//done
				return true;
			}
			//flag whitespace and continue
			DATA_SET(m_CharColors, m_ReadAt, col);
			m_ReadAt++;
		}
		return true;
	}
	bool DoIndentLeadingWhitespace(int maxremove = 10000000)
	{
		//this is always called on the start of a line
		//read while the character are whitespace
		int ct = 0;
		while (m_ReadAt < m_DataLen && ct<maxremove)
		{
			TCHAR c = DATA_CHAR(m_EBText, m_ReadAt);
			//is whitespace
			if (!m_Language->WHITESPACE.Valid(c))
			{
				return true;
			}
			//keep reading
			m_ReadAt++;
			ct++;
		}
		return true;
	}
	int CountMLCLeadingTabs()
	{
		int count = 0;
		size_t at = m_ReadAt;
		while (at < m_DataLen)
		{
		}
		return count;
	}
	wstring DoIndentMultiLineComment(bool firstcall, bool *done)
	{
		m_TempBuff = L"";
		*done = false;
		//read passed the start of multiline comment
		if (firstcall)
		{
			repeat(m_Language->MULTILINECOMMENTSTART.GetLength())
			{
				m_TempBuff += DATA_CHAR(m_EBText, m_ReadAt);
				m_ReadAt++;
			}

		}
		
		//read until we find the end of multiline comment
		while (m_ReadAt < m_DataLen)
		{
			TCHAR c = DATA_CHAR(m_EBText, m_ReadAt);
			//Precheck with the fast system if the char is the comment char
			if (c == '\r')
			{
				//consume \r\n
				m_ReadAt += 2;
				break;
			}
			else if (m_Language->COMMENTCHARS.Valid(c))
			{
				//and confirm it's the end comment
				if (m_Language->MULTILINECOMMENTEND.Valid(DATA_PTR(m_EBText, m_ReadAt)))
				{
					
					//consume the characters and done
					repeat(m_Language->MULTILINECOMMENTEND.GetLength())
					{
						m_TempBuff += DATA_CHAR(m_EBText, m_ReadAt);
						m_ReadAt++;
					}
					*done = true;
					break;
				}
			}
			//keep reading
			m_TempBuff += c;
			m_ReadAt++;
		}
		if(m_ReadAt >= m_DataLen)
		{
			*done = true;
		}
		//mark the region as processed
		return m_TempBuff;
	}


	wstring & DoIndentSingleLineComment()
	{
		m_TempBuff = L"";
		//read passed the start of single line comment //
		repeat(m_Language->SINGLELINECOMMENT.GetLength())
		{
			m_TempBuff += DATA_CHAR(m_EBText, m_ReadAt);
			m_ReadAt++;
		}

		while (m_ReadAt < m_DataLen)
		{
			TCHAR c = DATA_CHAR(m_EBText, m_ReadAt);
			//carriage return?
			if (c == '\r')
			{
				//consume \r\n
				m_ReadAt += 2;
				break;
			}
			else if (c == '\n')
			{
				//consume \r\n
				m_ReadAt += 1;
				break;
			}
			//keep reading
			m_TempBuff += c;
			m_ReadAt++;
		}
		//done the line
		return m_TempBuff;
	}


	wstring& DoIndentQuotedText()
	{
		m_TempBuff = L"";
		//reads what is after " or '
		//Was is " or '?
		TCHAR StartQuote = DATA_CHAR(m_EBText, m_ReadAt);
		//flag the start quote
		{
			m_TempBuff += DATA_CHAR(m_EBText, m_ReadAt);
			m_ReadAt++;

		}
		//read until end of quote
		while (m_ReadAt < m_DataLen)
		{
			TCHAR c = DATA_CHAR(m_EBText, m_ReadAt);
			//hit end quote? and not escape quote
			if (c == StartQuote && !m_Language->STRINGESCAPE.Valid(DATA_CHAR(m_EBText, m_ReadAt - 1)))
			{
				//done, leave
				m_TempBuff += c;
				m_ReadAt++;
				return m_TempBuff;
			}
			//not done, continue
			m_TempBuff += c;
			m_ReadAt++;

		}
		//eof, done
		return m_TempBuff;

	}
	wstring & IndentRightTrim(wstring s)
	{
		m_TempBuff = L""; 
		const TCHAR * t = s.c_str();
		size_t len = s.length();
		if (len)
		{
			while (len > 0 && m_Language->WHITESPACE.Valid(t[len - 1]))
			{
				len--;
			}
			m_TempBuff = s.substr(0, len);
		}
		return m_TempBuff;
	}
	wstring & IndentLeftTrim(wstring s)
	{
		m_TempBuff = L""; 
		const TCHAR * t = s.c_str();
		size_t len = s.length();
		if (len)
		{
			size_t pos = 0;
			while (pos < len && m_Language->WHITESPACE.Valid(t[pos]))
			{
				pos++;
			}
			m_TempBuff = s.substr(pos, len-pos);
		}
		return m_TempBuff;
	}
	
	TCHAR RightTrimIndentLine()
	{
		const TCHAR * t = m_IndentedLine.c_str();
		size_t len = m_IndentedLine.length();
		if (len)
		{
			while (len > 0 && m_Language->WHITESPACE.Valid(t[len-1]))
			{
				len--;
			}
			m_IndentedLine = m_IndentedLine.substr(0, len);
			return m_IndentedLine.c_str()[max(0, len - 1)];
		}
		else
		{
			return 0;
		}
	}
	bool IsPossibleInline(wstring &s)
	{
		return StartsWith(s, L"if") ||
			StartsWith(s, L"else") ||
			StartsWith(s, L"for") ||
			StartsWith(s, L"while") ||
			StartsWith(s, L"do") ||
			StartsWith(s, L"case") ||
			StartsWith(s, L"default") ||
			StartsWith(s, L"try") ||
			StartsWith(s, L"catch");

	}
};

