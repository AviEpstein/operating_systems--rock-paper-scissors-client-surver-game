#define _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS

#include "WriteToFile.h"
#include "SocketServer.h"


void writeToFile(char playerName[], char movePlayed[])
{
	char pathToFolder[] =  "..\\GameSession.txt";
	FILE *pFile;
	bool fileExists = doesFileExists(pathToFolder);
	if (fileExists == FALSE)
	{
		pFile = fopen(pathToFolder, "w");
		fclose(pFile);
	}
	pFile = fopen(pathToFolder, "a");
	if (pFile == NULL)
	{
		perror("Error opening file.");
	}
	else
	{
		char *toAdd = " ";
		strcat(playerName, toAdd);
		fprintf(pFile, playerName);
		//		strcat(toAdd, gradeToAppend);
		fprintf(pFile, movePlayed);
		fprintf(pFile, "\n");
	}
	fclose(pFile);
}



/*
 * Check if a file exist using fopen() function
 * return 1 if the file exist otherwise return 0
 */
bool doesFileExists(const char * filename)
{
	/* try to open file to read */
	FILE *file;
	if (file = fopen(filename, "r"))
	{
		fclose(file);
		return TRUE;
	}
	return FALSE;
}