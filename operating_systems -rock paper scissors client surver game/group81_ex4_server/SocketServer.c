/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
/*
 This file was written for instruction purposes for the
 course "Introduction to Systems Programming" at Tel-Aviv
 University, School of Electrical Engineering.
Last updated by Amnon Drory, Winter 2011.
 */
 /*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>

#include "SocketShared.h"
#include "SocketSendRecvTools.h"
#include "WriteToFile.h"
#include "SocketServer.h"

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#define NUM_OF_WORKER_THREADS 2

#define MAX_LOOPS 7

#define SEND_STR_SIZE 50

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

HANDLE ThreadHandles[NUM_OF_WORKER_THREADS];
SOCKET ThreadInputs[NUM_OF_WORKER_THREADS];

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

static int FindFirstUnusedThreadSlot();
static void CleanupWorkerThreads();
static DWORD ServiceThread(SOCKET *t_socket);

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
char *clientOpcodeArray[9] = { "CLIENT_REQUEST", "CLIENT_MAIN_MENU", "CLIENT_CPU", "CLIENT_VERSUS", "CLIENT_LEADERBOARD", "CLIENT_PLAYER_MOVE", "CLIENT_REPLAY", "CLIENT_REFRESH", "CLIENT_DISCONNECT" };
char *movesArray[5] = { "ROCK", "PAPER", "SCISSORS", "LIZARD", "SPOCK" };
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
/* Parameters for CreateEvent */
static const LPSECURITY_ATTRIBUTES P_SECURITY_ATTRIBUTES = NULL;
static const BOOL IS_MANUAL_RESET = FALSE; /* Auto-reset event */
static const BOOL IS_INITIALLY_SET = FALSE;
static const char* P_EVENT_NAME[] = { "Player_2_ready_event","player_1_ready_event" };



void MainServer(char *portNumberString)
{
	int Ind;
	int Loop;
	SOCKET MainSocket = INVALID_SOCKET;
	unsigned long Address;
	SOCKADDR_IN service;
	int bindRes;
	int ListenRes;

	// Initialize Winsock.
	WSADATA wsaData;
	int StartupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (StartupRes != NO_ERROR)
	{
		printf("error %ld at WSAStartup( ), ending program.\n", WSAGetLastError());
		// Tell the user that we could not find a usable WinSock DLL.                                  
		return;
	}

	/* The WinSock DLL is acceptable. Proceed. */

	// Create a socket.    
	MainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (MainSocket == INVALID_SOCKET)
	{
		printf("Error at socket( ): %ld\n", WSAGetLastError());
		goto server_cleanup_1;
	}

	// Bind the socket.
	/*
		For a server to accept client connections, it must be bound to a network address within the system.
		The following code demonstrates how to bind a socket that has already been created to an IP address
		and port.
		Client applications use the IP address and port to connect to the host network.
		The sockaddr structure holds information regarding the address family, IP address, and port number.
		sockaddr_in is a subset of sockaddr and is used for IP version 4 applications.
   */
   // Create a sockaddr_in object and set its values.
   // Declare variables

	Address = inet_addr(SERVER_ADDRESS_STR);
	if (Address == INADDR_NONE)
	{
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
			SERVER_ADDRESS_STR);
		goto server_cleanup_2;
	}

	service.sin_family = AF_INET;
	service.sin_addr.s_addr = Address;
	service.sin_port = htons(SERVER_PORT); //The htons function converts a u_short from host to TCP/IP network byte order 
									   //( which is big-endian ).
	/*
		The three lines following the declaration of sockaddr_in service are used to set up
		the sockaddr structure:
		AF_INET is the Internet address family.
		"127.0.0.1" is the local IP address to which the socket will be bound.
		2345 is the port number to which the socket will be bound.
	*/

	// Call the bind function, passing the created socket and the sockaddr_in structure as parameters. 
	// Check for general errors.
	bindRes = bind(MainSocket, (SOCKADDR*)&service, sizeof(service));
	if (bindRes == SOCKET_ERROR)
	{
		printf("bind( ) failed with error %ld. Ending program\n", WSAGetLastError());
		goto server_cleanup_2;
	}

	// Listen on the Socket.
	ListenRes = listen(MainSocket, SOMAXCONN);
	if (ListenRes == SOCKET_ERROR)
	{
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		goto server_cleanup_2;
	}

	// Initialize all thread handles to NULL, to mark that they have not been initialized
	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
		ThreadHandles[Ind] = NULL;

	printf("Waiting for a client to connect...\n");

	for (Loop = 0; Loop < MAX_LOOPS; Loop++)
	{
		SOCKET AcceptSocket = accept(MainSocket, NULL, NULL);
		if (AcceptSocket == INVALID_SOCKET)
		{
			printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
			goto server_cleanup_3;
		}

		printf("Client Connected.\n");

		Ind = FindFirstUnusedThreadSlot();
		bool indec = true;
		while (Ind == NUM_OF_WORKER_THREADS) //no slot is available
		{
			
			
			if (indec)
			{


				//send the connected thread that it is denied connection
						////receving the data one time every loop//////
				char SendStrDenied[SEND_STR_SIZE];
				TransferResult_t SendRes;
				TransferResult_t RecvRes;
				char *AcceptedStr = NULL;

				RecvRes = ReceiveString(&AcceptedStr, AcceptSocket);


				if (RecvRes == TRNS_FAILED)
				{
					printf("Service socket error while reading, closing thread.\n");
					closesocket(AcceptSocket);
					return 1;
				}
				else if (RecvRes == TRNS_DISCONNECTED)
				{
					printf("Connection closed while reading, closing thread.\n");
					closesocket(AcceptSocket);
					return 1;
				}
				else
				{
					//printf("Got string : %s\n", AcceptedStr);

				}
				char *incomingPramterlist[5];
				parsInputToList(incomingPramterlist, AcceptedStr);
				if (STRINGS_ARE_EQUAL(incomingPramterlist[0], "CLIENT_REQUEST"))
				{
					printf("No slots available for client, dropping the connection.\n");
					strcpy(SendStrDenied, "SERVER_DENIED:");
					strcat(SendStrDenied, portNumberString);
					strcat(SendStrDenied, "\n");
					SendRes = SendString(SendStrDenied, AcceptSocket);
					if (SendRes == TRNS_FAILED)
					{
						printf("Service socket error while writing, closing thread.\n");
						closesocket(AcceptSocket);
						return 1;
					}

					Ind = FindFirstUnusedThreadSlot();

					if (Ind != NUM_OF_WORKER_THREADS)
					{
						strcpy(SendStrDenied, "SERVER_APPROVED");
						SendRes = SendString(SendStrDenied, AcceptSocket);
						if (SendRes == TRNS_FAILED)
						{
							printf("Service socket error while writing, closing thread.\n");
							closesocket(AcceptSocket);
							return 1;
						}
						strcpy(SendStrDenied, "SERVER_MAIN_MENU");
						SendRes = SendString(SendStrDenied, AcceptSocket);
						if (SendRes == TRNS_FAILED)
						{
							printf("Service socket error while writing, closing thread.\n");
							closesocket(AcceptSocket);
							return 1;
						}
						indec = false;

					}

				}
				free(AcceptedStr);

			}
		}
		//else
		//{
			ThreadInputs[Ind] = AcceptSocket; // shallow copy: don't close 
											  // AcceptSocket, instead close 
											  // ThreadInputs[Ind] when the
											  // time comes.
			ThreadHandles[Ind] = CreateThread(
				NULL,
				0,
				(LPTHREAD_START_ROUTINE)ServiceThread,
				&(ThreadInputs[Ind]),
				0,
				NULL
			);
		//}
	} // for ( Loop = 0; Loop < MAX_LOOPS; Loop++ )

server_cleanup_3:

	CleanupWorkerThreads();

server_cleanup_2:
	if (closesocket(MainSocket) == SOCKET_ERROR)
		printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());

server_cleanup_1:
	if (WSACleanup() == SOCKET_ERROR)
		printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

static int FindFirstUnusedThreadSlot()
{
	int Ind;

	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
	{
		if (ThreadHandles[Ind] == NULL)
			break;
		else
		{
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject(ThreadHandles[Ind], 0);

			if (Res == WAIT_OBJECT_0) // this thread finished running
			{
				CloseHandle(ThreadHandles[Ind]);
				ThreadHandles[Ind] = NULL;
				break;
			}
		}
	}

	return Ind;
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

static void CleanupWorkerThreads()
{
	int Ind;

	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
	{
		if (ThreadHandles[Ind] != NULL)
		{
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject(ThreadHandles[Ind], INFINITE);

			if (Res == WAIT_OBJECT_0)
			{
				closesocket(ThreadInputs[Ind]);
				CloseHandle(ThreadHandles[Ind]);
				ThreadHandles[Ind] = NULL;
				break;
			}
			else
			{
				printf("Waiting for thread failed. Ending program\n");
				return;
			}
		}
	}
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

//Service thread is the thread that opens for each successful client connection and "talks" to the client.
static DWORD ServiceThread(SOCKET *t_socket)
{
	char SendStr[SEND_STR_SIZE];
	//char SendStr2[SEND_STR_SIZE];
	BOOL Done = FALSE;
	TransferResult_t SendRes;
	TransferResult_t RecvRes;

	static HANDLE mutexCheckFileExistsHandle;
	BOOL release_res;
	DWORD wait_res;//for mutexOpcode

	BOOL an_error_occured = FALSE;
	mutexCheckFileExistsHandle = CreateMutex(
		NULL,   /* default security attributes */
		FALSE,	/* don't lock mutex immediately */
		"mutexCheckFileExists"); /* un-named */
	if (mutexCheckFileExistsHandle == NULL)
	{
		return an_error_occured = TRUE;/// goto Main_Cleanup_3;  need to make clean up!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	}

	////handels to event 2 players
	HANDLE Player_2_ready_event_handle;
	HANDLE Player_1_ready_event_handle;
	Player_2_ready_event_handle = Player_ready_event(P_EVENT_NAME[0]);
	Player_1_ready_event_handle = Player_ready_event(P_EVENT_NAME[1]);
	BOOL is_success;
	bool first_thread=false;
	bool second_thread = false;
	bool two_player_mode = false;
	int numOfPlayers;
	////cheek if workedddddd!!!!

	strcpy(SendStr, "Welcome to this server!");
	
	SendRes = SendString(SendStr, *t_socket);

	if (SendRes == TRNS_FAILED)
	{
		printf("Service socket error while writing, closing thread.\n");
		closesocket(*t_socket);
		return 1;
	}

	///////////// Definitions//////////
	char playerName[15];
	char otherPlayerName[15];

	char pathToFolder[] = "..\\GameSession.txt";
	char pathToTwoPlayerFolder[] = "..\\twoPlayerMode.txt";
	char *playerMove = NULL;
	char otherPlayerMove[15];
	///////////////////////////////////



	while (!Done)
	{
		////receving the data one time every loop//////
		char *AcceptedStr = NULL;

		RecvRes = ReceiveString(&AcceptedStr, *t_socket);
		

		if (RecvRes == TRNS_FAILED)
		{
			printf("Service socket error while reading, closing thread.\n");
			closesocket(*t_socket);
			return 1;
		}
		else if (RecvRes == TRNS_DISCONNECTED)
		{
			printf("Connection closed while reading, closing thread.\n");
			closesocket(*t_socket);
			return 1;
		}
		else
		{
			//printf("Got string : %s\n", AcceptedStr);
		}
		//parsing incoming string

		char *incomingPramterlist[5];

		//PARSING INPUT FUNCTION
		parsInputToList(incomingPramterlist, AcceptedStr);

		/////////////////////////////////////////////////////////////////////


		//initiating connection and requesting client to diplay main menu
		if (STRINGS_ARE_EQUAL(incomingPramterlist[0], "CLIENT_REQUEST"))
		{
			strcpy(playerName , incomingPramterlist[1]);
			strcpy(SendStr, "SERVER_APPROVED");
			SendRes = SendString(SendStr, *t_socket);

			if (SendRes == TRNS_FAILED)
			{
				printf("Service socket error while writing, closing thread.\n");
				closesocket(*t_socket);
				return 1;
			}
			// requesting client to diplay main menu
			strcpy(SendStr, "SERVER_MAIN_MENU");

		}
		///////////////////////////////////////////////////////////////////
		//two player mode
		if (STRINGS_ARE_EQUAL(incomingPramterlist[0], "CLIENT_VERSUS"))
		{
			numOfPlayers = FindFirstUnusedThreadSlot();
			if (numOfPlayers == 1)
			{
				Sleep(30000);

			}
			numOfPlayers = FindFirstUnusedThreadSlot();
			if (numOfPlayers == 1)
			{
				strcpy(SendStr, "SERVER_NO_OPPONENTS");
				SendRes = SendString(SendStr, *t_socket);

				if (SendRes == TRNS_FAILED)
				{
					printf("Service socket error while writing, closing thread.\n");
					closesocket(*t_socket);
					return 1;
				}
				strcpy(SendStr, "SERVER_MAIN_MENU");
			}
			else
			{
				errno_t retval;
				FILE *pFile;
				// mutex 
				wait_res = WaitForSingleObject(mutexCheckFileExistsHandle, INFINITE);
				//////needs to be protected!!!

				bool fileExists = doesFileExists(pathToFolder);

				if (fileExists)
				{
					//	WaitForSingleObject(Player_1_ready_event_handle, INFINITE);/// maybe 30 sec waiting
					printf("i am the second thread\n");
					ReadFromFile(pathToFolder, otherPlayerName);
					pFile = fopen(pathToFolder, "w");
					fprintf(pFile, playerName);
					fclose(pFile);
					release_res = ReleaseMutex(mutexCheckFileExistsHandle);
					//needs to be protected
					//SET EVENT
					printf("second thread ready");
					is_success = SetEvent(Player_2_ready_event_handle);
					/* Check if succeeded and handle errors */

					WaitForSingleObject(Player_1_ready_event_handle, INFINITE);/// maybe 30 sec waiting
					second_thread = true;
					strcpy(SendStr, "SERVER_INVITE:");
					strcat(SendStr, otherPlayerName);
					strcat(SendStr, "\n");

					//send client invite 
					SendRes = SendString(SendStr, *t_socket);

					if (SendRes == TRNS_FAILED)
					{
						printf("Service socket error while writing, closing thread.\n");
						closesocket(*t_socket);
						return 1;
					}
					//send a request for the cients move
					strcpy(SendStr, "SERVER_PLAYER_MOVE_REQUEST");

				}
				else
				{



					printf("i am the first thread\n");
					pFile = fopen(pathToFolder, "w");
					fclose(pFile);
					pFile = fopen(pathToFolder, "a");
					if (pFile == NULL)
					{
						perror("Error opening file.");
					}
					else
					{
						//	strcat(playerName, "\n");
						fprintf(pFile, playerName);
						fclose(pFile);
					}
					release_res = ReleaseMutex(mutexCheckFileExistsHandle);
					first_thread = true;
					printf("first thread waiting\n");
					//wait for other thread to be ready

					WaitForSingleObject(Player_2_ready_event_handle, INFINITE);/// maybe 30 sec waiting
					strcpy(SendStr, "SERVER_INVITE:");
					ReadFromFile(pathToFolder, otherPlayerName);
					strcat(SendStr, otherPlayerName);
					strcat(SendStr, "\n");

					//tell other thread that were fished opning
					is_success = SetEvent(Player_1_ready_event_handle);
					/* Check if succeeded and handle errors */

					two_player_mode = true;
					//send client invite 
					SendRes = SendString(SendStr, *t_socket);

					if (SendRes == TRNS_FAILED)
					{
						printf("Service socket error while writing, closing thread.\n");
						closesocket(*t_socket);
						return 1;
					}
					//send a request for the cients move
					strcpy(SendStr, "SERVER_PLAYER_MOVE_REQUEST");

				}



				//cheek if

			}
		}
		////////////////////////////////////////////////////////////////

		// one player mode
		if (STRINGS_ARE_EQUAL(incomingPramterlist[0], "CLIENT_CPU"))
		{
			srand(time(0));
			int randChoice = rand(RAND_MAX ) % 5;
			strcpy(otherPlayerMove, movesArray[randChoice]);
			strcpy(SendStr, "SERVER_PLAYER_MOVE_REQUEST");
			strcpy(otherPlayerName, "Server");
		}

		///////////////////////////////////////////////////////////////////
		if (STRINGS_ARE_EQUAL(incomingPramterlist[0], "CLIENT_PLAYER_MOVE"))
		{
			
			int *winner= (int*)malloc(sizeof(int));
			playerMove = incomingPramterlist[1];

			///////////////////
			errno_t retval;
			FILE *pFile;
			//////////////////

			//next part should only be in 2 player mode
			//both threads most wait here so they enter together
			if (first_thread == true)
			{
			printf("receved first thread move\n");
			//open a file and wright my move
				pFile = fopen(pathToFolder, "w");
				fclose(pFile);
				pFile = fopen(pathToFolder, "a");
				if (pFile == NULL)
				{
					perror("Error opening file.");
				}
				else
				{
				//	strcat(playerName, "\n");
					fprintf(pFile, playerMove);
					fclose(pFile);
				}


				// setting event we have wroten are move
				is_success = SetEvent(Player_1_ready_event_handle);
				/* Check if succeeded and handle errors */

				//wait for other player move
				WaitForSingleObject(Player_2_ready_event_handle, INFINITE);/// maybe 30 sec waiting

				//read a file and get other players move
				ReadFromFile(pathToFolder, otherPlayerMove);
				// this point has both player moves

			printf("receved first thread move and finshed waiting\n");
			}
			else if(second_thread == true)
			{
			printf("receved second thread move\n");
			//	second_thread = false;//not foresure here
				WaitForSingleObject(Player_1_ready_event_handle, INFINITE);/// maybe 30 sec waiting

				//read a file and get other players move:
				ReadFromFile(pathToFolder, otherPlayerMove);
				//wright my move:
				pFile = fopen(pathToFolder, "w");
				fprintf(pFile, playerMove);
				fclose(pFile);

				//this point has both player moves
				is_success = SetEvent(Player_2_ready_event_handle);
				/* Check if succeeded and handle errors */

			printf("receved second thread move and finshed waiting\n");
			}


			gameWinner(playerMove, otherPlayerMove, winner);
		printf("game winner calculated:%s is %d  and played %s other player played %s \n", playerName, *winner, playerMove, otherPlayerMove);

			strcpy(SendStr, "SERVER_GAME_RESULTS:");
			strcat(SendStr, otherPlayerName);
			strcat(SendStr, ";");
			strcat(SendStr, otherPlayerMove);
			strcat(SendStr, ";");
			strcat(SendStr, playerMove);
			strcat(SendStr, ";");

			if (*winner == 0) //tie
			{
				strcat(SendStr, "Tie!");
				strcat(SendStr, "\n");
			}
			else if (*winner == 1)//i won
			{
				strcat(SendStr, playerName);
				strcat(SendStr, "\n");
			}
			else if (*winner == 2)//you won
			{
				strcat(SendStr, otherPlayerName);
				strcat(SendStr, "\n");
			}

			SendRes = SendString(SendStr, *t_socket);
			if (SendRes == TRNS_FAILED)
			{
				printf("Service socket error while writing, closing thread.\n");
				closesocket(*t_socket);
				return 1;
			}
		printf("finshed sending results\n");

			Sleep(500);//??
			free(winner);

			strcpy(SendStr, "SERVER_GAME_OVER_MENU");
		printf("sending server game over menu\n");
		}
		//////////////////////////////////////////////////////////////////

		if (STRINGS_ARE_EQUAL(incomingPramterlist[0], "CLIENT_MAIN_MENU"))
		{
			strcpy(SendStr, "SERVER_MAIN_MENU");

			if (first_thread == true)
			{
				//delete file game session	
				// mutex 
				wait_res = WaitForSingleObject(mutexCheckFileExistsHandle, INFINITE);
				//////needs to be protected!!!
				if (remove(pathToFolder) == 0)
					printf("Deleted successfully");
				else
					printf("Unable to delete the file");
				release_res = ReleaseMutex(mutexCheckFileExistsHandle);

				is_success = SetEvent(Player_1_ready_event_handle);
				WaitForSingleObject(Player_2_ready_event_handle, INFINITE);/// maybe 30 sec waiting
				first_thread = false;
			
			}
			if (second_thread == true)
			{
				//wright to other thread that we want to quit
				// mutex 
				wait_res = WaitForSingleObject(mutexCheckFileExistsHandle, INFINITE);
				//////needs to be protected!!!
				bool fileExists = doesFileExists(pathToFolder);
				release_res = ReleaseMutex(mutexCheckFileExistsHandle);
				if (fileExists)
				{
					//wright to game session quit
					FILE *pFile;
					pFile = fopen(pathToFolder, "w");
					fprintf(pFile, "quit");
					fclose(pFile);
				}


				is_success = SetEvent(Player_2_ready_event_handle);
				WaitForSingleObject(Player_1_ready_event_handle, INFINITE);/// maybe 30 sec waiting
				second_thread = false;
			}

		}
	
		////////////////////////////////////////////////////////////////
		if (STRINGS_ARE_EQUAL(incomingPramterlist[0], "CLIENT_REPLAY"))
		{
			if (first_thread == true) 
			{
				char otherPlayerChoice[15];
				is_success = SetEvent(Player_1_ready_event_handle);
				WaitForSingleObject(Player_2_ready_event_handle, INFINITE);/// maybe 30 sec waiting

				//read from file if other dude quit: send the message that the other dude quit
				ReadFromFile(pathToFolder, otherPlayerChoice);
				if (STRINGS_ARE_EQUAL(otherPlayerChoice, "quit"))
				{
				//removing game session
					if (remove(pathToFolder) == 0)
						printf("Deleted successfully");
					else
						printf("Unable to delete the file");

					strcpy(SendStr, "SERVER_OPPONENT_QUIT:");
					strcat(SendStr, otherPlayerName);
					strcat(SendStr, "\n");

					SendRes = SendString(SendStr, *t_socket);
					if (SendRes == TRNS_FAILED)
					{
						printf("Service socket error while writing, closing thread.\n");
						closesocket(*t_socket);
						return 1;
					}
					strcpy(SendStr, "SERVER_MAIN_MENU");
					first_thread=false;

				}
				// else send move request
				else
				{
					strcpy(SendStr, "SERVER_PLAYER_MOVE_REQUEST");
					//waiting for player 2 to be ready after reading and delete file
					WaitForSingleObject(Player_2_ready_event_handle, INFINITE);/// maybe 30 sec waiting
					//delete the game session folder
					if (remove(pathToFolder) == 0)
						printf("Deleted successfully");
					else
						printf("Unable to delete the file");

				}
			}

			if (second_thread == true)
			{
				is_success = SetEvent(Player_2_ready_event_handle);
				WaitForSingleObject(Player_1_ready_event_handle, INFINITE);/// maybe 30 sec waiting
				//if file exists:send the move request both want to play
				bool fileExists = doesFileExists(pathToFolder);
				if (fileExists)
				{
					strcpy(SendStr, "SERVER_PLAYER_MOVE_REQUEST");
					//now ready to delete the game session
					is_success = SetEvent(Player_2_ready_event_handle);
				}
				// else other dude quit
				else
				{
					strcpy(SendStr, "SERVER_OPPONENT_QUIT:");
					strcat(SendStr, otherPlayerName);
					strcat(SendStr, "\n");

					SendRes = SendString(SendStr, *t_socket);
					if (SendRes == TRNS_FAILED)
					{
						printf("Service socket error while writing, closing thread.\n");
						closesocket(*t_socket);
						return 1;
					}
					strcpy(SendStr, "SERVER_MAIN_MENU");
					second_thread = false;
				}
			}
		}
		
		if (STRINGS_ARE_EQUAL(incomingPramterlist[0], "CLIENT_DISCONNECT"))
		{
			//disconnect the thread from the surver
			//send to the oponenent(send to other thread "SERVER_NO_OPPONENTS")
			
			free(AcceptedStr);
			shutdown(*t_socket, SD_SEND);
			char *AcceptedStr = NULL;

			RecvRes = ReceiveString(&AcceptedStr, *t_socket);
;			goto cleen_up;

		}
		//////////////////////////////////////////////////////////////////
		SendRes = SendString(SendStr, *t_socket);
		if (SendRes == TRNS_FAILED)
		{
			printf("Service socket error while writing, closing thread.\n");
			closesocket(*t_socket);
			return 1;
		}
		free(AcceptedStr);
	}



	cleen_up:
	printf("Connection ended.\n");
	closesocket(*t_socket);
	is_success = CloseHandle(Player_2_ready_event_handle);
	is_success = CloseHandle(Player_1_ready_event_handle);
	

	/* Check if succeeded and handle errors */
	return 0;
}

HANDLE Player_ready_event(char* P_EVENT_NAME)
{
	HANDLE Player_ready_event_handle;
	DWORD last_error;

	/* Get handle to event by name. If the event doesn't exist, create it */
	Player_ready_event_handle = CreateEvent(
		P_SECURITY_ATTRIBUTES, /* default security attributes */
		IS_MANUAL_RESET,       /* manual-reset event */
		IS_INITIALLY_SET,      /* initial state is non-signaled */
		P_EVENT_NAME);         /* name */
	/* Check if succeeded and handle errors */

	last_error = GetLastError();
	/* If last_error is ERROR_SUCCESS, then it means that the event was created.
	   If last_error is ERROR_ALREADY_EXISTS, then it means that the event already exists */

	return Player_ready_event_handle;
}



void gameWinner(char *movePlayerOne, char *movePlayerTwo, int *winner)
{
	if (STRINGS_ARE_EQUAL(movePlayerOne, movePlayerTwo))
	{
		*winner = 0;
	}
	else
	{
		if (STRINGS_ARE_EQUAL(movePlayerOne, "PAPER"))
		{
			if (STRINGS_ARE_EQUAL(movePlayerTwo, "ROCK") || STRINGS_ARE_EQUAL(movePlayerTwo, "SPOCK"))
			{
				*winner = 1;
			}
			else
			{
				*winner = 2;
			}
		}
		else if (STRINGS_ARE_EQUAL(movePlayerOne, "SCISSORS"))
		{
			if (STRINGS_ARE_EQUAL(movePlayerTwo, "PAPER") || STRINGS_ARE_EQUAL(movePlayerTwo, "LIZARD"))
			{
				*winner = 1;
			}
			else
			{
				*winner = 2;
			}
		}
		else if (STRINGS_ARE_EQUAL(movePlayerOne, "LIZARD"))
		{
			if (STRINGS_ARE_EQUAL(movePlayerTwo, "PAPER") || STRINGS_ARE_EQUAL(movePlayerTwo, "SPOCK"))
			{
				*winner = 1;
			}
			else
			{
				*winner = 2;
			}
		}
		else if (STRINGS_ARE_EQUAL(movePlayerOne, "SPOCK"))
		{
			if (STRINGS_ARE_EQUAL(movePlayerTwo, "ROCK") || STRINGS_ARE_EQUAL(movePlayerTwo, "SCISSORS"))
			{
				*winner = 1;
			}
			else
			{
				*winner = 2;
			}
		}
		else if (STRINGS_ARE_EQUAL(movePlayerOne, "ROCK"))
		{
			if (STRINGS_ARE_EQUAL(movePlayerTwo, "LIZARD") || STRINGS_ARE_EQUAL(movePlayerTwo, "SCISSORS"))
			{
				*winner = 1;
			}
			else
			{
				*winner = 2;
			}
		}
	}

}
