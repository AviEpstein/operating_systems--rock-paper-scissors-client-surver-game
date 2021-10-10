/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
/* 
 This file was written for instruction purposes for the 
 course "Introduction to Systems Programming" at Tel-Aviv
 University, School of Electrical Engineering, Winter 2011, 
 by Amnon Drory.
*/
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
#ifndef SOCKET_CLIENT_H
#define SOCKET_CLIENT_H

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

void MainClient(char *serverAddressString, char *serverPortNumberString, char *userNameString);

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

//
//enum opcode_client
//{
//	CLIENT_REQUEST		= 1,
//	CLIENT_MAIN_MENU	= 2,
//	CLIENT_CPU			= 3,
//	CLIENT_LEADERBOARD  = 4,
//	CLIENT_PLAYER_MOVE	= 5,
//	CLIENT_REPLAY		= 6,
//	CLIENT_REFRESH		= 7,
//	CLIENT_DISCONNECT	= 8,
//}opcode_client;

HANDLE GetReadingFromUserEvent(char* P_EVENT_NAME);

typedef struct THREAD_params
{
	char* protectedOpCodeTable[5] ;
	char playerName[15];
	char serverAddressString[16];
	char serverPortNumberString[5];

	
} THREAD_params;



#endif // SOCKET_CLIENT_H