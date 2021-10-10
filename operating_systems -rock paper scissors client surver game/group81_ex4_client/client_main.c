#define _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS

#include "SocketClient.h"
#include "SocketShared.h"


int main(int argc, char *argv[])
{
	//Checking that input is valid

	if (argc < 4)
	//if (argc<3)
	{
		printf("ERROR: Not enough input arguments");
		return ERROR_CODE;
	}

	char serverAddressString[16];
	char serverPortNumberString[5];
	char userNameString[20];


	strcpy(serverAddressString, argv[1]);
//printf("IP: %s\n", serverAddressString);
	strcpy(serverPortNumberString, argv[2]);
//printf("Port: %s\n", portNumberString);
	strcpy(userNameString, argv[3]);
//printf("UserName: %s", userNameString);

	//static int tryToReconnect = 1;
	//static int exitMenu = 2;







	MainClient(serverAddressString, serverPortNumberString, userNameString);
	return 0;
}