#include "commandScript.h"
#include<fstream>
using namespace std;


CCommandScript::CCommandScript()
{
	totalScriptLines = 0;
	currentLine= 0;
	currentLineChar= 0;
	m_script= 0;
}

CCommandScript::~CCommandScript()
{
	Shutdown();
}

bool CCommandScript::LoadScriptFile(char* fileName)
{
	ifstream input1,input2;
	char tempLine[MAX_LINE_SIZE];

	input1.open(fileName);
	if(!input1.is_open())
		return false;
	Shutdown();

	while(!input1.eof())
	{
		input1.getline(tempLine,MAX_LINE_SIZE,'\n');
		totalScriptLines++;
	}
	input1.close();

	input2.open(fileName);
	if(!input2.is_open())
		return false;

	m_script = new char*[totalScriptLines];

	for(int i=0;i<totalScriptLines;i++)
	{
		m_script[i] = new char[MAX_LINE_SIZE + 1];
		input2.getline(m_script[i],MAX_LINE_SIZE,'\n');
	}
	input2.close();

	return true;
}

void CCommandScript::ParseCommand(char* destcommand)
{
	int commandSize = 0;

	if(!destcommand)
		return;

	if(currentLine >= totalScriptLines)
		return;

	if(currentLineChar >= (int)strlen(m_script[currentLine]))
		return;

	destcommand[0] = '\0';

	if(IsLineComment())
	{
		destcommand[0] = '#';
		destcommand[1] = '\0';

		return;
	}

	while(currentLineChar < (int)strlen(m_script[currentLine]))
	{
		if(m_script[currentLine][currentLineChar] == ' '||
			m_script[currentLine][currentLineChar] == '\n')
			break;

		destcommand[commandSize] = m_script[currentLine][currentLineChar];
		commandSize++;
		currentLineChar++;
	}

	currentLineChar++;
	destcommand[commandSize] = '\0';
}

 void CCommandScript::ParseStringParse(char* destString)
{
	int paramSize = 0;
   bool endQuoteFound = false;

   // If destcommand is NULL, or if we run out of lines, or at the end
   // of the current line then we return.
   if(!destString) return;
   if(currentLine >= totalScriptLines) return;
   if(currentLineChar >= (int)strlen(m_script[currentLine])) return;

   // Initialize string.
   destString[0] = '\0';

   // Skip beginning quote.
   currentLineChar++;

   // Loop through every character until you find a end quote or newline.
   // That means we are at the end of a string.
   while(currentLineChar < (int)strlen(m_script[currentLine]))
      {
         if(m_script[currentLine][currentLineChar] == '"')
            {
               endQuoteFound = true;
               break;
            }

         if(m_script[currentLine][currentLineChar] == '\n')
            break;

         // Save the text in the array.
         destString[paramSize] = m_script[currentLine][currentLineChar];
         paramSize++;
         currentLineChar++;
      }

   // Skip end quotes and next space or newline.  In this system we don't
   // allow strings to take up multiple lines.  You can simple have
   // multiple print string commands instead.
   if(endQuoteFound) currentLineChar += 2;
   else currentLineChar++;

   destString[paramSize] = '\0';
}


void CCommandScript::MoveToNextLine()
{
	currentLine++;
	currentLineChar = 0;
}
void CCommandScript::MoveToStart()
{
	currentLine = 0;
	currentLineChar = 0;
}

int CCommandScript::GetCurrentLineNum()
{
	return currentLine;
}

int CCommandScript::GetTotalLines()
{
	return totalScriptLines;
}

bool CCommandScript::IsLineComment()
{
	if(m_script[currentLine][0] == '#')
		return true;

	return false;
}

void CCommandScript::Shutdown()
{
	if(m_script)
	{
		for(int i=0;i<totalScriptLines;i++)
		{
			if(m_script[i])
			{
				delete[] m_script[i];
				m_script[i] = 0;
			}
		}

		delete m_script;
		m_script = 0;
	}

	totalScriptLines = 0;
	currentLineChar = 0;
	currentLine = 0;
}