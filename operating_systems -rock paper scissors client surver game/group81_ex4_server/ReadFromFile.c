#define _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS

#include "ReadFromFile.h"
#include "SocketServer.h"

void ReadFromFile(char* path, char *name)
{
	errno_t retval;
	FILE *p_stream = NULL;
	char p_line[15] = { 0 };
	char *p_ret_str = NULL;


	// Open file
	retval = fopen_s(&p_stream, path, "r");
	if (0 != retval)
	{
		//here we need to deal with the fact that this thread failed and think what to do with it
		printf("Failed to open file.\n");
		//return STATUS_CODE_FAILURE;
	}

	// Read lines
	//p_ret_str = fgets(p_line, 4, p_stream);
	fgets(p_line, 15, p_stream);
	strcpy(name , p_line);

	// Close file
	retval = fclose(p_stream);
	if (0 != retval)
	{
		printf("Failed to close file.\n");
		//return STATUS_CODE_FAILURE;
	}
}