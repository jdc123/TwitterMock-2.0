/*
 * Jakub Cichon
 * Twitter Mock (Distributed and Replicated)
 * CS3254
 * Spring 2013
 */
 //Includes
 #include <stdio.h> //puts, sscanf
 #include <stdlib.h> //exit, atoi
 #include <arpa/inet.h> //htonl, inet_addr
 #include <sys/socket.h> //socket
 #include <string.h> //strlen, strchr
 #include <strings.h> //bzero
 #include <unistd.h> //close 
 #include <fcntl.h> //open
 #include <errno.h>
 #include <time.h> //time
 #include <pthread.h> //mutexes
 
 //Socket related constants
 #define PORT_LOW 1024 //Smallest "public" port number
 #define PORT_HIGH 65535 //Highest "public" port number
 #define IP_NUM_FIELDS 4 //Number of fields in an IP Address separated by .
 #define IP_LOW_END 0 //Lowest allowed value of an IP field
 #define IP_HIGH_END 255 //Highest allowed value of an IP field
 #define BACKLOG 1
 
 //Server application related constants
 //Basic
 #define USERNAME_SIZE 32
 #define MD5_SIZE 32
 #define TWEET_SIZE 140
 #define WHITE_SPACE_SIZE 1 
 #define NEWLINE_SIZE 1
 #define PADDING_CHAR ' '
 #define SEPARATOR "\t\n"
 #define SEPARATOR_CHAR '\t'
 #define TIMESTAMP_SIZE 10
 #define MAX_REQUEST_STRING_LENGTH 16
 #define MAX_UNIQUE_ID_LENGTH 32
 
 //Entries 
 #define CREDENTIALS_ENTRY_SIZE (USERNAME_SIZE + WHITE_SPACE_SIZE + MD5_SIZE + NEWLINE_SIZE)
 #define FOLLOWEES_ENTRY_SIZE (USERNAME_SIZE + WHITE_SPACE_SIZE + USERNAME_SIZE + NEWLINE_SIZE)
 #define TWEET_ENTRY_SIZE (USERNAME_SIZE + WHITE_SPACE_SIZE + TIMESTAMP_SIZE + WHITE_SPACE_SIZE + TWEET_SIZE + NEWLINE_SIZE)

 //Number of Entry fields
 #define NUM_FIELDS_CREDENTIALS 2 	//userName + md5
 #define NUM_FIELDS_FOLLOWEES 2		//userName + userName
 #define NUM_FIELDS_TWEETS 3 		//userName + timeStamp + tweet
 
 //Response/Request
 #define MAX_RESPONSE_SIZE (TWEET_ENTRY_SIZE) //Server --> Client (TWEET_ENTRY_SIZE is the longest entry)
 #define MAX_REQUEST_SIZE (MAX_UNIQUE_ID_LENGTH + WHITE_SPACE_SIZE + MAX_REQUEST_STRING_LENGTH + WHITE_SPACE_SIZE + USERNAME_SIZE + WHITE_SPACE_SIZE) //Client --> Server

 //Time
 #define TIMEOUT 5 //in seconds
 
 //Exit codes
 #define INVALID_ARGS 1
 #define INVALID_PORT_NUM 2
 #define INVALID_IP_ADDR 3
 #define SOCKET_FAILED 4
 #define BIND_FAILED 5
 #define LISTEN_FAILED 6
 #define ACCEPT_FAILED 7
 #define SEND_FAILED 8
 #define RECV_FAILED 9
 #define FGETS_FAILED 10
 #define FOPEN_FAILED 11
 #define FWRITE_FAILED 12
 #define FCLOSE_FAILED 13
 #define SNPRINTF_FAILED 14
 #define FSEEK_FAILED 15
 #define CLOSE_FAILED 16
 #define MALLOC_FAILED 17
 #define INVALID_TOKENS 18
 #define REALLOC_FAILED 19
 #define STRCHR_FAILED 20
 #define CONNECT_FAILED 21
 #define INVALID_SELECTION 22
 #define FCNTL_FAILED 23
 
 //Filenames
 #define CREDENTIALS_FILE "userCredentials.txt"
 #define TWEETS_FILE "tweets.txt"
 #define FOLLOWEES_FILE "followees.txt"
 #define LOG_FILE "log.txt"
 
 //Mutexes
 pthread_mutex_t credentialsMutex = PTHREAD_MUTEX_INITIALIZER;
 pthread_mutex_t followeesMutex = PTHREAD_MUTEX_INITIALIZER;
 pthread_mutex_t tweetsMutex = PTHREAD_MUTEX_INITIALIZER;
 pthread_mutex_t logMutex = PTHREAD_MUTEX_INITIALIZER;
 
 //flags
 #define WRITE_FLAGS (O_RDWR)
 #define APPEND_FLAGS (O_APPEND)

struct connection_bundle {
	int clientSock;
};  

int getPortNumber(char* portString);
int addTweet(char uname[USERNAME_SIZE], char time[TIMESTAMP_SIZE], char tweet[TWEET_SIZE]);
int addFollowee(char uname[USERNAME_SIZE], char followee[USERNAME_SIZE]);
int removeFollowee(char uname[USERNAME_SIZE], char followee[USERNAME_SIZE]);
int verifyRegistration(char uname[USERNAME_SIZE], char pwd[MD5_SIZE]);
int verifyLogin(char uname[USERNAME_SIZE], char pwd[MD5_SIZE]);
void tokenize(char *allTokens[], char *input, char *delimeter);
void listTweets(char uname[USERNAME_SIZE], char **followees, int clientSock, int size);
void listFollowees(char uname[USERNAME_SIZE], int clientSock);
void loadFollowees(char uname[USERNAME_SIZE], char ***followees, int *size);
void listOthers(char uname[USERNAME_SIZE], char **followees, int clientSock, int size);
int inArray(char uname[USERNAME_SIZE], char *users[], int size);
void validateArguments(char *arr[], int num);
int removeEntry(char uname[USERNAME_SIZE], char *fileName, int entrySize, int numOfFields, char *formatString, int compIndex, pthread_mutex_t *mutex);

//Modified for distribution
void* getInput(void *args);
void sendResponse(char *allTokens[], char response[MAX_RESPONSE_SIZE], struct connection_bundle *connBundle, char request[MAX_REQUEST_SIZE]);

//Distributed stuff added
void initConnectionBundle(struct connection_bundle *cBundle);
int createSockets(struct sockaddr_in *other, int port);
void setPrimary();
int establishConnection(int otherSock, struct sockaddr_in* other);
int receiveAcknowledgement(int otherSock, char response[MAX_RESPONSE_SIZE]);
void handleUpdate(char *allTokens[], char response[MAX_RESPONSE_SIZE]);
void handleRead(char *allTokens[], char response[MAX_RESPONSE_SIZE], int clientSock);
void notifyBackups(char request[MAX_REQUEST_SIZE]);
int sendToBackup(int *portGlobal, pthread_mutex_t *mut, char request[MAX_REQUEST_SIZE]);
void addLog(char request[MAX_REQUEST_SIZE]);
int checkLog(char request[MAX_REQUEST_SIZE]);
void conductElections(char request[MAX_REQUEST_SIZE], int port);
int selectCandidate(int myPort, int otherPort1, int otherPort2);
int killPort(int target);

//Global server information
int MY_PORT;
int OTHER_PORT1;
int OTHER_PORT2;
int IS_PRIMARY = 0;
int PRIMARY = -1;

//Server information mutexes
pthread_mutex_t myPortMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t other1Mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t other2Mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t isPrimaryMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t updateMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t electionMutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[]){
	//Variable definitions that are socket related
	int serverSock, clientSock, sizeOfSockAddr;
	struct sockaddr_in server, client;
	if(argc != 4) //./server myPort otherPort1 otherPort2
	{
		perror("Wrong number of arguments");
		exit(INVALID_ARGS);
	}
	
	//Port numbers from command line
	MY_PORT = getPortNumber(argv[1]);
	OTHER_PORT1 = getPortNumber(argv[2]);
	OTHER_PORT2 = getPortNumber(argv[3]);
	
	//Create Socket for the server
	if((serverSock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("Failed to create a socket");
		exit(SOCKET_FAILED);
	}
	puts("Created server socket.");
	
	//Prepare sockAddr struct
	server.sin_family = AF_INET;
	server.sin_port = htons((short)MY_PORT); 
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	
	//Bind socket to  port
	if(bind(serverSock, (struct sockaddr *)&server, sizeof(server)) == -1){
		perror("Failed to bind a socket to a port");
		exit(BIND_FAILED);
	}
	puts("Binded a socket to port.");

	//Who is the primary?
	setPrimary();
	if(IS_PRIMARY)
	{
		printf("Primary: %i\n", MY_PORT);
	}
	else
	{
		printf("Back-up: %i\n", MY_PORT);
	} 
	while(1)
	{
		//Listen on a socket
		if(listen(serverSock, BACKLOG) == -1){
			perror("Failed listening on a socket.");
			exit(LISTEN_FAILED);
		}
		puts("Listening on a socket.");
		
		//Accept client connection
		sizeOfSockAddr = sizeof(struct sockaddr_in);
		if((clientSock = accept(serverSock, (struct sockaddr *)&client, (socklen_t*)&sizeOfSockAddr)) == -1)
		{
			perror("Failed to accept a connection.");
			exit(ACCEPT_FAILED);
		}
		printf("My Client: %i\n", clientSock);
		puts("Connection established with a client.");
		//Receive request
		pthread_t tid1;
		//pthread_create(&tid1, NULL, getInput, &connBundle);
		//1. Request: The front end issues the request, containing a unique identifier, to the primary replica manager.
		pthread_create(&tid1, NULL, getInput, (void*)(intptr_t)clientSock);
		
	}
	return 0;
}

void* getInput(void *args){
	//Receives a client socket and calls functions that send the response back to the client 
	//or closes the socket if no data is received 
	int clientSock = (int)(intptr_t)args;
	struct connection_bundle connBundle;
	initConnectionBundle(&connBundle);
	
	//Defining and clearing request and response buffers
	char request[MAX_REQUEST_SIZE];
	bzero(request, MAX_REQUEST_SIZE);
	char response[MAX_RESPONSE_SIZE];
	bzero(response, MAX_RESPONSE_SIZE);
	
	//Receive client's request
	int readSize;
	if((readSize = recv(clientSock, request, MAX_REQUEST_SIZE, 0)) < 0)
	{
		perror("Recv failed.");
		exit(RECV_FAILED);
	}
	//If client did not send anything
	else if(readSize == 0)
	{
		puts("Client disconnected.");
		if(close(clientSock) == -1)
		{
			perror("Close failed.");
			exit(CLOSE_FAILED);
		}
	}
	else
	{
		char *allTokens[MAX_REQUEST_SIZE];
		
		//Store a copy of the request for ater use in sendResponse function
		char reqCopy[MAX_REQUEST_SIZE];
		if(snprintf(reqCopy, MAX_REQUEST_SIZE, "%s", request) < 0)
		{
			perror("Snprintf failed.");
			exit(SNPRINTF_FAILED);
		}
		
		printf("Request: %s", request);
		//remove preceding whitespace
		char *filteredRequest;
		filteredRequest = strchr(request, SEPARATOR_CHAR);
		filteredRequest++; 
		
		//Tokenize request
		tokenize(allTokens, filteredRequest, SEPARATOR);
		connBundle.clientSock = clientSock;
		sendResponse(allTokens, response, &connBundle, reqCopy);
		
		//Free up dynamically allocated tokens
		for(int i = 0; allTokens[i]; i++){
			free(allTokens[i]);
		}
			
		//Clear buffers
		bzero(request, MAX_REQUEST_SIZE);
		bzero(response, MAX_RESPONSE_SIZE);
		
		//Close socket
		if(close(clientSock) == -1)
		{
			perror("Close failed.");
			exit(CLOSE_FAILED);
		}
		puts("Closed a socket.");
	}
	return NULL;
}
int inArray(char uname[USERNAME_SIZE], char *users[], int size){
	//Returns 1 if user is found in the array. Else, returns 0
	for(int i = 0; i < size; i++)
	{
		if((strncmp(uname, users[i], strlen(uname)) == 0) && (strlen(uname) == strlen(users[i])))
		{
			return 1;
		} 
	}
	return 0;
}
int addTweet(char uname[USERNAME_SIZE], char time[TIMESTAMP_SIZE], char tweet[TWEET_SIZE]){
	//Appends uname, current server time and the tweet to the end of the tweet file
	FILE * fp;
	pthread_mutex_lock(&tweetsMutex);
	if ((fp = fopen(TWEETS_FILE, "a")) == NULL)
	{
		perror("Couldn't open file");
		exit(FOPEN_FAILED);
	}
	if(fprintf(fp, "%s\t%s\t%s\n", uname, time, tweet) < 0) // zero objects written to file
	{
		perror("Couldn't write to file");
		exit(FWRITE_FAILED);
	}
	if(fclose(fp) == EOF)
	{
		perror("Close failed.");
		exit(FCLOSE_FAILED);
	}
	pthread_mutex_unlock(&tweetsMutex);
	return 1;
}
int addFollowee(char uname[USERNAME_SIZE], char followee[USERNAME_SIZE]){
	//Appends uname-followee pair to the file containing all such pairs.
	FILE * fp;
	pthread_mutex_lock(&followeesMutex);
	if ((fp = fopen(FOLLOWEES_FILE, "a")) == NULL)
	{
		perror("Couldn't open file");
		exit(FOPEN_FAILED);
	}
	if(fprintf(fp, "%s\t%s\n", uname, followee) < 0) // zero objects written to file
	{
		perror("Couldn't write to file");
		exit(FWRITE_FAILED);
	}
	if(fclose(fp) == EOF)
	{
		perror("Close failed.");
		exit(FCLOSE_FAILED);
	}
	pthread_mutex_unlock(&followeesMutex);
	return 1;
}
int removeFollowee(char uname[USERNAME_SIZE], char followee[USERNAME_SIZE]){
	//Fills the entry containing uname-followee pair with whitespace
	FILE * fp;
	pthread_mutex_lock(&followeesMutex);
	if ((fp = fopen(FOLLOWEES_FILE, "r+")) == NULL)
	{
		perror("Couldn't open file");
		exit(FOPEN_FAILED);
	}
	char cmpStr[FOLLOWEES_ENTRY_SIZE];
	int size = strlen(uname) + strlen(followee) + 2; //2 is for \n and \t

	if(snprintf(cmpStr, size, "%s\t%s\n", uname, followee) < 0) // zero objects written to file
	{
		perror("Snprintf failed.");
		exit(SNPRINTF_FAILED);
	}
	char followees_entry[FOLLOWEES_ENTRY_SIZE];
	char buff[size];
	bzero(buff, size);
	int flag = 0;
	while(fgets(followees_entry, FOLLOWEES_ENTRY_SIZE, fp))
	{
		int diff = strncmp(followees_entry, cmpStr, strlen(cmpStr));
		if(diff == 0) // Match found
		{
			long offset = size;
			if(fseek(fp, -offset, SEEK_CUR) == (off_t)-1)
			{
				perror("Fseek failed.");
				exit(FSEEK_FAILED);
			}
			for(int i = 0; i < size; i++)
			{
				buff[i] = PADDING_CHAR;
			}
			buff[(size-1)] = '\0';
			if(fprintf(fp, "%s", buff) < 0) // zero objects written to file
			{
				perror("Couldn't write to file");
				exit(FWRITE_FAILED);
			}	
			flag = 1;
			break;
		}
	}
	if(fclose(fp) == EOF)
	{
		perror("Close failed.");
		exit(FCLOSE_FAILED);
	}
	pthread_mutex_unlock(&followeesMutex);
	if(flag) return 1;
	else return 0;
}
int removeEntry(char uname[USERNAME_SIZE], char *fileName, int entrySize, int numOfFields, char *formatString, int compIndex, pthread_mutex_t *mutex){
	//Generic function that ovewrites a line with whitespace in a passed in file matching the specified condition below
	//Returns flag: 0 on failure and >0 on success
	FILE * fp;
	int flag = 0;
	pthread_mutex_lock(mutex);
	if ((fp = fopen(fileName, "r+")) == NULL)
	{
		perror("Couldn't open file");
		exit(FOPEN_FAILED);
	}
	char oneLine[entrySize];
	char buff[entrySize];
	bzero(buff, entrySize);
	while(fgets(oneLine, entrySize, fp))
	{
		char* entryTok[numOfFields+1];
		tokenize(entryTok, oneLine, SEPARATOR);
		if(entryTok[compIndex]){
			int diff = strncmp(uname, entryTok[compIndex], strlen(uname));
			if(diff == 0 && (strlen(uname) == strlen(entryTok[compIndex]))) // Match found
			{
				long offset = 0;
				for(int i = 0; entryTok[i]; i++)
				{
					offset += (strlen(entryTok[i]) + 1); // 1 being either the tab or newline char that follows
				}
				if(fseek(fp, -offset, SEEK_CUR) == (off_t)-1)
				{
					perror("Fseek failed.");
					exit(FSEEK_FAILED);
				}
				for(int i = 0; i < offset; i++)
				{
					buff[i] = PADDING_CHAR;
				}
				buff[(offset-1)] = '\0';
				if(fprintf(fp, "%s", buff) < 0) // zero objects written to file
				{
					perror("Couldn't write to file");
					exit(FWRITE_FAILED);
				}	
				flag++;
				bzero(buff, entrySize);
			}
		}
		for(int i = 0; entryTok[i]; i++)
		{
			free(entryTok[i]);
		}
		bzero(oneLine, entrySize);
	}
	
	if(fclose(fp) == EOF)
	{
		perror("Close failed.");
		exit(FCLOSE_FAILED);
	}
	pthread_mutex_unlock(mutex);
	if(flag) return 1;
	else return 0;
	 
}
int verifyRegistration(char uname[USERNAME_SIZE], char pwd[MD5_SIZE]){
	//Checks if the user exists in the credentials file. If he does, then 0 is returned. If he doesn't then she is added.
	//Entry is appended to the end of file
	FILE * fp;
	pthread_mutex_lock(&credentialsMutex);
	if ((fp = fopen(CREDENTIALS_FILE, "a+")) == NULL)
	{
		perror("Couldn't open file");
		exit(FOPEN_FAILED);
	}
	char cmpStr[CREDENTIALS_ENTRY_SIZE];
	int size = strlen(uname) + strlen(pwd) + 2; //2 is for \n and \t
	if(snprintf(cmpStr, size, "%s\t%s\n", uname, pwd) < 0) // zero objects written to file
	{
		perror("Snprintf failed.");
		exit(SNPRINTF_FAILED);
	}
	char credentials_entry[CREDENTIALS_ENTRY_SIZE];
	int exists = 0;
	while(fgets(credentials_entry, CREDENTIALS_ENTRY_SIZE, fp))
	{
		int diff = strncmp(credentials_entry, cmpStr, strlen(uname)+1);
		if(diff == 0) //User already exists in the database
		{
			exists = 1;
		}
	}
	if(exists == 0) // no matches found
	{
		if(fprintf(fp, "%s\t%s\n", uname, pwd) < 0) // zero objects written to file
		{
			perror("Couldn't write to file");
			exit(FWRITE_FAILED);
		}
		if(fclose(fp) == EOF)
		{
			perror("Close failed.");
			exit(FCLOSE_FAILED);
		}
		pthread_mutex_unlock(&credentialsMutex);
		return 1; //Successfully added a user
	}
	else //match exists
	{
		if(fclose(fp) == EOF)
		{
			perror("Close failed.");
			exit(FCLOSE_FAILED);
		}
		pthread_mutex_unlock(&credentialsMutex);
		return 0; //Failed to register a user
	}
	
}
int verifyLogin(char uname[USERNAME_SIZE], char pwd[MD5_SIZE]){
	//Checks if the user exists in the credentials file. If he does exist, 1 is returned, else 0.
	FILE * fp;
	pthread_mutex_lock(&credentialsMutex);
	if ((fp = fopen(CREDENTIALS_FILE, "r")) == NULL)
	{
		perror("Couldn't open file");
		exit(FOPEN_FAILED);
	}
	char cmpStr[CREDENTIALS_ENTRY_SIZE];
	int size = strlen(uname) + strlen(pwd) + 2; //2 is for \n and \t
	if(snprintf(cmpStr, size, "%s\t%s\n", uname, pwd) < 0) // zero objects written to file
	{
		perror("Snprintf failed.");
		exit(SNPRINTF_FAILED);
	}
	char credentials_entry[CREDENTIALS_ENTRY_SIZE];
	while(fgets(credentials_entry, CREDENTIALS_ENTRY_SIZE, fp))
	{
		int diff = strncmp(credentials_entry, cmpStr, strlen(cmpStr));
		if(diff == 0) // Match found
		{
			if(fclose(fp) == EOF)
			{
				perror("Close failed.");
				exit(FCLOSE_FAILED);
			}
			pthread_mutex_unlock(&credentialsMutex);
			return 1; //Match found
		}
	}
	if(fclose(fp) == EOF)
	{
		perror("Close failed.");
		exit(FCLOSE_FAILED);
	}
	pthread_mutex_unlock(&credentialsMutex);
	return 0; //Didn't find a match
}       
void listFollowees(char uname[USERNAME_SIZE], int clientSock){
	//Sends a message containing usernames of uname's followees (one at a time) to the clientSock
	FILE * fp;
	pthread_mutex_lock(&followeesMutex);
	if ((fp = fopen(FOLLOWEES_FILE, "r")) == NULL)
	{
		perror("Couldn't open file");
		exit(FOPEN_FAILED);
	}
	char oneLine[FOLLOWEES_ENTRY_SIZE];
	bzero(oneLine, FOLLOWEES_ENTRY_SIZE);
	while(fgets(oneLine, FOLLOWEES_ENTRY_SIZE, fp))
	{
		char* followeeEntryTok[NUM_FIELDS_FOLLOWEES+1];
		tokenize(followeeEntryTok, oneLine, SEPARATOR);
		//if followeeEntryTok[0] and followeeEntryTok[1] exist, and followeeEntryTok[0] == uname
		if(followeeEntryTok[0] && followeeEntryTok[1] && strncmp(uname, followeeEntryTok[0], strlen(uname)) == 0 && strlen(uname) == strlen(followeeEntryTok[0]))
		{
			char temp[USERNAME_SIZE+1];
			bzero(temp, USERNAME_SIZE+1);
			if(snprintf(temp, USERNAME_SIZE+1, "%s\n", followeeEntryTok[1]) < 0)
			{
				perror("Snprintf failed.");
				exit(SNPRINTF_FAILED);
			}
			if(send(clientSock, temp, strlen(temp), 0) == -1)
			{
				perror("Send Failed.");
				exit(SEND_FAILED);
			}
			
		}
		for(int i = 0; followeeEntryTok[i]; i++)
		{
			free(followeeEntryTok[i]);
		}
		bzero(oneLine, FOLLOWEES_ENTRY_SIZE);
	}
	if(fclose(fp) == EOF)
	{
		perror("Close failed.");
		exit(FCLOSE_FAILED);
	}
	pthread_mutex_unlock(&followeesMutex);
}
void loadFollowees(char uname[USERNAME_SIZE], char ***followees, int *size){
	//Store all the usernames that uname is following (excluding his own uname) into followees
	FILE * fp;
	pthread_mutex_lock(&followeesMutex);
	if ((fp = fopen(FOLLOWEES_FILE, "r")) == NULL)
	{
		perror("Couldn't open file");
		exit(FOPEN_FAILED);
	}
	char oneLine[FOLLOWEES_ENTRY_SIZE];
	bzero(oneLine, FOLLOWEES_ENTRY_SIZE);
	while(fgets(oneLine, FOLLOWEES_ENTRY_SIZE, fp))
	{
		char* followeeEntryTok[NUM_FIELDS_FOLLOWEES+1];
		tokenize(followeeEntryTok, oneLine, SEPARATOR);
		//if followeeEntryTok[0] and followeeEntryTok[1] exist, and followeeEntryTok[0] == uname
		if(followeeEntryTok[0] && followeeEntryTok[1]
		&& strncmp(uname, followeeEntryTok[0], strlen(uname)) == 0 && (strlen(uname) == strlen(followeeEntryTok[0])))
		{
			//malloc entry
			char *entry = (char*)malloc(FOLLOWEES_ENTRY_SIZE*sizeof(char));
			if(entry == NULL)
			{
				perror("Malloc failed.");
				exit(MALLOC_FAILED);
			}
			if(snprintf(entry, FOLLOWEES_ENTRY_SIZE, "%s", followeeEntryTok[1]) < 0)
			{
				perror("Snprintf failed.");
				exit(SNPRINTF_FAILED);
			}
			//realloc array
			(*followees) = (char **)realloc((*followees), (*size)*sizeof(char*));
			if((*followees) == NULL)
			{
				perror("Realloc failed.");
				exit(REALLOC_FAILED);
			}
			(*followees)[(*size)-1] = entry;
			(*size)++;
		}
		for(int i = 0; followeeEntryTok[i]; i++)
		{
			free(followeeEntryTok[i]);
		}
		bzero(oneLine, FOLLOWEES_ENTRY_SIZE);
		
	}
	if(fclose(fp) == EOF)
	{
		perror("Close failed.");
		exit(FCLOSE_FAILED);
	}
	pthread_mutex_unlock(&followeesMutex);
}
void listOthers(char uname[USERNAME_SIZE], char **followees, int clientSock, int size){
	//Sends all usernames that uname is not following (excluding his own uname) to the clientSock
	
	FILE * fp;
	pthread_mutex_lock(&credentialsMutex);
	if ((fp = fopen(CREDENTIALS_FILE, "r")) == NULL)
	{
		perror("Couldn't open file");
		exit(FOPEN_FAILED);
	}
	char oneLine[CREDENTIALS_ENTRY_SIZE];
	bzero(oneLine, CREDENTIALS_ENTRY_SIZE);
	while(fgets(oneLine, CREDENTIALS_ENTRY_SIZE, fp))
	{
		char* credentialsEntryTok[NUM_FIELDS_CREDENTIALS+1];
		tokenize(credentialsEntryTok, oneLine, SEPARATOR);
		//puts(credentialsEntryTok[0]);
		//if credentialsEntryTok[0] exists, and is not in uname's followees and is not uname
		if(credentialsEntryTok[0] && !inArray(credentialsEntryTok[0], followees, size) && !(strncmp(uname, credentialsEntryTok[0], strlen(uname)) == 0 && (strlen(uname) == strlen(credentialsEntryTok[0]))))
		 {
			char temp[USERNAME_SIZE+1];
			bzero(temp, USERNAME_SIZE+1);
			if(snprintf(temp, USERNAME_SIZE+1, "%s\n", credentialsEntryTok[0]) < 0)
			{
				perror("Snprintf failed.");
				exit(SNPRINTF_FAILED);
			}
			if(send(clientSock, temp, strlen(temp), 0) == -1)
			{
				perror("Send Failed.");
				exit(SEND_FAILED);
			}
		}
		
		for(int i = 0; credentialsEntryTok[i]; i++)
		{
			free(credentialsEntryTok[i]);
		}
		bzero(oneLine, CREDENTIALS_ENTRY_SIZE);
	}
	if(fclose(fp) == EOF)
	{
		perror("Close failed.");
		exit(FCLOSE_FAILED);
	}
	pthread_mutex_unlock(&credentialsMutex);
}
void listTweets(char uname[USERNAME_SIZE], char **followees, int clientSock, int size){
	//sends all of uname's tweets as well as all tweets of followees to the clientSock
	FILE * fp;
	pthread_mutex_lock(&tweetsMutex);
	if ((fp = fopen(TWEETS_FILE, "r")) == NULL)
	{
		perror("Couldn't open file");
		exit(FOPEN_FAILED);
	}
	char oneLine[TWEET_ENTRY_SIZE];
	bzero(oneLine, TWEET_ENTRY_SIZE);
	while(fgets(oneLine, TWEET_ENTRY_SIZE, fp))
	{
		char* tweetsEntryTok[NUM_FIELDS_TWEETS+1];
		tokenize(tweetsEntryTok, oneLine, SEPARATOR);
		//if author of a tweet is in uname's followees array OR author is uname
		if(tweetsEntryTok[0] && (inArray(tweetsEntryTok[0], followees, size) ||
		 (strncmp(uname, tweetsEntryTok[0], strlen(uname)) == 0 && strlen(uname) == strlen(tweetsEntryTok[0]))))
		{
			char temp[TWEET_ENTRY_SIZE];
			bzero(temp, TWEET_ENTRY_SIZE);
			if(snprintf(temp, TWEET_SIZE+3, "%s\t%s\t%s\n", tweetsEntryTok[0], tweetsEntryTok[1], tweetsEntryTok[2]) < 0)
			{
				perror("Snprintf failed.");
				exit(SNPRINTF_FAILED);
			}
			if(send(clientSock, temp, strlen(temp), 0) == -1)
			{
				perror("Send Failed.");
				exit(SEND_FAILED);
			}
		}
		
		for(int i = 0; tweetsEntryTok[i]; i++)
		{
			free(tweetsEntryTok[i]);
		}
		bzero(oneLine, TWEET_ENTRY_SIZE);
	}
	if(fclose(fp) == EOF)
	{
		perror("Close failed.");
		exit(FCLOSE_FAILED);
	}
	pthread_mutex_unlock(&tweetsMutex);
}
int getPortNumber(char* portString){
//Converts port number from string and check if it's valid (taken from my HW#7 from CS239)
	int port = atoi(portString);
	if(port < PORT_LOW || port > PORT_HIGH)
	{
		perror("Invalid port number");
		exit(INVALID_PORT_NUM);
	}
	return port;
}
void tokenize(char *allTokens[], char *input, char* delimeter){
	//Tokenize user input
	char *p = NULL;	
	int ind = 0;	
	for(p = strtok(input, delimeter); p; p = strtok(NULL, delimeter))
	{
		char* oneToken = (char*)malloc(strlen(p)+1);
		if(oneToken == NULL){
			perror("Failed to malloc.");
			exit(MALLOC_FAILED);
		}
		strncpy(oneToken, p, strlen(p)+1);
		allTokens[ind] = oneToken;
		ind++;
	}
	
	allTokens[ind] = 0;
}
void sendResponse(char *allTokens[], char response[MAX_RESPONSE_SIZE], struct connection_bundle *connBundle, char request[MAX_REQUEST_SIZE]){
	//Selects a passed in statement and appropriately assigns a return message
	//General mechanism for message passing between server and client
	int clientSock = connBundle->clientSock;
	int isPrimary = IS_PRIMARY;
	
	/*2. Coordination: The primary takes each request atomically, in the order in which it
	receives it. It checks the unique identifier, in case it has already executed the request,
	and if so it simply resends the response.*/	
	//If request is an UPDATE
	if((strncmp(allTokens[0], "REGISTER", strlen(allTokens[0])) == 0) ||
		(strncmp(allTokens[0], "ADD_TWEET", strlen(allTokens[0])) == 0) ||
		(strncmp(allTokens[0], "ADD_FOLLOWEE", strlen(allTokens[0])) == 0) || 
		(strncmp(allTokens[0], "REMOVE_FOLLOWEE", strlen(allTokens[0])) == 0) ||
		(strncmp(allTokens[0], "DELETE_ACCOUNT", strlen(allTokens[0])) == 0)){
			//check log
			int found = checkLog(request); //stores only updates
			if(found){
				if(snprintf(response, MAX_RESPONSE_SIZE, "%i\n", found) < 0)
				{
					perror("Snprintf failed.");
					exit(SNPRINTF_FAILED);
				}
				//Make sure the back-ups have updated logs
				if(isPrimary)
				{
					notifyBackups(request);
				}
				if(send(clientSock, response, MAX_RESPONSE_SIZE, 0) == -1)
				{
					perror("Send Failed.");
				}
			}
			else
			{
				//3. Execution: The primary executes the request and stores the response.
				//Execute update
				pthread_mutex_lock(&updateMutex);
				handleUpdate(allTokens, response);
				//Log Response
				addLog(request);
				/*4. Agreement: If the request is an update, then the primary sends the updated state,
					the response and the unique identifier to all the backups. The backups send an
					acknowledgement.*/
				if(isPrimary)
				{
					notifyBackups(request);
				}
				/*5. Response: The primary responds to the front end, which hands the response back to the client.*/
				if(send(clientSock, response, MAX_RESPONSE_SIZE, 0) == -1)
				{
					perror("Send Failed.");
					//exit(SEND_FAILED);
				}
				pthread_mutex_unlock(&updateMutex);
				
				
			}	  
		}
	else if((strncmp(allTokens[0], "LOGIN", strlen(allTokens[0])) == 0) ||
			(strncmp(allTokens[0], "LIST_FOLLOWEES", strlen(allTokens[0])) == 0) ||
			(strncmp(allTokens[0], "LIST_OTHERS", strlen(allTokens[0])) == 0) ||
			(strncmp(allTokens[0], "LIST_TWEETS", strlen(allTokens[0])) == 0)){
			//Execute read & respond
			handleRead(allTokens, response, clientSock);
			
	}
	else if((strncmp(allTokens[0], "GET_PRIMARY", strlen(allTokens[0])) == 0))
	{	
		pthread_mutex_lock(&electionMutex);
		int target = (int)atoi(allTokens[1]);
		printf("Target: %s", allTokens[1]);
		puts(request);
		int killed = killPort(target);
		if(killed){
			int myPort = MY_PORT;
			int otherPort1 = OTHER_PORT1;
			int otherPort2 = OTHER_PORT2;
			
			//make choice
			int candidate = selectCandidate(myPort, otherPort1, otherPort2);
			char electoralRequest[MAX_REQUEST_SIZE];
			pthread_mutex_lock(&isPrimaryMutex);
			PRIMARY = candidate;
			if(candidate == myPort)
			{
				IS_PRIMARY = 1;
			}
			pthread_mutex_unlock(&isPrimaryMutex);
			if(snprintf(electoralRequest, MAX_REQUEST_SIZE, "dd\tAPPOINT\t%i", candidate) < 0)
			{
				perror("Snprintf failed.");
				exit(SNPRINTF_FAILED);
			}
			if(otherPort1 != -1)
			{
				//Send candidate
				conductElections(electoralRequest, otherPort1);
			}
			if(otherPort2 != -1)
			{
				conductElections(electoralRequest, otherPort2);
			}
			if(snprintf(response, MAX_RESPONSE_SIZE, "%i\n", candidate) < 0)
			{
				perror("Snprintf failed.");
				exit(SNPRINTF_FAILED);
			}
		}
		else
		{
			//already dead
			int primary = PRIMARY;
			if(primary == -1)
			{
				puts("Invalid primary");
			}
			if(snprintf(response, MAX_RESPONSE_SIZE, "%i\n", primary) < 0)
			{
				perror("Snprintf failed.");
				exit(SNPRINTF_FAILED);
			}
		}
		printf("New leader: %s\n", response);
		if(send(clientSock, response, MAX_RESPONSE_SIZE, 0) == -1)
		{
			perror("Send Failed.");
		}
		pthread_mutex_unlock(&electionMutex);
	}
	else if((strncmp(allTokens[0], "APPOINT", strlen(allTokens[0])) == 0))
	{
		int newPort = atoi(allTokens[1]);
		pthread_mutex_lock(&myPortMutex);
		int myPort = MY_PORT;
		pthread_mutex_lock(&myPortMutex);
		pthread_mutex_lock(&isPrimaryMutex);
		PRIMARY = newPort;
		if(myPort == PRIMARY)
		{
			IS_PRIMARY = 1;
		}
		pthread_mutex_unlock(&isPrimaryMutex);
	}
	else{
			exit(INVALID_SELECTION);
		}
}
void validateArguments(char *arr[], int num){
	//Checks if any arguments from index 0 to num (excluding num) are NULL
	for(int i = 0; i < num; i++)
	{
		if(arr[i] == NULL){ exit(INVALID_ARGS); }
	}
}
void initConnectionBundle(struct connection_bundle *cBundle){
	//Initiates the connection bundle
	//cBundle->clientSock =  cSock;
	cBundle->clientSock = -1;
	//cBundle->otherSock1 = -1;
	//cBundle->otherSock2 = -1;
}
int createSockets(struct sockaddr_in *other, int port){
	//If Primary, prepare the sockets of backup servers
	//Include mutexes here

	
	int otherSock;
	//Create socket for other servers
	if((otherSock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("Failed to create a socket");
		exit(SOCKET_FAILED);
	}
	//Put socket in a non-blocking state
	if(fcntl(otherSock, F_SETFL, O_NONBLOCK) == -1)
	{
		perror("Fcntl failed.");
		exit(FCNTL_FAILED);
	}
	other->sin_family = AF_INET;
	other->sin_port = htons((short)port);
	other->sin_addr.s_addr = htonl(INADDR_ANY);
	puts("Created other socket.");
	return otherSock;
}
int establishConnection(int otherSock, struct sockaddr_in* other){
	time_t start, end;
	start = clock();
	end = clock();
	int connected = 0;
	while (((end-start)/CLOCKS_PER_SEC) < TIMEOUT)
	{
		if(connect(otherSock, (struct sockaddr *)other, sizeof(*other)) != -1)
		{
			connected = 1;
			break;
		}
		end = clock();
	}
	return connected;
}
int receiveAcknowledgement(int sock, char response[MAX_RESPONSE_SIZE]){
	time_t start, end;
	start = clock();
	end = clock();
	int received = 0;
	while (((end-start)/CLOCKS_PER_SEC) < TIMEOUT)
	{
		if(recv(sock, response, MAX_RESPONSE_SIZE, 0) == -1){
			received = 1;
			break;
		}
		end = clock();
	}
	return received;
}
int sendToBackup(int *portGlobal, pthread_mutex_t *mut, char request[MAX_REQUEST_SIZE]){
	//message is sent from primary to a backup, and acknowledged by the backup
	int otherPort;
	struct sockaddr_in other;

	pthread_mutex_lock(mut);
	otherPort = (*portGlobal);
	pthread_mutex_unlock(mut);
	//printf("What port? %i\n", otherPort);
	if(otherPort != -1){
		//If port is alive (could be invalidated if dead) then get sockets
		int sock = createSockets(&other, otherPort);
		int conn = establishConnection(sock, &other);
		if(conn == -1){
			//puts("establishConnection failed");
			pthread_mutex_lock(mut);
			(*portGlobal) = -1;
			pthread_mutex_unlock(mut);
			return -1;
			//Server is dead
		}
		else{
			//Send request to backup server
			//printf("Sock before send %i\n", sock);
			if(send(sock, request, MAX_REQUEST_SIZE, 0) == -1){
				puts("send failed");
				pthread_mutex_lock(mut);
				(*portGlobal) = -1;
				pthread_mutex_unlock(mut);
				return -1;
			}
			return sock;
		}
	}
	return -1;
}
void setPrimary(){
	if(MY_PORT > OTHER_PORT1 && MY_PORT > OTHER_PORT2)
	{
		IS_PRIMARY = 1;
		PRIMARY = MY_PORT;
	}
	else if(OTHER_PORT1 > MY_PORT && OTHER_PORT1 > OTHER_PORT2)
	{
		IS_PRIMARY = 0;
		PRIMARY = OTHER_PORT1;
	}
	else
	{
		IS_PRIMARY = 0;
		PRIMARY = OTHER_PORT2;
	}
}
void handleUpdate(char *allTokens[], char response[MAX_RESPONSE_SIZE]){
	//Executes an update and stores a response
	if(strncmp(allTokens[0], "REGISTER", strlen(allTokens[0])) == 0){
		validateArguments(allTokens, 3);
		int val = verifyRegistration(allTokens[1], allTokens[2]);
		if(snprintf(response, MAX_RESPONSE_SIZE, "%i\n", val) < 0)
		{
			perror("Snprintf failed.");
			exit(SNPRINTF_FAILED);
		}
	}
	else if(strncmp(allTokens[0], "ADD_TWEET", strlen(allTokens[0])) == 0){
		validateArguments(allTokens, 4);
		//puts(allTokens[2]);
		int val = addTweet(allTokens[1], allTokens[2], allTokens[3]);
		if(snprintf(response, MAX_RESPONSE_SIZE, "%i\n", val) < 0)
		{
			perror("Snprintf failed.");
			exit(SNPRINTF_FAILED);
		}
	}
	else if(strncmp(allTokens[0], "ADD_FOLLOWEE", strlen(allTokens[0])) == 0){
		validateArguments(allTokens, 3);
		int val = addFollowee(allTokens[1], allTokens[2]);
		if(snprintf(response, MAX_RESPONSE_SIZE, "%i\n", val) < 0)
		{
			perror("Snprintf failed.");
			exit(SNPRINTF_FAILED);
		}
	}
	else if(strncmp(allTokens[0], "REMOVE_FOLLOWEE", strlen(allTokens[0])) == 0){
		validateArguments(allTokens, 3);
		int val = removeFollowee(allTokens[1], allTokens[2]);
		if(snprintf(response, MAX_RESPONSE_SIZE, "%i\n", val) < 0)
		{
			perror("Snprintf failed.");
			exit(SNPRINTF_FAILED);
		}
	}
	else if(strncmp(allTokens[0], "DELETE_ACCOUNT", strlen(allTokens[0])) == 0){
		validateArguments(allTokens, 2);
		int val = removeEntry(allTokens[1], CREDENTIALS_FILE, CREDENTIALS_ENTRY_SIZE, NUM_FIELDS_CREDENTIALS, "%s\t%s\n", 0, &credentialsMutex); //val represents if uname was deleted from credentials file
		removeEntry(allTokens[1], TWEETS_FILE, TWEET_ENTRY_SIZE, NUM_FIELDS_TWEETS, "%s\t%s\t%s\n", 0, &tweetsMutex);
		removeEntry(allTokens[1], FOLLOWEES_FILE, FOLLOWEES_ENTRY_SIZE, NUM_FIELDS_FOLLOWEES, "%s\t%s\n", 0, &followeesMutex);
		removeEntry(allTokens[1], FOLLOWEES_FILE, FOLLOWEES_ENTRY_SIZE, NUM_FIELDS_FOLLOWEES, "%s\t%s\n", 1, &followeesMutex);
		if(snprintf(response, MAX_RESPONSE_SIZE, "%i\n", val) < 0)
		{
			perror("Snprintf failed.");
			exit(SNPRINTF_FAILED);
		}
	}
	else{
			exit(INVALID_SELECTION);
		}
		
}
void handleRead(char *allTokens[], char response[MAX_RESPONSE_SIZE], int clientSock){
	//executes and sends read requests to the client
	if(strncmp(allTokens[0], "LOGIN", strlen(allTokens[0])) == 0){
		validateArguments(allTokens, 3);
		int val = verifyLogin(allTokens[1], allTokens[2]);
		if(snprintf(response, MAX_RESPONSE_SIZE, "%i\n", val) < 0)
		{
			perror("Snprintf failed.");
			exit(SNPRINTF_FAILED);
		}
		if(send(clientSock, response, MAX_RESPONSE_SIZE, 0) == -1)
		{
			perror("Send Failed.");
			exit(SEND_FAILED);
		}
	}
	else if(strncmp(allTokens[0], "LIST_FOLLOWEES", strlen(allTokens[0])) == 0){
		validateArguments(allTokens, 2);
		listFollowees(allTokens[1], clientSock);
	}
	else if(strncmp(allTokens[0], "LIST_OTHERS", strlen(allTokens[0])) == 0){
		validateArguments(allTokens, 2);
		int size = 1;
		char **followees = (char **)malloc(size*sizeof(char*));
		if(followees == NULL)
		{
			perror("Malloc failed");
			exit(MALLOC_FAILED);
		}
		loadFollowees(allTokens[1], &followees, &size);
		listOthers(allTokens[1], followees, clientSock, size-1);
		for(int i = 0; i < (size-1); i++)
		{
			free(followees[i]);
		}
		free(followees);
			
	}
	else if(strncmp(allTokens[0], "LIST_TWEETS", strlen(allTokens[0])) == 0){
		validateArguments(allTokens, 2);
		int size = 1;
		char **followees = (char **)malloc(size*sizeof(char*));
		if(followees == NULL)
		{
			perror("Malloc failed");
			exit(MALLOC_FAILED);
		}
		loadFollowees(allTokens[1], &followees, &size);
		listTweets(allTokens[1], followees, clientSock, size-1);
		for(int i = 0; i < (size-1); i++)
		{
			free(followees[i]);
		}
		free(followees);
	}

	else{
			exit(INVALID_SELECTION);
		}	
}
void notifyBackups(char request[MAX_REQUEST_SIZE]){
		//sends the message to the backups and waits for response
		int sock1, sock2;
		sock1 = sendToBackup(&OTHER_PORT1, &other1Mutex, request);
		printf("Other 1: %i\n", OTHER_PORT1);
		//sleep(10);
		sock2 = sendToBackup(&OTHER_PORT2, &other2Mutex, request);
		printf("Other 2: %i\n", OTHER_PORT2);
		char response1[MAX_RESPONSE_SIZE];
		bzero(response1, MAX_RESPONSE_SIZE);
		char response2[MAX_RESPONSE_SIZE];
		bzero(response2, MAX_RESPONSE_SIZE);
		if(!receiveAcknowledgement(sock1, response1)){
			puts("recv1 failed");
			pthread_mutex_lock(&other1Mutex);
			(OTHER_PORT1) = -1; //declare dead :(
			pthread_mutex_unlock(&other1Mutex);
		}
		if(!receiveAcknowledgement(sock2, response2)){
			puts("recv2 failed");
			pthread_mutex_lock(&other2Mutex);
			(OTHER_PORT2) = -1; //declare dead :(
			pthread_mutex_unlock(&other2Mutex);
		}				
		if(close(sock1) == -1)
		{
			perror("Close of backup socket1 failed.");
			//exit(CLOSE_FAILED);
		}
		if(close(sock2) == -1)
		{
			perror("Close of backup socket2 failed.");
			//exit(CLOSE_FAILED);
		}
	
}
void addLog(char request[MAX_REQUEST_SIZE]){
	//Appends a log entry to the log file
	FILE * fp;
	pthread_mutex_lock(&logMutex);
	if ((fp = fopen(LOG_FILE, "a")) == NULL)
	{
		perror("Couldn't open file");
		exit(FOPEN_FAILED);
	}
	if(fprintf(fp, "%s\n", request) < 0) // zero objects written to file
	{
		perror("Couldn't write to file");
		exit(FWRITE_FAILED);
	}
	if(fclose(fp) == EOF)
	{
		perror("Close failed.");
		exit(FCLOSE_FAILED);
	}
	pthread_mutex_unlock(&logMutex);
}
int checkLog(char request[MAX_REQUEST_SIZE]){
	//check if a given request exists in the log
	FILE * fp;
	pthread_mutex_lock(&logMutex);
	if ((fp = fopen(LOG_FILE, "r")) == NULL)
	{
		perror("Couldn't open file");
		exit(FOPEN_FAILED);
	}

	char log_entry[MAX_REQUEST_SIZE];
	while(fgets(log_entry, MAX_REQUEST_SIZE, fp))
	{
		int diff = strncmp(request, log_entry, strlen(request));
		if(diff == 0) // Match found
		{
			if(fclose(fp) == EOF)
			{
				perror("Close failed.");
				exit(FCLOSE_FAILED);
			}
			pthread_mutex_unlock(&logMutex);
			return 1; //Match found
		}
	}
	if(fclose(fp) == EOF)
	{
		perror("Close failed.");
		exit(FCLOSE_FAILED);
	}
	pthread_mutex_unlock(&logMutex);
	return 0; //Didn't find a match
}
int killPort(int target){
	int otherPort1 = OTHER_PORT1;
	int otherPort2 = OTHER_PORT2;
	//Invalidates a dead port number
	if(target == otherPort1)
	{
		pthread_mutex_lock(&other1Mutex);
		OTHER_PORT1 = -1;
		pthread_mutex_unlock(&other1Mutex);
		pthread_mutex_lock(&isPrimaryMutex);
		PRIMARY = -1;
		pthread_mutex_unlock(&isPrimaryMutex);		
		return 1;
	}
	else if(target == otherPort2)
	{
		pthread_mutex_lock(&other2Mutex);
		OTHER_PORT2 = -1;
		pthread_mutex_unlock(&other2Mutex);
		pthread_mutex_lock(&isPrimaryMutex);
		PRIMARY = -1;
		pthread_mutex_unlock(&isPrimaryMutex);		
		return 1;
	}
	return 0;
}
int selectCandidate(int myPort, int otherPort1, int otherPort2){
	if(myPort > otherPort1 && myPort > otherPort2)
	{
		return myPort;
	}
	else if(otherPort1 > myPort && otherPort1 > otherPort2)
	{
		return otherPort1;
	}
	else if(otherPort2 > otherPort1 && otherPort2 > myPort)
	{
		return otherPort2;
	}
	else{
		perror("Something went terribly wrong in selecting candidates.");
		return -1;
	}
	
}
void conductElections(char request[MAX_REQUEST_SIZE], int port){
	//sends the message to the backups and waits for response
	int otherPort1 = OTHER_PORT1;
	int otherPort2 = OTHER_PORT2;
	
	char response[MAX_RESPONSE_SIZE];
	int sock;
	if(port == otherPort1){
		sock = sendToBackup(&OTHER_PORT1, &other1Mutex, request);
	}
	else if(port == otherPort2){
		sock = sendToBackup(&OTHER_PORT2, &other2Mutex, request);
	}
	else{
		perror("Invalid port");
		exit(INVALID_PORT_NUM);
	}
	if(!receiveAcknowledgement(sock, response)){
		puts("Backup down");
		if(port == otherPort1){
			pthread_mutex_lock(&other1Mutex);
			(OTHER_PORT1) = -1;
			pthread_mutex_unlock(&other1Mutex);
		}
		else if(port == otherPort2){
			pthread_mutex_lock(&other2Mutex);
			(OTHER_PORT2) = -1;
			pthread_mutex_unlock(&other2Mutex);
		}
		else{
			perror("Failed to do port comparison");
		}
	}
	if(close(sock) == -1)
	{
		perror("Close of backup socket failed.");
		//exit(CLOSE_FAILED);
	}
}
