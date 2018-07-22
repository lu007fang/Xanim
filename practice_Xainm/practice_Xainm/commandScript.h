#pragma once
#ifndef _COMMANDSCRIPT_H_
#define _COMMANDSCRIPT_H_

#define MAX_LINE_SIZE 3072

class CCommandScript
{
public:
	CCommandScript();
	~CCommandScript();

	bool LoadScriptFile(char* fileName);
	void ParseCommand(char* destcommand);
	void Shutdown();

	int GetTotalLines();
	int GetCurrentLineNum();
	void MoveToNextLine();
	void MoveToStart();
	bool IsLineComment();

	void ParseStringParse(char* destString);
	bool ParseBoolParam();
	int ParseIntParam();
    float ParseFloatParam();

private:
	int totalScriptLines;
	int currentLine;
	int currentLineChar;
	char** m_script;
};



#endif