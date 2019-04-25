#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
 #define nanosecondsecond 1000000000


typedef struct Clock 
{
	int	second;
	long int nanosecond;
	int totalcount;
} Clock;

struct mesg_buffer { 
    long message_type; 
    char message_text[10]; 
	int resource;
	int requestType;
	int pid;
} message; 
static long sed;
Clock *clock;
int r;


int RandomNumber()
{
	srand(sed++ * getpid());
	r =rand()%100;
	if(r <= 60)
		return 1;			
	else if(r <= 90)
		return 2;			
	else
		return 3;			


}

int main(int argc,char *argv[], char* envp[])
 {
	 sed = time(NULL);
	 int max[20];
	 int alc[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	 int clockid = atoi(argv[1]);
	 int msgid = atoi(argv[2]);
	 int i,pid,temp,t;
	 int second = 0;
	 long nanosecond = 0;
	 pid = getpid();
 
	 
	 clock = shmat(clockid, NULL, 0);
	 clock -> totalcount +=1;
	 
	 for(i = 0; i < 20;i++)
		 max[i] = atoi(envp[i]); 	

	 while(1){

	
		if(clock -> second  >= second || clock -> nanosecond  >= nanosecond)
		{
		srand(sed++ * pid);
		temp = rand() % 100;
		nanosecond += temp; 
		if(nanosecond > nanosecondsecond)
		{
			second += nanosecond/nanosecondsecond;
			nanosecond %= nanosecondsecond;
		}	
		t = RandomNumber();
		if(t == 3)
			break;
		else if(t == 2)
		{
				temp = -1;
				 for(i = 0; i < 20;i++)
						if(alc[i]>0)
							temp = i;
				if(temp != -1)
				{	
				message.resource = temp;
				srand(sed++ * pid);
				message.requestType = 0;    		//Release resource;		
				message.message_type = 1; 
				message.pid = pid;
				alc[temp] -= 1; 
				msgsnd(msgid, &message, sizeof(message), 0); 
				}
				else
						break;
				
		}
		else
		{		
				
				srand(sed++ * pid);
				temp = rand()%20;
				if(alc[temp] < max[temp])
				{
				message.resource =temp;
				srand(sed++ * pid);
				message.requestType = 1;    		//Request resource;		
				message.message_type = 1; 
				message.pid = pid;
				msgsnd(msgid, &message, sizeof(message), 0); 
				msgrcv(msgid, &message, sizeof(message), pid, 0);
				alc[temp] += 1; 
				}
				
				
		}
		}
		 
	 }
	 message.requestType = 2;    				
	 message.message_type = 1; 
	 message.pid = pid;
	 msgsnd(msgid, &message, sizeof(message), 0); 
	 shmdt(clock);
	 return 0;
}

 
