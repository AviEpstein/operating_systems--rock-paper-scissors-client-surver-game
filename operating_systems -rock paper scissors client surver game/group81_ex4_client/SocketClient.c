#define _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
/* 
 This file was written for instruction purposes for the 
 course "Introduction to Systems Programming" at Tel-Aviv
 University, School of Electrical Engineering.
Last updated by Amnon Drory, Winter 2011.
 */
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <winsock2.h>


#include "SocketShared.h"
#include "SocketSendRecvTools.h"
#include "SocketClient.h"

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

SOCKET m_socket;


static char wholeCommandToSend[100];
static bool intilizingFlag = FALSE;
/////////////////////////?


//char *clientOpcodeArray[9] = { "CLIENT_REQUEST", "CLIENT_MAIN_MENU", "CLIENT_CPU", "CLIENT_VERSUS", "CLIENT_LEADERBOARD", "CLIENT_PLAYER_MOVE", "CLIENT_REPLAY", "CLIENT_REFRESH", "CLIENT_DISCONNECT" };
//
static HANDLE mutexOpcodeArrayHandle;
BOOL release_res;
DWORD wait_res;//for mutexOpcode
TransferResult_t RecvRes;
//////////////////?

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/


/* Parameters for CreateEvent */
static const LPSECURITY_ATTRIBUTES P_SECURITY_ATTRIBUTES = NULL;
static const BOOL IS_MANUAL_RESET = FALSE; /* Auto-reset event */
static const BOOL IS_INITIALLY_SET = FALSE;
//static  char *P_EVENT_NAME[] = { "Reading_from_user_event","Ready_to_recive_data_event" };

//Reading data coming from the server
static DWORD RecvDataThread(LPVOID lpParam)
{
	THREAD_params *p_params;
	/* check if lpparam is null */
	if (NULL == lpParam)
	{
		///return ERROR_code;
	}
	p_params = (THREAD_params *)lpParam;

	HANDLE Reading_from_user_event_handle;
	HANDLE Ready_to_recive_data_event_handle;
		
	BOOL is_success;
	///handling the events:
	char Reading_from_user_event_name[40];
	char Ready_to_recive_data_event_name[40];
	strcpy(Reading_from_user_event_name, p_params->playerName);
	strcpy(Ready_to_recive_data_event_name, p_params->playerName);


	Reading_from_user_event_handle = GetReadingFromUserEvent(Reading_from_user_event_name);
	Ready_to_recive_data_event_handle = GetReadingFromUserEvent(Ready_to_recive_data_event_name);
	/* Check if succeeded and handle errors */

	TransferResult_t RecvRes;
	//char **protectedOpCodeTable;
	//protectedOpCodeTable = (char**)lpParam;

	/////////////////////////?
	char SendStr[256];
	TransferResult_t SendRes;
	//////////////////////?

	while (1) 
	{
		char *AcceptedStr = NULL;
		
		RecvRes = ReceiveString( &AcceptedStr , m_socket );

		if ( RecvRes == TRNS_FAILED )
		{
			printf("Socket error while trying to write data to socket\n");
			return 0x555;
		}
		else if ( RecvRes == TRNS_DISCONNECTED )
		{
			printf("Server closed connection. Bye!\n");
			return 0x555;
		}

		else
		{
		//	printf("%s\n",AcceptedStr);

	
			/////////////////////writing to shared array ////////////////////////////////////

			
			///	if (wait_res != WAIT_OBJECT_0) ReportErrorAndEndProgram();
			parsInputToList(p_params->protectedOpCodeTable, AcceptedStr);
		
			///if (release_res == FALSE) ReportErrorAndEndProgram();

			///////////////////////////////////////////////////////////////////
			if (STRINGS_ARE_EQUAL(p_params->protectedOpCodeTable[0], "SERVER_APPROVED"))
			{
			}

			//checking if server wants to display to the user th main menu
			if (STRINGS_ARE_EQUAL(p_params->protectedOpCodeTable[0], "SERVER_MAIN_MENU"))
			{
				printf("\n        Main Menu:      \n");
				printf("1. play against another player(if connected)\n");
				printf("2. play against the server\n");
				//printf("3. display the leaderboard\n");
				printf("3. disconnect from server\n");

				///set event from user/sendig thread that signals an input from user needs to be made

				is_success = SetEvent(Reading_from_user_event_handle);
				/* Check if succeeded and handle errors */
				// waiting for send thread to send the opropriate message before starting to recive again
				WaitForSingleObject(Ready_to_recive_data_event_handle, INFINITE);

				//read from mutex protected desion will be 1 or 2 or 3 or 4
				//if 1 then were playig against another player and can we need to send to the surver "CLIENT VERSUS"

			}
			/////////////////////////////////////////////////////////////////////////////
			if (STRINGS_ARE_EQUAL(p_params->protectedOpCodeTable[0], "SERVER_PLAYER_MOVE_REQUEST"))
			{
				printf("Choose a move from the list: Rock, Paper, Scissors, Lizard, Spock\n");
				is_success = SetEvent(Reading_from_user_event_handle);
				/* Check if succeeded and handle errors 
				 waiting for send thread to send the opropriate message before starting to recive again */
				WaitForSingleObject(Ready_to_recive_data_event_handle, INFINITE);

			}
			////////////////////////////////////////////////////////////////////////////
			if (STRINGS_ARE_EQUAL(p_params->protectedOpCodeTable[0], "SERVER_GAME_RESULTS"))
			{
				printf("You played: %s\n", p_params->protectedOpCodeTable[3]);
				printf("%s played: %s\n", p_params->protectedOpCodeTable[1], p_params->protectedOpCodeTable[2]);
				if (!STRINGS_ARE_EQUAL(p_params->protectedOpCodeTable[4], "Tie!"))
				{
					printf("%s won!\n\n", p_params->protectedOpCodeTable[4]);
				}
			}
			////////////////////////////////////////////////////////////////////////////
			if (STRINGS_ARE_EQUAL(p_params->protectedOpCodeTable[0], "SERVER_GAME_OVER_MENU"))
			{
				printf("Choose what to do next:\n");
				printf("1. Play again\n");
				printf("2. Return to the main menu\n\n");
				is_success = SetEvent(Reading_from_user_event_handle);
				/* Check if succeeded and handle errors */
				// waiting for send thread to send the opropriate message before starting to recive again
				WaitForSingleObject(Ready_to_recive_data_event_handle, INFINITE);
			}
			///////////////////////////////////////////////////////////////////////////
			//cheeking if invited to play against another player
			if (STRINGS_ARE_EQUAL(p_params->protectedOpCodeTable[0], "SERVER_INVITE"))
			{
				printf("2 players invite happend other player is %s\n\n", p_params->protectedOpCodeTable[1]);
			}
			///////////////////////////////////////////////////////////////////////////
			if (STRINGS_ARE_EQUAL(p_params->protectedOpCodeTable[0], "SERVER_OPPONENT_QUIT"))
			{
				printf("Your opponent %s chose not to play again. Redirecting to main menu", p_params->protectedOpCodeTable[1]);

			}
			//////////////////////////////////////////////////////////////////////////////
			if (STRINGS_ARE_EQUAL(p_params->protectedOpCodeTable[0], "SERVER_DENIED"))
			{
				printf("Server closed connection. Bye!\n");
				printf("Server on %s : %s denied the connection request\n", p_params->serverAddressString, p_params->serverPortNumberString);
				printf("choose what to do next:\n");
				printf("1. Try to reconnect\n");
				printf("2. exit\n\n");
				is_success = SetEvent(Reading_from_user_event_handle);
				WaitForSingleObject(Ready_to_recive_data_event_handle, INFINITE);
				//return 0x555;
				//free(AcceptedStr);
				//goto cleen_up_recive_thread;
			}
			////////////////////////////////////////////////////////////////////////////
			if (STRINGS_ARE_EQUAL(p_params->protectedOpCodeTable[0], "SERVER_NO_OPPONENTS"))
			{
				printf("No oponent was found. Re-directing to main menu\n\n");

				
			}

		}
		

		free(AcceptedStr);
	}

	cleen_up_recive_thread:	

	is_success = CloseHandle(Reading_from_user_event_handle);
	is_success = CloseHandle(Ready_to_recive_data_event_handle);
	/* Check if succeeded and handle errors */

}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

//Sending data to the server
static DWORD SendDataThread(LPVOID lpParam)
{
	THREAD_params *p_params;
	/* check if lpparam is null */
	if (NULL == lpParam)
	{
		///return ERROR_code;
	}
	p_params = (THREAD_params *)lpParam;



	char SendStr[256];
	char choiceFromUser[256];
	TransferResult_t SendRes;

	char Reading_from_user_event_name[40];
	char Ready_to_recive_data_event_name[40];
	strcpy(Reading_from_user_event_name, p_params->playerName);
	strcpy(Ready_to_recive_data_event_name, p_params->playerName);


	HANDLE Reading_from_user_event_handle;
	HANDLE Ready_to_recive_data_event_handle;
	BOOL is_success;

	Reading_from_user_event_handle = GetReadingFromUserEvent(Reading_from_user_event_name);
	Ready_to_recive_data_event_handle = GetReadingFromUserEvent(Ready_to_recive_data_event_name);
	/* Check if succeeded and handle errors */
	bool two_player_mode = false;

	while (1)
	{
	
		//release_res = ReleaseMutex(mutexOpcodeHandle);
		//free(opcode);
		//////////////////////////////?
		if (intilizingFlag == TRUE)
		{
			intilizingFlag = FALSE;
			strcpy(SendStr, wholeCommandToSend);
			SendRes = SendString(SendStr, m_socket);

			if (SendRes == TRNS_FAILED)
			{
				printf("Socket error while trying to write data to socket\n");
				return 0x555;
			}
		}
		

		/* Wait to be authorized to recive from user */
		WaitForSingleObject(Reading_from_user_event_handle, INFINITE);
		/* Check if succeeded and handle errors */

		gets_s(choiceFromUser, sizeof(choiceFromUser)); //Reading a string from the keyboard

		if (STRINGS_ARE_EQUAL(p_params->protectedOpCodeTable[0], "SERVER_MAIN_MENU") && STRINGS_ARE_EQUAL(choiceFromUser, "1"))
		{
			strcpy(SendStr, "CLIENT_VERSUS");
			two_player_mode = true;
		}
		/////////////////////////////////////////////////////////////////////////////////////
		if (STRINGS_ARE_EQUAL(p_params->protectedOpCodeTable[0], "SERVER_MAIN_MENU") && STRINGS_ARE_EQUAL(choiceFromUser, "2"))
		{
			strcpy(SendStr, "CLIENT_CPU");
		}
		////////////////////////////////////////////////////////////////////////////////////////	
		if (STRINGS_ARE_EQUAL(p_params->protectedOpCodeTable[0], "SERVER_MAIN_MENU") && STRINGS_ARE_EQUAL(choiceFromUser, "3"))
		{
			strcpy(SendStr,"CLIENT_DISCONNECT");
			SendRes = SendString(SendStr, m_socket);

			if (SendRes == TRNS_FAILED)
			{
				printf("Socket error while trying to write data to socket\n");
				return 0x555;
			}
			Sleep(2000);
			goto clean_up_send_thread;
			
		}
		/////////////////////////////////////////////////////////////////////////////////////////	
		if (STRINGS_ARE_EQUAL(p_params->protectedOpCodeTable[0], "SERVER_PLAYER_MOVE_REQUEST"))
		{
			bool inputIsValidMove = false;
			while (!inputIsValidMove)
			{
				char *userMove = strupr(choiceFromUser);
				if (STRINGS_ARE_EQUAL(userMove, "ROCK") || STRINGS_ARE_EQUAL(userMove, "PAPER") || STRINGS_ARE_EQUAL(userMove, "SCISSORS") || STRINGS_ARE_EQUAL(userMove, "LIZARD") || STRINGS_ARE_EQUAL(userMove, "SPOCK"))
				{
					inputIsValidMove = true;
					strcpy(SendStr, "CLIENT_PLAYER_MOVE:");
					strcat(SendStr, userMove);
					strcat(SendStr, "\n");
				}
				else
				{
					printf("You entered an invalid move. please re-enter your choice\n");
					printf("Choose a move from the list: Rock, Paper, Scissors, Lizard, Spock\n\n");
					gets_s(choiceFromUser, sizeof(choiceFromUser));
				}
			}
		}
		//////////////////////////////////////////////////////////////////////////////////////////
		if (STRINGS_ARE_EQUAL(p_params->protectedOpCodeTable[0], "SERVER_GAME_OVER_MENU") && STRINGS_ARE_EQUAL(choiceFromUser, "1"))
		{
			if(two_player_mode == true) //player played against the computer
			{
				strcpy(SendStr,"CLIENT_REPLAY" );

			}
			else //player played against other player
			{
				strcpy(SendStr, "CLIENT_CPU");
			}
		}
		////////////////////////////////////////////////////////////////////////////////////////
		if (STRINGS_ARE_EQUAL(p_params->protectedOpCodeTable[0], "SERVER_GAME_OVER_MENU") && STRINGS_ARE_EQUAL(choiceFromUser, "2"))
		{
			two_player_mode = false;
			strcpy(SendStr, "CLIENT_MAIN_MENU");
		}
		//////////////////////////////////////////////////////////////////////////////////////
		if (STRINGS_ARE_EQUAL(p_params->protectedOpCodeTable[0], "SERVER_DENIED") && STRINGS_ARE_EQUAL(choiceFromUser, "1"))
		{
			//try to connect again
			//client req insteed of next
			strcpy(SendStr, "CLIENT_REQUEST");
			//intilizingFlag = TRUE;
			//SendRes = SendString(SendStr, m_socket);
			//if (SendRes == TRNS_FAILED)
			//{
			//	printf("Socket error while trying to write data to socket\n");
			//	return 0x555;
			//}

			//strcpy(SendStr, "CLIENT_REQUEST");
			//Sleep(5000);
			//MainClient (p_params->serverAddressString, p_params->serverPortNumberString, p_params->playerName);

		}
		//////////////////////////////////////////////////////////////////////////////////////
		if (STRINGS_ARE_EQUAL(p_params->protectedOpCodeTable[0], "SERVER_DENIED") && STRINGS_ARE_EQUAL(choiceFromUser, "2"))
		{
			//quit
			//return 0x555; //"quit" signals an exit from the client side
			goto clean_up_send_thread;
		}
		//////////////////////////////////////////////////////////////////////////////////////
		if (STRINGS_ARE_EQUAL(SendStr, "quit"))
		{
			//return 0x555; //"quit" signals an exit from the client side
			goto clean_up_send_thread;
		}
		//////////////////////////////////////////////////////////////////////////////////////
	

		SendRes = SendString(SendStr, m_socket);

		if (SendRes == TRNS_FAILED)
		{
			printf("Socket error while trying to write data to socket\n");
			return 0x555;
		}

		//setting event that the recevivg thred can contiue to recive
		is_success = SetEvent(Ready_to_recive_data_event_handle);

		/////wait for event here

	}
	clean_up_send_thread:
	is_success = CloseHandle(Reading_from_user_event_handle);
	is_success = CloseHandle(Ready_to_recive_data_event_handle);
	/* Check if succeeded and handle errors */

}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

void MainClient(char *serverAddressString, char *serverPortNumberString, char *userNameString)
{
	//////////////////////?
	//char recievedOpcode[20];
	//wholeCommandToSend = (char*)malloc(50 * sizeof(char));
	//*wholeCommandToSend = "Nothing";

	BOOL an_error_occured = FALSE;
	mutexOpcodeArrayHandle = CreateMutex(
		NULL,   /* default security attributes */
		FALSE,	/* don't lock mutex immediately */			
		"opcodeArrayMutex"); /* un-named */
	if (mutexOpcodeArrayHandle == NULL) 
	{ 
		return an_error_occured = TRUE;/// goto Main_Cleanup_3;  need to make clean up!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	}
	


	
	THREAD_params thread_params;
	strcpy(thread_params.playerName, userNameString);
	strcpy(thread_params.serverAddressString, serverAddressString);
	strcpy(thread_params.serverPortNumberString, serverPortNumberString);

	SOCKADDR_IN clientService;
	HANDLE hThread[2];

    // Initialize Winsock.
    WSADATA wsaData; //Create a WSADATA object called wsaData.
	//The WSADATA structure contains information about the Windows Sockets implementation.
	
	//Call WSAStartup and check for errors.
    int iResult = WSAStartup( MAKEWORD(2, 2), &wsaData );
    if ( iResult != NO_ERROR )
        printf("Error at WSAStartup()\n");

	//Call the socket function and return its value to the m_socket variable. 
	// For this application, use the Internet address family, streaming sockets, and the TCP/IP protocol.
	
	// Create a socket.
    m_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

	// Check for errors to ensure that the socket is a valid socket.
    if ( m_socket == INVALID_SOCKET ) {
        printf( "Error at socket(): %ld\n", WSAGetLastError() );
        WSACleanup();
        return;
    }
	/*
	 The parameters passed to the socket function can be changed for different implementations. 
	 Error detection is a key part of successful networking code. 
	 If the socket call fails, it returns INVALID_SOCKET. 
	 The if statement in the previous code is used to catch any errors that may have occurred while creating 
	 the socket. WSAGetLastError returns an error number associated with the last error that occurred.
	 */


	//For a client to communicate on a network, it must connect to a server.
    // Connect to a server.

    //Create a sockaddr_in object clientService and set  values.
    clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr(serverAddressString); //Setting the IP address to connect to

	int serverPortNumberInt = atoi(serverPortNumberString);
    clientService.sin_port = htons(serverPortNumberInt); //Setting the port to connect to.
//printf("Server port as int: %d", serverPortNumberInt);
	/*
		AF_INET is the Internet address family. 
	*/

    // Call the connect function, passing the created socket and the sockaddr_in structure as parameters.
	// Repeats choise menu until succees
	// Check for general errors.
	BOOL connectionFlag = FALSE;
	while (!connectionFlag)
	{
		if (connect(m_socket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
			printf("Failed connecting to server on %s:%s.\n", serverAddressString, serverPortNumberString);
			printf("Choose what to do next:\n1. Try to reconnect \n2. Exit\n");
			char decision[5];
			gets_s(decision, sizeof(decision));
			if (STRINGS_ARE_EQUAL(decision, "1"))
			{
				connectionFlag = FALSE;
			}
			else if (STRINGS_ARE_EQUAL(decision, "2"))
			{
				connectionFlag = TRUE;
				WSACleanup();
				return;
			}
			else
			{
				printf("Invalid decision. Decision must be '1' or '2', without spaces. System will try to reconnect automatically.\n");
			}
		}
		else
		{
			connectionFlag = TRUE;
			//wait_res = WaitForSingleObject(mutexOpcodeHandle, INFINITE);//// i dont think we need a mutex this will only run once before the are multipule strings

			strcpy(wholeCommandToSend, "CLIENT_REQUEST:");
			strcat(wholeCommandToSend, userNameString);
			strcat(wholeCommandToSend, "\n");

			intilizingFlag = TRUE;

			//release_res = ReleaseMutex(mutexOpcodeHandle);



			//enum opcode_client clientConnect;
			//clientConnect = CLIENT_REQUEST;
			//char *result = OpcodeClientMenu(clientConnect, userNameString);
			//printf("Connected to server on %s:%s\n", serverAddressString, serverPortNumberString);
		}
	}

    // Send and receive data.
	/*
		In this code, two integers are used to keep track of the number of bytes that are sent and received. 
		The send and recv functions both return an integer value of the number of bytes sent or received, 
		respectively, or an error. Each function also takes the same parameters: 
		the active socket, a char buffer, the number of bytes to send or receive, and any flags to use.
	*/
		

	hThread[0] = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)SendDataThread,
		&thread_params,//&threadParams,
		0,
		NULL
	);
	hThread[1] = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)RecvDataThread,
		&thread_params,
		0,
		NULL
	);



	WaitForMultipleObjects(2,hThread,FALSE,INFINITE);

	TerminateThread(hThread[0],0x555);
	TerminateThread(hThread[1],0x555);

	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);
	shutdown(m_socket, SD_SEND);
	char *AcceptedStr = NULL;

	RecvRes = ReceiveString(&AcceptedStr, m_socket);

	/*char *AcceptedStr = NULL;

	RecvRes = ReceiveString(&AcceptedStr, m_socket);

	if (RecvRes == TRNS_FAILED)
	{
		printf("Socket error while trying to write data to socket\n");
		return 0x555;
	}
	else if (RecvRes == TRNS_DISCONNECTED)
	{
		printf("Server closed connection. Bye!\n");
		return 0x555;
	}
	free(AcceptedStr);*/
	closesocket(m_socket);
	
	WSACleanup();
    
	return;
}

HANDLE GetReadingFromUserEvent(char* P_EVENT_NAME)
{
	HANDLE Reading_from_user_event_handle;
	DWORD last_error;

	/* Get handle to event by name. If the event doesn't exist, create it */
	Reading_from_user_event_handle = CreateEvent(
		P_SECURITY_ATTRIBUTES, /* default security attributes */
		IS_MANUAL_RESET,       /* manual-reset event */
		IS_INITIALLY_SET,      /* initial state is non-signaled */
		P_EVENT_NAME);         /* name */
	/* Check if succeeded and handle errors */

	last_error = GetLastError();
	/* If last_error is ERROR_SUCCESS, then it means that the event was created.
	   If last_error is ERROR_ALREADY_EXISTS, then it means that the event already exists */

	return Reading_from_user_event_handle;
}

