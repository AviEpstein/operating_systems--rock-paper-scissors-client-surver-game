#define _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS
#include "SocketServer.h"
#include "SocketShared.h"


int main(int argc, char *argv[])
{
	//Checking that input is valid

	if (argc < 2)
	{
		printf("ERROR: Not enough input arguments");
		return ERROR_CODE;
	}

	char *portNumberString[5];

	strcpy(portNumberString, argv[1]);
	MainServer(portNumberString);
}