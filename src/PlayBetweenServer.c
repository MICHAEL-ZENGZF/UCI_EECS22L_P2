#include"PlayBetween.h"
#include"GUI.h"


extern const char *Program;

int Shutdown = 0;/* keep running until Shutdown == 1 */

bool firstTimeOut=true;//as is its name describes
bool ServicesListInit=false;
char RotateLine[]={'-','\\','|','/'};//for showing a rotating line after still waiting...
uchar RotateDirection=0;//the current rotate direction

int PlayBetweenSocketFD;//as is its name describes
int PlayBetweenServerPort;//as is its name describes

pthread_t PlayBetweenLooperID;//the thread id for the playbetween server looper
OnlinePlayCallback *RecvCallback;//will be called when a play pack is received

int TimeOutMicroSec=250000;//as is its name describes

extern OnlinePlayer localPlayer, remotePlayer;//as is its name describes
extern bool isPlayingWithOpponent;//as is its name describes
extern bool isWaitingChallengeRespose;//as is its name describes

//as is its name describes
void PlayBetweenTimeOutHandler()//the hanle function for timeout
{
    if(firstTimeOut){
        printf("local server running... ");
        firstTimeOut=false;
    }
    printf("%c",RotateLine[RotateDirection]);
    fflush(stdout);
    RotateDirection=++RotateDirection%4;
    printf("\b");
}

int MakeServerSocket(		/* create a socket on this server */
	uint16_t PortNo)
{
    int ServSocketFD;
    struct sockaddr_in ServSocketName;

    /* create the socket */
    ServSocketFD = socket(PF_INET, SOCK_STREAM, 0);//the socket descriptor representing the endpoint
    if (ServSocketFD < 0)
    {   FatalError("service socket creation failed");
    }
    /* bind the socket to this server */
    ServSocketName.sin_family = AF_INET;
    ServSocketName.sin_port = htons(PortNo);
    ServSocketName.sin_addr.s_addr = htonl(INADDR_ANY);

    //Bind the socket
    if (bind(ServSocketFD, (struct sockaddr*)&ServSocketName,
		sizeof(ServSocketName)) < 0){
        printf("Fail to bind server to socket, port %d may has been used\n",PortNo);
        return -1;
    }
    /* start listening to this socket */
    if (listen(ServSocketFD, 5) < 0)	/* max 5 clients in backlog */
    {   FatalError("listening on socket failed");
    }
    return ServSocketFD;
} /* end of MakeServerSocket */

//the listener for play pack
void PlayPackListener(int DataSocketFD)
{
    firstTimeOut=false;
    int  l;
    char *SendBuf=NULL;
    char *fullbuf=readFullBuffer(DataSocketFD);
    printf("Someone just send you:%s\n",fullbuf);
    if(matchRegex("iChooseColor:.",fullbuf)){
        if(isWaitingChallengeRespose){
            isWaitingChallengeRespose=false;
            isPlayingWithOpponent=true;
            char strColor=fullbuf[13];
            if(strColor=='w'){
                remotePlayer.color=WHITE;
                localPlayer.color=BLACK;
            }
            else{
                remotePlayer.color=BLACK;
                localPlayer.color=WHITE;
            }
            SendBuf=COLOR_SELECTION_RECEIVED;
            guio_removeWaitActionDialog();
        }
        else{
            SendBuf=CHALLENGE_IS_CANCELED;
        }
    }
    else if(!strcmp(fullbuf,PLAYBETWEEN_USER_QUIT)){
        guio_InformMsg("Your opponent have surrendered\nCongratulations!!!\nYou have win the game!!!");
        isPlayingWithOpponent=false;
    }
    else{
        PackPlay packPlay=decodeStrPP(fullbuf);
        RecvCallback->RecvCallback(packPlay);
        SendBuf=PLAY_PACK_RECEIVED;
    }

    free(fullbuf);

    //TODO: answer request
    
    if(SendBuf==NULL)SendBuf="";
    l = strlen(SendBuf);
#ifdef PRINT_LOG
    printf("%s: Sending response: %s.\n", Program, SendBuf);
#endif

    int LastSendLen;
    //send back to client
    LastSendLen = write(DataSocketFD, SendBuf, l);
    if (LastSendLen < 0)FatalError("writing to data socket failed");
}

//the main looper for local server
void PlayBetweenLooper()
{
    int Timeout=TimeOutMicroSec;
    int DataSocketFD;	/* socket for a new client */
    socklen_t ClientLen;
    struct sockaddr_in
	ClientAddress;	/* client address we connect with */
    fd_set ActiveFDs;	/* socket file descriptors to select from */
    fd_set ReadFDs;	/* socket file descriptors ready to read from */

    int res, i;
    struct timeval TimeVal;

    FD_ZERO(&ActiveFDs);		/* set of active sockets */
    FD_SET(PlayBetweenSocketFD, &ActiveFDs);	/* server socket is active */
    while(!Shutdown)
    {
	ReadFDs = ActiveFDs;
    TimeVal.tv_sec  = Timeout / 1000000;
	TimeVal.tv_usec = Timeout % 1000000;

	/* block until input arrives on active sockets or until timeout */
	res = select(FD_SETSIZE, &ReadFDs, NULL, NULL, &TimeVal);
	if (res < 0)
	{   FatalError("wait for input or timeout (select) failed");
	}
	if (res == 0)	/* timeout occurred */
	{
	    PlayBetweenTimeOutHandler();
	}
	else		/* some FDs have data ready to read */
	{   for(i=0; i<FD_SETSIZE; i++)
	    {   if (FD_ISSET(i, &ReadFDs))
		{   if (i == PlayBetweenSocketFD)
		    {	/* connection request on server socket */
#ifdef DEBUG
			printf("\n%s: Accepting new client...\n", Program);
#endif
			ClientLen = sizeof(ClientAddress);
			DataSocketFD = accept(PlayBetweenSocketFD,
				(struct sockaddr*)&ClientAddress, &ClientLen);
			if (DataSocketFD < 0)
			{   FatalError("data socket creation (accept) failed");
			}
#ifdef DEBUG
			printf("%s: New client connected from %s:%hu.\n",
				Program,
				inet_ntoa(ClientAddress.sin_addr),
				ntohs(ClientAddress.sin_port));
#endif
			FD_SET(DataSocketFD, &ActiveFDs);
		    }
		    else
		    {   /* active communication with a client */
#ifdef DEBUG
			printf("%s: Dealing with client on FD%d...\n",
				Program, i);
#endif
			PlayPackListener(i);
#ifdef DEBUG
			printf("%s: Closing client connection FD%d.\n\n",
				Program, i);
#endif
			close(i);
			FD_CLR(i, &ActiveFDs);
		    }
		}
	    }
	}
    }
} /* end of ServerMainLoop */

//the callback for play pack
void PPcalllbackFunc(PackPlay pack)
{
    if(!isPlayingWithOpponent)return;
    if(pack.Action==PLAYBETWEEN_CHAT){
        
    }
    else if(pack.Action==PLAYBETWEEN_PLAY){
        env_play2(RecvCallback->pGameState,pack.start_pt,pack.end_pt,0);
    }
    else if(pack.Action==PLAYBETWEEN_UNDO){
        env_undo(RecvCallback->pGameState);
        env_undo(RecvCallback->pGameState);
    }
}


//as is its name describes
//callback: this function will be called when a PackPlay is received
int InitPlayBetweenServer(OnlinePlayCallback *callback)
{
    #ifdef PRINT_LOG
    printf("%s: Creating the User Pack listen socket...\n", Program);
    #endif
    int PortNb=11200;
    for(;PortNb<12000;PortNb++)
    {
        PlayBetweenSocketFD = MakeServerSocket(PortNb);//Make a socket
        if(PlayBetweenSocketFD>=0)break;
    }

    printf("%s: Providing PlayPackListener service at port %d...\n", Program, PortNb);
    PlayBetweenServerPort=PortNb;
    callback->RecvCallback=PPcalllbackFunc;
    RecvCallback=callback;
    
    //start the PlayBetweenLooper
    int ret=pthread_create(&PlayBetweenLooperID,NULL,(void*)PlayBetweenLooper,NULL);

    if(ret!=0){
        printf("Create Local Sever thread fail, exiting\n");
        exit(1);
    }

    return PortNb;
}
//as is its name describes
void ShutPlayBetweenServer()
{
    #ifdef PRINT_LOG
    printf("\n%s: Shutting down UserPackListener service\n", Program);
    #endif
    Shutdown=1;
    pthread_join(PlayBetweenLooperID,NULL);
    close(PlayBetweenSocketFD);
}
