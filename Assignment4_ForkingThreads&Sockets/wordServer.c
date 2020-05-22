
//	Compile with:
//	$ gcc wordServer.c -o wordServer -lpthread

//---		Header file inclusion					---//
//
//

#include	"wordClientServer.h"
#include	<pthread.h>	// For pthread_create()


//---		Definition of constants:				---//

#define		STD_OKAY_MSG		"Okay"

#define		STD_ERROR_MSG		"Error doing operation"

#define		STD_BYE_MSG		"Good bye!"

#define		THIS_PROGRAM_NAME	"wordServer"

#define		OUTPUT_FILENAME		"out.txt"

#define		ERROR_FILENAME		"err.txt"

const int	ERROR_FD		= -1;


//---		Definition of functions:				---//



//  PURPOSE:  To run the server by 'accept()'-ing client requests from
//	'listenFd' and doing them.
void		doServer	(int		listenFd
				)
{
  //  I.  Application validity check:

  //  II.  Server clients:
  pthread_t		threadId;
  pthread_attr_t	threadAttr;
  int			threadCount	= 0;

  // YOUR CODE HERE

  int clientDescriptor;
  int * pass;
  
  pthread_attr_init(&threadAttr);
  pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);
  
  while( (clientDescriptor = accept(listenFd, NULL, NULL) ) > 0){
	  
	  pass = (int*)malloc(sizeof(int)*2);
	  pass[0] = clientDescriptor;
	  pass[1] = threadCount;
	  threadCount++;
	  
	  pthread_create(&threadId, &threadAttr, handleClient, pass);
	  
  }
  
  //  III.  Finished:
}

void* handleClient(void* vPtr){
	
	int * localArray =  (int*) vPtr;
	int fd = localArray[0];
	int threadNum = localArray[1];
	free(localArray);
	
	//  II.B.  Read command:
  char	buffer[BUFFER_LEN];
  char	command;
  int	fileNum;
  char	text[BUFFER_LEN];
  int 	shouldContinue	= 1;

  while  (shouldContinue)
  {
    text[0]	= '\0';

    read(fd,buffer,BUFFER_LEN);
    printf("Thread %d received: %s\n",threadNum,buffer);
    sscanf(buffer,"%c %d \"%[^\"]\"",&command,&fileNum,text);

    // YOUR CODE HERE
	if(command == DIR_CMD_CHAR){
		printDir(fd);
	}
	
	if(command == WRITE_CMD_CHAR){
		writeFile(fd, fileNum, text);
	}
	
	if(command == SPELL_CMD_CHAR){
		spellCheck(fd, fileNum);
	}
	
	if(command == DELETE_CMD_CHAR){
		deleteFile(fd, fileNum);
	}
	
	if(command == QUIT_CMD_CHAR){
		 write(fd, STD_BYE_MSG, strlen(STD_BYE_MSG) + 1);
		 shouldContinue = 0;
	}
	
  }
  
  printf("Thread %d quitting.\n",threadNum);
  return(NULL);
	
}

void printDir(int fd){
	
  char	bufferLocal[BUFFER_LEN];
  DIR*		dirPtr	= opendir(".");
  int usedSize = 0;
  
  bufferLocal[0] = '\0';
  
  if  (dirPtr == NULL)
  {
	  write(fd, STD_ERROR_MSG, strlen(STD_ERROR_MSG) + 1);
      return;
  }
  
  struct dirent*	entryPtr;
	
  while  ( (entryPtr = readdir(dirPtr)) != NULL){
	if(usedSize +   strlen(entryPtr->d_name) + 1 > BUFFER_LEN){
		break;
	}else{
		usedSize = usedSize + strlen(entryPtr->d_name) + 1;
		strncat(bufferLocal, entryPtr->d_name, BUFFER_LEN);
		strncat(bufferLocal, "\n", BUFFER_LEN);
	}
	
  }
  closedir(dirPtr);
  write(fd, bufferLocal, strlen(bufferLocal) + 1);
	
}

void writeFile(int fd, int fileNum, char text[]){
	FILE* localFile;
	char fileString[6];
	fileString[0] = '\0';
	sprintf(fileString, "%d.txt", fileNum);
	localFile = fopen(fileString, "w");
	int printedChars = 0;
	
	if(localFile == NULL){
		write(fd, STD_ERROR_MSG, strlen(STD_ERROR_MSG) + 1);
		return;
	}
	
	printedChars = fprintf(localFile, "%s", text);
	fclose(localFile);
	
	if(printedChars == strlen(text)){
		write(fd, STD_OKAY_MSG, strlen(STD_OKAY_MSG) + 1);
	}else{
		write(fd, STD_ERROR_MSG, strlen(STD_ERROR_MSG) + 1);
	}
	
	
}

void spellCheck(int fd, int fileNum){
	
	int fileFd;
	char fileString[6];
	fileString[0] = '\0';
	sprintf(fileString, "%d.txt", fileNum);
	fileFd = open( fileString,O_RDWR ,0660);
	
	if(fileFd == -1){
		write(fd, STD_ERROR_MSG, strlen(STD_ERROR_MSG) + 1);
		return;
	}
	
	long	length	= lseek(fileFd,0,SEEK_END);
	int	status	= write(fileFd,ENDING_TEXT,sizeof(ENDING_TEXT)-1);
	
	int   fromSpeller[2];
	
	if  (pipe(fromSpeller)  < 0)  {
		write(fd, STD_ERROR_MSG, strlen(STD_ERROR_MSG) + 1);
	}
	
	int pid = fork();
	
	if( pid == 0){
		//Baby
		lseek(fileFd,0,SEEK_SET);
		close(0);
		dup(fileFd);
		close(1);
		dup(fromSpeller[1]);
		
		int execError;
		execError = execl(SPELLER_PROGNAME, SPELLER_PROGNAME, SPELLER_PROG_OPTION);
		
		if(execError == -1){
			write(fromSpeller[1], STD_ERROR_MSG, strlen(STD_ERROR_MSG) + 1);
			write(fromSpeller[1], ENDING_TEXT, strlen(ENDING_TEXT) + 1);
			exit(EXIT_FAILURE);
		}
		
	} else{
		
		int readBytes;
		char missSpellings[BUFFER_LEN];
		missSpellings[0] = '\0';
		char tempString[BUFFER_LEN];
		int totalBytes = 0;
		
		while( (readBytes = read (fromSpeller[0], tempString, BUFFER_LEN)) > 0){
			
			totalBytes = totalBytes + readBytes + 1;
			
			if(totalBytes > BUFFER_LEN){
				break;
			}
			
			char* p = strstr(tempString, ENDING_TEXT);
			
			if(p != NULL){
				*p = '\0';
			}
			
			tempString[readBytes] = '\0';
		
			strcat(missSpellings, tempString);
			
			if( p == NULL){
			strcat(missSpellings, "\n");
			}
			
			if(p != NULL)
				break;
		}
		int status;
		
		wait(&status);
		ftruncate(fileFd,length);
		close(fromSpeller[0]);
		close(fileFd);
	
		write(fd, missSpellings, strlen(missSpellings) + 1);
	
	}

	
}

void deleteFile(int fd, int fileNum){
	char fileString[6];
	sprintf(fileString, "%d.txt", fileNum);
	int errorCheck;
	errorCheck = unlink(fileString);
	
	if(errorCheck == -1){
		write(fd, STD_ERROR_MSG, strlen(STD_ERROR_MSG) + 1);
	}else{
		write(fd, STD_OKAY_MSG, strlen(STD_OKAY_MSG) + 1);
	}
}


//  PURPOSE:  To decide a port number, either from the command line arguments
//	'argc' and 'argv[]', or by asking the user.  Returns port number.
int		getPortNum	(int	argc,
				 char*	argv[]
				)
{
  //  I.  Application validity check:

  //  II.  Get listening socket:
  int	portNum;

  if  (argc >= 2)
    portNum	= strtol(argv[1],NULL,0);
  else
  {
    char	buffer[BUFFER_LEN];

    printf("Port number to monopolize? ");
    fgets(buffer,BUFFER_LEN,stdin);
    portNum	= strtol(buffer,NULL,0);
  }

  //  III.  Finished:  
  return(portNum);
}


//  PURPOSE:  To attempt to create and return a file-descriptor for listening
//	to the OS telling this server when a client process has connect()-ed
//	to 'port'.  Returns that file-descriptor, or 'ERROR_FD' on failure.
int		getServerFileDescriptor
				(int		port
				)
{
  //  I.  Application validity check:

  //  II.  Attempt to get socket file descriptor and bind it to 'port':
  //  II.A.  Create a socket
  int socketDescriptor = socket(AF_INET, // AF_INET domain
			        SOCK_STREAM, // Reliable TCP
			        0);

  if  (socketDescriptor < 0)
  {
    perror(THIS_PROGRAM_NAME);
    return(ERROR_FD);
  }

  //  II.B.  Attempt to bind 'socketDescriptor' to 'port':
  //  II.B.1.  We'll fill in this datastruct
  struct sockaddr_in socketInfo;

  //  II.B.2.  Fill socketInfo with 0's
  memset(&socketInfo,'\0',sizeof(socketInfo));

  //  II.B.3.  Use TCP/IP:
  socketInfo.sin_family = AF_INET;

  //  II.B.4.  Tell port in network endian with htons()
  socketInfo.sin_port = htons(port);

  //  II.B.5.  Allow machine to connect to this service
  socketInfo.sin_addr.s_addr = INADDR_ANY;

  //  II.B.6.  Try to bind socket with port and other specifications
  int status = bind(socketDescriptor, // from socket()
		    (struct sockaddr*)&socketInfo,
		    sizeof(socketInfo)
		   );

  if  (status < 0)
  {
    perror(THIS_PROGRAM_NAME);
    return(ERROR_FD);
  }

  //  II.B.6.  Set OS queue length:
  listen(socketDescriptor,5);

  //  III.  Finished:
  return(socketDescriptor);
}


int		main		(int	argc,
				 char*	argv[]
				)
{
  //  I.  Application validity check:

  //  II.  Do server:
  int 	      port	= getPortNum(argc,argv);
  int	      listenFd	= getServerFileDescriptor(port);
  int	      status	= EXIT_FAILURE;

  if  (listenFd >= 0)
  {
    doServer(listenFd);
    close(listenFd);
    status	= EXIT_SUCCESS;
  }

  //  III.  Finished:
  return(status);
}