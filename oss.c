#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <stdio.h> 
#include <stdlib.h>
#include <signal.h> 
#include <unistd.h>
#include <semaphore.h> 
#include <fcntl.h>
#include <string.h>
#include <time.h>

#define nanosecondsecond 1000000000
#define SNAME "/mysem13543"
#include "queue.h"
#define n 18
#define m 20

long seed;

typedef struct Time_Clock 
{
	int	second;
	long int nanosecond;
	int totalcount;
} Time_Clock;

struct messageg_buffer { 
    long message_type; 
    char message_text[10]; 
	int system_resource;
	int systemrequestType;
	int pid;
} message; 

typedef struct systemstate {
int system_resource[m];
int availablemem[m];
int systemclaim[n][m];
int allocation[n][m];
} systemstate;

FILE *fptr;
systemstate s;
int Time_Clockid,msgid;
Time_Clock *logTime_Clock;
int lines = 0,verbose = 0;
int pids[18] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
int statusVector[18]  = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
char *envp[21];
struct Queue pq;
struct Queue resq;
int deadLockRun = 0, deadLockDetected  = 0,unblocked = 0, blocked = 0, acknowledged = 0, granted = 0;
void MatrixPrint();
void sigintHandler(int sig_num) 
{ 
	int i,j;
    if(sig_num == 2)
    	fprintf(stderr,"interrupt occured in program\n");
    else
	fprintf(stderr,"Program exceded time limit\n");
	fprintf(stderr,"Status (Time_Clock) --> Sec --> %d, NanoSec --> %d\n", logTime_Clock -> second, logTime_Clock -> nanosecond);
	fprintf(stderr,"Childs Forked (Total) --> %d\n",logTime_Clock -> totalcount);
	
   for(i = 0; i < 18; i++)
       if(pids[i] > 0 )
	       kill(pids[i], SIGTERM);
	MatrixPrint();
	fprintf(stderr,"childs which are remaining removed \n");
        fprintf(stderr,"Run (DeadLock) --> %d Detected --> %d\n",deadLockRun,deadLockDetected);
	fprintf(stderr,"Requests --> system resource  \n");
	fprintf(stderr,"Granted--> %d Blocked --> %d Release --> %d, UnBlocked --> %d\n",granted,blocked,acknowledged,unblocked);
	shmdt(logTime_Clock);
	fprintf(stderr,"Shared Memory detached\n");
	fclose(fptr);
	fprintf(stderr,"File pointer are closed\n");
	shmctl(Time_Clockid,IPC_RMID,NULL);
	fprintf(stderr,"Shared Memory Cleared \n");
	msgctl(msgid, IPC_RMID, NULL); 
	 fprintf(stderr,"Message Queue Cleared \n");
    abort(); 
    fflush(stdout); 
}
int safe (systemstate S) {
	
	deadLockRun++;
	int currentavail[m];
	int totalcount = 0,i,j,c,k;
	int found = 0, possible = 1,completed = 0;
	
	for(i=0; i<n; i++)
		if(statusVector[i] == 0)
			totalcount++;
	int rest[totalcount]; 
	for(i=0; i<m; i++)
		currentavail[i] = S.availablemem[i];
	j=0;
	for(i=0; i<n; i++)
		if(statusVector[i] == 0)
			rest[j++] = i;
	possible = 1;
	
	while (possible) {
		for(j=0;j<totalcount;j++)
		{
		if(rest[j] != -1)
		{
		k = rest[j];
		c = 0;
		for(i=0; i<m; i++)
			if((S.systemclaim[k][i] - S.allocation[k][i]) <= currentavail[i])
				c++;
		if(c == m)
			{
				found = 1;
				break;
			}
		}
		}
		if (found) { 
			for(i=0; i<m; i++)
				currentavail[i] = currentavail[i] + S.allocation[k][i];
			rest[j] = -1;
			found  = 0;
		}
		else
			possible = 0;	

	}
		for(i=0; i<totalcount; i++)
			if(rest[i] >= 0)
			{
				deadLockDetected++;
				return 0;
			}
		return 1;
}
 
void MatrixPrint() 
{
	int i,j,p,t;
	fprintf(fptr,"\n-------------------------*************************-------------------------\n");
	fprintf(fptr,"system_resource : ");
	for(i=0; i<m; i++) 
		fprintf(fptr,"%d ",s.system_resource[i]);
	fprintf(fptr,"\n"); 
	fprintf(fptr,"Avalible : ");
	for(i=0; i<m; i++) 
		fprintf(fptr,"%d ",s.availablemem[i]);	
	fprintf(fptr,"\n"); 
	fprintf(fptr,"allocationated Matrix : \n");
	for(i=0; i<n; i++) 
	{
      for(j=0;j<m;j++) 
         fprintf(fptr,"%d ", s.allocation[i][j]);
	  fprintf(fptr,"\n"); 	
    }
	fprintf(fptr,"\n-------------------------*************************-------------------------\n"); 
}
void wakeUp()
{
	int tP,tR,tem;
	int i,j,tempP[n],tempR[n],totalcount;
	for(i=0;i<n;i++)
	{
		tempP[i] = -1;
		tempR[i] = -1;
	}
	totalcount=0;
	while((tem = pop(&pq)) > 0 )
	{ 
	
	tP = tem;
	tR = pop(&resq);
				if(s.availablemem[tR] > 0)
				{
				s.availablemem[tR] -=1;
				s.allocation[getLoc(tP)][tR] +=1;
				statusVector[getLoc(tP)] = 0;			
				int p = safe(s);
				if(p)
				{
						unblocked++;
						granted++;
					  message.message_type = tP;
					  msgsnd(msgid, &message, sizeof(message),0); 
					  if(granted % 20 == 0 && !verbose)
					  {
						  MatrixPrint();
						  lines += 25;
					  }
					  if(lines < 10000)
						fprintf(fptr,"Master unblocking --> %d granting --> R%d time --> %d:%d\n",tP,tR,logTime_Clock -> second, logTime_Clock -> nanosecond,lines++);
				}
				else
				{
					s.availablemem[tR] +=1;
					s.allocation[getLoc(tP)]
					[tR] -=1;
					tempP[totalcount]  = tP;
					tempR[totalcount] = tR;
 					totalcount++;
					statusVector[getLoc(tP)] = 1;	
				}
				}
				
				
	}
	
	for(i=0;i<totalcount;i++)
	{
		push(&pq,tempP[i]);
		push(&resq,tempR[i]);
	}
	
}
void intilize()
{
	int i,j,r;
	for(i=0; i<m; i++) 
	{
		srand(seed++);
		r = (rand() % (20 - 10 + 1)) + 10;
		s.system_resource[i] = r;
		s.availablemem[i] = r;
		
      for(j=0;j<n;j++)
	  {
		  s.allocation[j][i] = 0;
		  s.systemclaim[j][i] = 0;
	  }
	}
}
int forkChild()
{
	
	int i;
	for(i  = 0; i < 18; i++ )
		if(pids[i] == -1)
			return i;
	return -1;
}

int getLoc(int pid)
{
	int i;
	for(i  = 0; i < 18; i++ )
		if(pids[i] == pid)
			return i;
	return -1;
}

void createMax(int loc)
{
	int i;
	char c[sizeof(int)];
	for(i=0; i<m; i++)
	{
		srand(seed++);
		s.systemclaim[loc][i] = (rand()% 10) + 1;
		snprintf(c, sizeof(int), "%d",s.systemclaim[loc][i]); 
		envp[i] = malloc(sizeof(c)); 
        strcpy(envp[i], c);
		
	}

}
void advTime_Clock()
{
	logTime_Clock -> second += 1;
	if(logTime_Clock -> nanosecond > nanosecondsecond)
		{
			logTime_Clock -> second += logTime_Clock -> nanosecond/nanosecondsecond;
			logTime_Clock -> nanosecond %= nanosecondsecond;
		}	
	
}

int main (int argc,char *argv[]) 
{ 
	intilize();
	seed = time(NULL);
	char arg1[15];
	char arg2[15];
	char arg3[20];
	char *filename = "default";
	int c, i,status,pid,t=2;
	int TimeToGenrate = 0;
	
	signal(SIGALRM, sigintHandler);
    signal(SIGINT, sigintHandler);
	
	while ((c = getopt (argc, argv, "hl:t:v:")) != -1)
	switch (c)
    {
		case 'h':
			printf("\nOPTIONS :\n");
            printf("-h for HELP \n");
			printf("-l filename  (File where output to be stored)\n");
			printf("-v  1 (To set Verbose ON)\n");
			printf("-t z (where z is maximum no.of second program allowed to run\n");
			return 1;
		case 'l':
			filename = optarg;
			break;
		case 't':
			t = atoi(optarg);
			break;	
		case 'v':
			verbose = atoi(optarg);
			break;	
		case '?':
			if (optopt == 'z' || optopt == 'l' || optopt == 's')
				fprintf (stderr, "Option -%c requires an argument.\n", optopt);
			else if (isprint (optopt))
				fprintf (stderr, "Unknown option `-%c'.\n", optopt);
			else
				fprintf (stderr,"Unknown option character `\\x%x'.\n",optopt);
			return 1;
		default:
        abort ();
      }
	  
	fptr = fopen(filename, "w");
    if(fptr == NULL)
    {
      fprintf(stderr,"File Open ERROR \n");
      exit(1);
    }
	fprintf(stderr,"Log file : %s     ",filename);
	if(verbose)
		fprintf(stderr,"Verbose ON\n");
	else
		fprintf(stderr,"Verbose OFF (-v 1 to ON verbose)\n");
	key_t key = ftok(".",'v');
	key_t key2 = ftok(".",'r');
	
	Time_Clockid = shmget(key,sizeof(logTime_Clock),0666|IPC_CREAT);
	if ( Time_Clockid == -1 )
		fprintf(stderr,"OSS : Error in shmget2");
	
	logTime_Clock = (Time_Clock *)shmat(Time_Clockid, NULL, 0);
	
	logTime_Clock -> second = 0;
	logTime_Clock -> nanosecond = 0;
	logTime_Clock -> totalcount  = 0;
	
	msgid = msgget(key2, 0666 | IPC_CREAT); 
	
	snprintf(arg1,15,"%d", Time_Clockid);
	snprintf(arg2,15,"%d",msgid);
	
	
	
	alarm(t);	
	int loc = -1,p;
	while(1)
	{
		if((loc = forkChild()) >= 0  && logTime_Clock -> second  >= TimeToGenrate)
		{
			createMax(loc);
			if( (pid = fork()) == 0)
			{
				execle("./user","./user",arg1,arg2,arg3,(char *)NULL,envp);
				fprintf(stderr,"%s failed to exec user!\n",argv[0]);
				exit(0);
			}
			statusVector[loc]  = 0;
			srand(seed++);
			TimeToGenrate += rand()%2; 
			pids[loc] = pid;
		}
		
		if(msgrcv(msgid, &message, sizeof(message), 1,IPC_NOWAIT) != -1)
		{
			if(message.systemrequestType == 1)
			{
				if(s.availablemem[message.system_resource] > 0)
				{
				if(lines < 10000 && !verbose)					
					fprintf(fptr,"Master Process --> %d requesting --> R%d time --> %d:%ld\n",message.pid,message.system_resource, logTime_Clock -> second, logTime_Clock -> nanosecond,lines++);
				s.availablemem[message.system_resource] -=1;
				s.allocation[getLoc(message.pid)][message.system_resource] +=1; 
				int p = safe(s);
				if(p)
				{
					  
					  message.message_type = message.pid;
					  granted++;
					  if(granted % 20 == 0 && !verbose)
					  {
						  MatrixPrint();
						  lines += 25;
					  }
					  if(lines < 10000 && !verbose)	
						fprintf(fptr,"Master granting --> %d request --> R%d time --> %d:%ld\n",message.pid,message.system_resource, logTime_Clock -> second, logTime_Clock -> nanosecond,lines++);
					  msgsnd(msgid, &message, sizeof(message),0); 
				}
				else
				{
					blocked++;
					s.availablemem[message.system_resource] +=1;
					s.allocation[getLoc(message.pid)][message.system_resource] -=1;
					if(lines < 10000)	
						fprintf(fptr,"Master blocking --> %d requesting --> R%d time --> %d:%ld\n",message.pid,message.system_resource, logTime_Clock -> second, logTime_Clock -> nanosecond,lines++);	
					statusVector[getLoc(message.pid)] = 1;	
					push(&resq,message.system_resource);
					push(&pq,message.pid);
					wakeUp();
					
					
				}
				}
				else
				{
					blocked++;
					if(lines < 10000)	
						fprintf(fptr,"Master blocking  --> %d requesting --> R%d time --> %d:%ld resources NOT avaliable\n",message.pid,message.system_resource, logTime_Clock -> second, logTime_Clock -> nanosecond,lines++);	
					push(&resq,message.system_resource);
					push(&pq,message.pid);
					
					statusVector[getLoc(message.pid)] = 1;	
					wakeUp();
				}
			}
			else if(message.systemrequestType == 0)
			{
					acknowledged++;
					if(lines < 10000 && !verbose)	
						fprintf(fptr,"Master cknowledged Process --> %d releasing --> R%d time --> %d:%ld\n",message.pid,message.system_resource, logTime_Clock -> second, logTime_Clock -> nanosecond,lines++);		
					s.availablemem[message.system_resource] +=1;
					s.allocation[getLoc(message.pid)][message.system_resource] -=1; 
					wakeUp();
			}
			else if(message.systemrequestType == 2)
			{
				int k,status;
				k = getLoc(message.pid);
				if(lines < 10000)	
					fprintf(fptr,"Master detected Process --> %d completed Extection --> %d:%ld - Releasing system_resources\n",message.pid,logTime_Clock -> second, logTime_Clock -> nanosecond,lines++);
				for(i=0; i<m; i++)
				{
				s.availablemem[i] = s.availablemem[i] + s.allocation[k][i];
				 s.allocation[k][i] = 0;
				}
				
				pids[k] = -1;
				statusVector[getLoc(message.pid)] = -1;
				waitpid(message.pid, &status,0);
			}
		}
		else
				advTime_Clock();
		logTime_Clock -> nanosecond += 100;	
		if(logTime_Clock -> nanosecond > nanosecondsecond)
		{
			logTime_Clock -> second += logTime_Clock -> nanosecond/nanosecondsecond;
			logTime_Clock -> nanosecond %= nanosecondsecond;
		}	
		
	} 
	
}
	
	
	
