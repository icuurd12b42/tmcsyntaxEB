#include "stdafx.h"
#include "HookedCtrl.h"
#include "Parser.h"


Parser::Parser()
{
}


Parser::~Parser()
{
}

void Parser::Parse(int TextLen) {
	m_ReadAt = 0;
	m_DataLen = TextLen;
	m_Language = m_Ctrl->m_Language;
	SeekFirstDifference();
	
	while (m_ReadAt< m_DataLen)
	{
		bool processed = false;
		TCHAR c = DATA_CHAR(m_EBText, m_ReadAt);
		if (!processed && m_Language->COMMENTCHARS.Valid(c))
		{
			if (m_Language->MULTILINECOMMENTSTART.Valid(DATA_PTR(m_EBText, m_ReadAt)))
			{
				processed = DoMultiLineComment();
			}
			else if (m_Language->SINGLELINECOMMENT.Valid(DATA_PTR(m_EBText, m_ReadAt)))
			{
				processed = DoSingleLineComment();
				
			}
		}
		if (!processed && m_Language->QUOTES.Valid(c))
		{
			processed = DoQuotedText();
		}
		if (!processed && m_Language->WHITESPACE.Valid(c))
		{
			processed = DoWhitespace();
		}
		if (!processed && m_Language->BINPRECHECK.Valid(c))
		{
			processed = DoBinary();
		}
		if (!processed && m_Language->OCTPRECHECK.Valid(c))
		{
			processed = DoOctal();
		}
		if (!processed && m_Language->HEXPRECHECK.Valid(c))
		{
			processed = DoHex();
		}
		if (!processed && m_Language->NUMPRECHECK.Valid(c))
		{
			processed = DoNumber();
		}
		if (!processed && m_Language->VALIDNAMESTART.Valid(c))
		{
			CompoundString();
			processed = true;
		}
		
		if (!processed && m_Language->COLORSET1CHARS.Valid(c))
		{
			DATA_SET(m_EBTextMirror, m_ReadAt, c);
			DATA_SET(m_CharColors, m_ReadAt, m_Language->COLORSET1);
			processed = true;
			m_ReadAt++;
		}
		if (!processed && m_Language->COLORSET2CHARS.Valid(c))
		{
			DATA_SET(m_EBTextMirror, m_ReadAt, c);
			DATA_SET(m_CharColors, m_ReadAt, m_Language->COLORSET2);
			processed = true;
			m_ReadAt++;
		}
		if (!processed && m_Language->COLORSET3CHARS.Valid(c))
		{
			DATA_SET(m_EBTextMirror, m_ReadAt, c);
			DATA_SET(m_CharColors, m_ReadAt, m_Language->COLORSET3);
			processed = true;
			m_ReadAt++;
		}
		if (!processed && m_Language->COLORSET4CHARS.Valid(c))
		{
			DATA_SET(m_EBTextMirror, m_ReadAt, c);
			DATA_SET(m_CharColors, m_ReadAt, m_Language->COLORSET4);
			processed = true;
			m_ReadAt++;
		}
		//last one bad char 
		if(!processed)
		{
			DATA_SET(m_EBTextMirror, m_ReadAt, c);
			DATA_SET(m_CharColors, m_ReadAt, m_Language->BADCHARSCOLOR);
			m_ReadAt++;
		}

		//finally the ).Function() or ].Property handling, another hack, to hell with the consequences for other languages
		if (m_ReadAt > 0) //because read at-1 used
		{ 
			//since the single charr handler likely ate the ] or ) and it's likely right next to the current pos, unless someone made an odd language file...
			//get the last eaten character
			bool done = false;
			while (!done)
			{
				TCHAR c = DATA_CHAR(m_EBText, m_ReadAt - 1);
				// is ) or ]
				if ((c == ')' || c == ']' || c == '\'' || c == '\"'))
				{
					//are we on a . or ->?
					int WordSize = 0;
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
									COLORREF col = m_Language->WORDBINDERCOLOR;
									DATA_SET(m_EBTextMirror, m_ReadAt, DATA_CHAR(m_EBText, m_ReadAt));
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
	int wordcount = 0;
	TCHARString fullname; //for the full
	int fullnamestart = m_ReadAt;
	TCHARString ThisName;
	//for this word and any subsequent . word like Something.Something.something
	do {
		int thisnamestart = m_ReadAt;
		//get the word
		TCHARString ThisName = DoValidName();
		//Just read [something]

		//check if joiner something[->]
		int WordSize = 0;
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
			TCHARString ClassName = fullname;


			fullname += L".";
			//Loose class is Either ClassName or .PossibleMember for the search
			TCHARString ClassFuncname;
			if (wordcount || ClassFuncsOnly) ClassFuncname += L"."; //set the prefix for class functions so search will do ".FuncName" search in ref table
			ClassFuncname += ThisName;
			//handle the [->] type joiner, flag and set color
			TCHAR s[] = { 0,0 };
			repeat(WordSize) {
				COLORREF col = m_Language->WORDBINDERCOLOR;
				DATA_SET(m_EBTextMirror, m_ReadAt, DATA_CHAR(m_EBText, m_ReadAt));
				DATA_SET(m_CharColors, m_ReadAt, col);
				m_ReadAt++;
			}

			//color the table positions
			if ((IsFirstWord == true || IsStaticClass == true) && ClassFuncsOnly == false && m_Language->STATICCLASS.Valid(ClassName))
			{
				//if(Something.Something.Something. exactly)
				IsStaticClass = true;
				ColorizeWord(ThisName, thisnamestart, m_Language->STATICCLASSCOLOR);
			}
			else if ((IsFirstWord == true || IsClassFunc == true || ClassFuncsOnly == true) && m_Language->CLASSFUNCS.Valid(ClassFuncname))
			{
				//If CallsName or .MemberName
				IsClassFunc = true;
				ColorizeWord(ThisName, thisnamestart, m_Language->CLASSFUNCCOLOR);
			}
			else
			{
				//here we hit imvalid class name or member
				ColorizeWord(ThisName, thisnamestart, m_Language->UNKNOWNWORDCOLOR);
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
			TCHARString ClassName = fullname;
			//color the table positions
			//Loose clase uses ".MemberName"
			TCHARString ClassFuncname;
			ClassFuncname += L".";
			ClassFuncname += ThisName;

			if (m_Language->STATICCLASS.Valid(ClassName))
			{
				ColorizeWord(ThisName, thisnamestart, m_Language->STATICCLASSCOLOR);
			}
			else if (m_Language->CLASSFUNCS.Valid(ClassFuncname))
			{
				ColorizeWord(ThisName, thisnamestart, m_Language->CLASSFUNCCOLOR);
			}
			else
			{
				ColorizeWord(ThisName, thisnamestart, m_Language->UNKNOWNWORDCOLOR);
			}
			done = true;
		}
		else
		{
			//OK, we did not consruct a this.that.shomething string...
			//plain old [something]... single valid word, may be any of the words or any of the dictionaries
			COLORREF col = m_Language->UNKNOWNWORDCOLOR;
			TCHARString TheName;
			
			if (ClassFuncsOnly) TheName += L".";
			TheName += ThisName;

			if (ClassFuncsOnly == false && m_Language->STATICCLASS.Valid(TheName))
			{
				col = m_Language->STATICCLASSCOLOR;
			}
			else if (m_Language->CLASSFUNCS.Valid(TheName))
			{
				col = m_Language->CLASSFUNCCOLOR;
			}
			else if (ClassFuncsOnly == false && m_Language->FUNCTIONS.Valid(TheName))
			{
				col = m_Language->FUNCTIONCOLOR;
			}
			else if (ClassFuncsOnly == false && m_Language->KEYWORDS.Valid(TheName))
			{
				col = m_Language->KEYWORDCOLOR;
			}
			else if (ClassFuncsOnly == false && m_Language->LITERALS.Valid(TheName))
			{
				col = m_Language->LITERALCOLOR;
			}
			else if (ClassFuncsOnly == false && DATA_CHAR(m_EBText,m_ReadAt) == '(')
			{
				//not found in dictionaries but ( at the end, assume user function
				col = m_Language->USERFUNCTIONCOLOR;
			}
			ColorizeWord(ThisName, thisnamestart, col);
			done = true;
		}

		//if we read up and found another
		IsFirstWord = false;
		wordcount++;
	} while (!done);
}

/*
void Parser::CompoundString()
{
//For the record I'm very exited this works but not especially proud of the hackcoding
bool done = false;
int DidJoin = 0;
bool IsFirstWord = true;
bool IsStaticClass = false;
bool IsClassFunc = false;
bool ThereIsADot = false;
int wordcount = 0;
TCHARString fullname; //for the full
int fullnamestart = m_ReadAt;
TCHARString ThisName;
//for this word and any subsequent . word like Something.Something.something
do {
int thisnamestart = m_ReadAt;
//get the word
TCHARString ThisName = DoValidName();
//Just read [something]

//check if joiner something[->]
int WordSize = 0;
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
TCHARString ClassName = fullname;


fullname += L".";
//Loose class is Either ClassName or .PossibleMember for the search
TCHARString ClassFuncname;
if (wordcount) ClassFuncname += L"."; //set the prefix for class functions so search will do ".FuncName" search in ref table
ClassFuncname += ThisName;
//handle the [->] type joiner, flag and set color
TCHAR s[] = { 0,0 };
repeat(WordSize) {
COLORREF col = m_Language->WORDBINDERCOLOR;
DATA_SET(m_EBTextMirror, m_ReadAt, DATA_CHAR(m_EBText, m_ReadAt));
DATA_SET(m_CharColors, m_ReadAt, col);
m_ReadAt++;
}

//color the table positions
if ((IsFirstWord == true || IsStaticClass == true) && m_Language->STATICCLASS.Valid(ClassName))
{
//if(Something.Something.Something. exactly)
IsStaticClass = true;
ColorizeWord(ThisName, thisnamestart, m_Language->STATICCLASSCOLOR);
}
else if ((IsFirstWord == true || IsClassFunc == true) && m_Language->CLASSFUNCS.Valid(ClassFuncname))
{
//If CallsName or .MemberName
IsClassFunc = true;
ColorizeWord(ThisName, thisnamestart, m_Language->CLASSFUNCCOLOR);
}
else
{
//here we hit imvalid class name or member
ColorizeWord(ThisName, thisnamestart, m_Language->UNKNOWNWORDCOLOR);
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
TCHARString ClassName = fullname;
//color the table positions
//Loose clase uses ".MemberName"
TCHARString ClassFuncname;
ClassFuncname += L".";
ClassFuncname += ThisName;
if (m_Language->STATICCLASS.Valid(ClassName))
{
ColorizeWord(ThisName, thisnamestart, m_Language->STATICCLASSCOLOR);
}
else if (m_Language->CLASSFUNCS.Valid(ClassFuncname))
{
ColorizeWord(ThisName, thisnamestart, m_Language->CLASSFUNCCOLOR);
}
else
{
ColorizeWord(ThisName, thisnamestart, m_Language->UNKNOWNWORDCOLOR);
}
done = true;
}
else
{
//OK, we did not consruct a this.that.shomething string...
//plain old [something]... single valid word, may be any of the words or any of the dictionaries
COLORREF col = m_Language->UNKNOWNWORDCOLOR;
if (m_Language->STATICCLASS.Valid(ThisName))
{
col = m_Language->STATICCLASSCOLOR;
}
else if (m_Language->CLASSFUNCS.Valid(ThisName))
{
col = m_Language->CLASSFUNCCOLOR;
}
else if (m_Language->FUNCTIONS.Valid(ThisName))
{
col = m_Language->FUNCTIONCOLOR;
}
else if (m_Language->KEYWORDS.Valid(ThisName))
{
col = m_Language->KEYWORDCOLOR;
}
else if (m_Language->LITERALS.Valid(ThisName))
{
col = m_Language->LITERALCOLOR;
}
ColorizeWord(ThisName, thisnamestart, col);
done = true;
}

//if we read up and found another
IsFirstWord = false;
wordcount++;
} while (!done);
}
*/
/*
//are we on a . or ->?
int WordSize = 0;
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
COLORREF col = m_Language->WORDBINDERCOLOR;
DATA_SET(m_EBTextMirror, m_ReadAt, DATA_CHAR(m_EBText, m_ReadAt));
DATA_SET(m_CharColors, m_ReadAt, col);
m_ReadAt++;
}
int thisnamestart = m_ReadAt;
TCHARString ThisName = DoValidName();
TCHARString ClassFuncName;
ClassFuncName += L".";
ClassFuncName += ThisName;
if (m_Language->CLASSFUNCS.Valid(ClassFuncName))
{
ColorizeWord(ThisName, thisnamestart, m_Language->CLASSFUNCCOLOR);
}
else
{
ColorizeWord(ThisName, thisnamestart, m_Language->UNKNOWNWORDCOLOR);
}
}
}
}
*/