#pragma once
#define DATA_PTR(ptr,offset) (TCHAR*)(ptr + offset)
#define DATA_CHAR(ptr,offset) (TCHAR)*(ptr + offset)
#define DATA_SET(ptr,offset,value) *(ptr + offset) = value
#define COLOR_GET(offset) (m_CharColors[offset])


class Parser
{
private:
	TCHAR *m_EBText = NULL;
	TCHAR *m_EBTextMirror = NULL;
	COLORREF *m_CharColors = NULL;
	HookedCtrl *m_Ctrl = NULL;
	Language *m_Language = NULL;
	size_t m_DataLen = 65535;
	size_t m_ReadAt = 0;
public:
	Parser();
	~Parser();
	void LinkToEbContent(TCHAR *EBText, TCHAR *EBTextMirror, COLORREF *CharColors, HookedCtrl *Ctrl) {
		m_EBText = EBText;
		m_EBTextMirror = EBTextMirror;
		m_CharColors = CharColors;
		m_Ctrl = Ctrl;
		
	};
	void Parse(size_t TextLen);
	void RewindToBOL()
	{
		m_ReadAt = max(0, m_ReadAt - 1);
		COLORREF startcol = COLOR_GET(m_ReadAt);
		COLORREF col;
		while (m_ReadAt > 0)
		{
			col = COLOR_GET(m_ReadAt);
			if (col != startcol) // || m_Language->COMMENTCHARS.Valid(c) || m_Language->QUOTES.Valid(c))
			{
				//it's on \n move one to start of line
				m_ReadAt++;
				return;
			}

			m_ReadAt--;
		}
		/*
		//rewind to start of line
		m_ReadAt = max(0, m_ReadAt - 1);
		TCHAR c;
		while (m_ReadAt > 0)
		{
			c = DATA_CHAR(m_EBText, m_ReadAt);
			if (c == '\n') // || m_Language->COMMENTCHARS.Valid(c) || m_Language->QUOTES.Valid(c))
			{
				//it's on \n move one to start of line
				m_ReadAt++;
				return;
			}
				
			m_ReadAt--;
		}
		*/
	}
	void CompoundString(bool ClassFuncsOnly = false);
	void SeekFirstDifference()
	{
		return;
		//find out where the old data does not match the new one
		while (m_ReadAt< m_DataLen)
		{
			if (DATA_CHAR(m_EBText, m_ReadAt) != DATA_CHAR(m_EBTextMirror, m_ReadAt))
			{
				//ok, changes start here, but rewind to start of line to reprocess it
				
					RewindToBOL();
				
				return;
			}
			m_ReadAt++;
		} 
	}
	bool DoBinary()
	{
		size_t startpos = m_ReadAt;
		//are you sure?
		size_t WordSize = m_Language->BINPREFIXES.ValidTCHAR((m_EBText + m_ReadAt));
		if (WordSize)
		{
			COLORREF col = m_Language->NUMBERCOLOR;
			//flag prefix
			repeat(WordSize)
			{
				DATA_SET(m_EBTextMirror, m_ReadAt, DATA_CHAR(m_EBText, m_ReadAt));
				DATA_SET(m_CharColors, m_ReadAt, col);
				m_ReadAt++;
			}
			//loop to the end of the number
			while (m_ReadAt < m_DataLen)
			{
				TCHAR c = DATA_CHAR(m_EBText, m_ReadAt);
				if (m_Language->BASEBIN.Valid(c))
				{
					DATA_SET(m_EBTextMirror, m_ReadAt, c);
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
			COLORREF col = m_Language->NUMBERCOLOR;
			//flag prefix
			repeat(WordSize)
			{
				DATA_SET(m_EBTextMirror, m_ReadAt, DATA_CHAR(m_EBText, m_ReadAt));
				DATA_SET(m_CharColors, m_ReadAt, col);
				m_ReadAt++;
			}
			//loop to the end of the number
			while (m_ReadAt < m_DataLen)
			{
				TCHAR c = DATA_CHAR(m_EBText, m_ReadAt);
				if (m_Language->BASEOCT.Valid(c))
				{
					DATA_SET(m_EBTextMirror, m_ReadAt, c);
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
			COLORREF col = m_Language->NUMBERCOLOR;
			//flag prefix
			repeat(WordSize)
			{
				DATA_SET(m_EBTextMirror, m_ReadAt, DATA_CHAR(m_EBText, m_ReadAt));
				DATA_SET(m_CharColors, m_ReadAt, col);
				m_ReadAt++;
			}
			//loop to the end of the number
			while (m_ReadAt < m_DataLen)
			{
				TCHAR c = DATA_CHAR(m_EBText, m_ReadAt);
				if (m_Language->BASEHEX.Valid(c))
				{
					DATA_SET(m_EBTextMirror, m_ReadAt, c);
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
		COLORREF col = m_Language->NUMBERCOLOR;
		size_t startpos = m_ReadAt;
		while (m_ReadAt < m_DataLen)
		{
			TCHAR c = DATA_CHAR(m_EBText, m_ReadAt);
			if (m_Language->BASEDEC.Valid(c))
			{
				DATA_SET(m_EBTextMirror, m_ReadAt, c);
				DATA_SET(m_CharColors, m_ReadAt, col);
				did_a_num = true;
				m_ReadAt++;
			}
			else if (c == '.' && did_dot == false && did_e == false)
			{

				DATA_SET(m_EBTextMirror, m_ReadAt, c);
				DATA_SET(m_CharColors, m_ReadAt, col);
				did_dot = true;
				did_a_num = true;
				m_ReadAt++;

			}
			else if (c == 'e' && did_a_num == true && did_e == false)
			{
				DATA_SET(m_EBTextMirror, m_ReadAt, c);
				DATA_SET(m_CharColors, m_ReadAt, col);
				m_ReadAt++;
				if (DATA_CHAR(m_EBText, m_ReadAt) == '-' || DATA_CHAR(m_EBText, m_ReadAt) == '+')
				{
					DATA_SET(m_EBTextMirror, m_ReadAt, c);
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
							DATA_SET(m_EBTextMirror, m_ReadAt, DATA_CHAR(m_EBText, m_ReadAt));
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
	void DoMultiLineCommentEnd()
	{
		//read over the */ 
		COLORREF col = m_Language->COMMENTCOLOR;
		//flag the word characters as read and mark
		repeat(m_Language->MULTILINECOMMENTEND.GetLength())
		{
			//mark as read and set color map
			DATA_SET(m_EBTextMirror, m_ReadAt, DATA_CHAR(m_EBText, m_ReadAt));
			DATA_SET(m_CharColors, m_ReadAt, col);
			m_ReadAt++;
		}
	}
	bool DoMultiLineComment()
	{
		//reads what is after /*
		COLORREF col = m_Language->COMMENTCOLOR;
		//flag the word characters as read and mark
		repeat(m_Language->MULTILINECOMMENTSTART.GetLength())
		{
			//mark as read and set color map
			DATA_SET(m_EBTextMirror, m_ReadAt, DATA_CHAR(m_EBText, m_ReadAt));
			DATA_SET(m_CharColors, m_ReadAt, col);
			m_ReadAt++;
		}

		//read until */
		bool IsStillIt = true;
		while (m_ReadAt < m_DataLen)
		{
			TCHAR c = DATA_CHAR(m_EBText, m_ReadAt);
			//is / or * ?
			if (m_Language->COMMENTCHARS.Valid(c))
			{
				//is "*/"?
				if (m_Language->MULTILINECOMMENTEND.Valid(DATA_PTR(m_EBText, m_ReadAt)))
				{
					DoMultiLineCommentEnd();
					IsStillIt = false;
				}
			}
			//Done?
			if (IsStillIt)
			{
				//mark as read and set color map
				DATA_SET(m_EBTextMirror, m_ReadAt, c);
				DATA_SET(m_CharColors, m_ReadAt, col);
			}
			else
			{
				return true;
			}
			//keep reading
			m_ReadAt++;
		}
		return true;
	}
	
	bool DoQuotedText()
	{
		//reads what is after " or '
		COLORREF col = m_Language->QUOTECOLOR;
		//Was is " or '?
		TCHAR StartQuote = DATA_CHAR(m_EBText, m_ReadAt);
		//flag the quote
		{
			DATA_SET(m_EBTextMirror, m_ReadAt, DATA_CHAR(m_EBText, m_ReadAt));
			DATA_SET(m_CharColors, m_ReadAt, col);
			m_ReadAt++;
		}
		//read until end of quote
		bool IsStillIt = true;
		while (m_ReadAt < m_DataLen)
		{
			TCHAR c = DATA_CHAR(m_EBText, m_ReadAt);
			//is is string escape? skip over \"
			if (m_Language->STRINGESCAPE.Valid(c))
			{
				m_ReadAt++; //move to the quote or whatever was there, the next ++ will move beyond it
			}
			else if (c == StartQuote)
			{
				IsStillIt = false;
			}
			//Done?
			if (IsStillIt)
			{
				//mark as read and set color map
				DATA_SET(m_EBTextMirror, m_ReadAt, c);
				DATA_SET(m_CharColors, m_ReadAt, col);
			}
			else
			{
				//mark the last quote
				DATA_SET(m_EBTextMirror, m_ReadAt, c);
				DATA_SET(m_CharColors, m_ReadAt, col);
				m_ReadAt++;
				return true;
			}
			//keep reading
			m_ReadAt++;
		}
		return true;
	}
	bool DoSingleLineComment()
	{
		//reads what is after //
		COLORREF col = m_Language->COMMENTCOLOR;
		//flag the word characters as read and mark
		repeat(m_Language->SINGLELINECOMMENT.GetLength())
		{
			DATA_SET(m_EBTextMirror, m_ReadAt, DATA_CHAR(m_EBText, m_ReadAt));
			DATA_SET(m_CharColors, m_ReadAt, col);
			m_ReadAt++;
		}
		//read until \r
		bool IsStillIt = true;
		while (m_ReadAt < m_DataLen)
		{
			TCHAR c = DATA_CHAR(m_EBText, m_ReadAt);
			if (c == '\r')
			{
				IsStillIt = false;
			}
			//Done?
			if (IsStillIt)
			{
				//mark as read and set color map
				DATA_SET(m_EBTextMirror, m_ReadAt, c);
				DATA_SET(m_CharColors, m_ReadAt, col);
			}
			else
			{
				return true;
			}
			//keep reading
			m_ReadAt++;
		}
		return true;
	}
	void ColorizeWord( TCHARString word, size_t wordStart, COLORREF col)
	{
		size_t wordend = wordStart + word.length();
		for (size_t i = wordStart; i < wordend; i++)
		{
			DATA_SET(m_CharColors, i, col);
		}
	};
	void SetWordColor(size_t wordStart, TCHARString word)
	{

		size_t wordend = wordStart + word.length();
		//match word to one of the dictionary to set the color of the word
		COLORREF col = m_Language->UNKNOWNWORDCOLOR;
		if (0)
		{

		}
		else if (m_Language->STATICCLASS.Valid(word))
		{
			col = m_Language->STATICCLASSCOLOR;
		}
		else if (m_Language->FUNCTIONS.Valid(word))
		{
			col = m_Language->FUNCTIONCOLOR;
		}
		else if (m_Language->KEYWORDS.Valid(word))
		{
			col = m_Language->KEYWORDCOLOR;
		}
		else if (m_Language->LITERALS.Valid(word))
		{
			col = m_Language->LITERALCOLOR;
		}
		else if (m_Language->STATICCLASS.Valid(word))
		{
			col = m_Language->STATICCLASSCOLOR;
		}
		else if (m_Language->CLASSFUNCS.Valid(word))
		{
			col = m_Language->CLASSFUNCCOLOR;
		}

		for (size_t i = wordStart; i < wordend; i++)
		{
			DATA_SET(m_CharColors, i, col);
		}
	};
	TCHARString DoValidName()
	{
		//reads what is after a proper function start
		TCHARString word;
		
		TCHAR s[] = { 0,0 };
		
		//flag the character as read accumulate word
		{
			TCHAR c = DATA_CHAR(m_EBText, m_ReadAt);
			s[0] = c;
			word += s;
			DATA_SET(m_EBTextMirror, m_ReadAt, c);
			
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
				
				DATA_SET(m_EBTextMirror, m_ReadAt, c);
				
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
		COLORREF col = m_Language->TEXTBACKCOLOR;
		//reads all whitespace 'till no more
		//flag the quote
		{
			DATA_SET(m_EBTextMirror, m_ReadAt, DATA_CHAR(m_EBText, m_ReadAt));
			DATA_SET(m_CharColors, m_ReadAt, col);
			m_ReadAt++;
		}
		//read until end of quote
		bool IsStillIt = true;
		while (m_ReadAt < m_DataLen)
		{
			TCHAR c = DATA_CHAR(m_EBText, m_ReadAt);
			//is is string escape? skip over \"
			if (!m_Language->WHITESPACE.Valid(c))
			{
				IsStillIt = false;
			}
			//Done?
			if (IsStillIt)
			{
				//mark as read and set color map
				DATA_SET(m_EBTextMirror, m_ReadAt, c);
				DATA_SET(m_CharColors, m_ReadAt, col);
			}
			else
			{
				return true;
			}
			//keep reading
			m_ReadAt++;
		}
		return true;
	}
	
	
};

