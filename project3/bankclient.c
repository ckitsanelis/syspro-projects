#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include "func.h"

int main (int argc, char* argv[])
{
	if(argc!=7)
	{
		printf("Incorrect number of arguments\n");
		exit(EXIT_FAILURE);
	}
	char host_name[100];
	int port_number;
	FILE *fp_commands;
	int file_exists=1;

	if(strcmp(argv[1],"-p")==0)
		port_number=atoi(argv[2]);
	else if(strcmp(argv[3],"-p")==0)
		port_number=atoi(argv[4]);
	else if(strcmp(argv[5],"-p")==0)
		port_number=atoi(argv[6]);
	else
	{
		printf("Port number wasn't given\n");
		exit(EXIT_FAILURE);
	}
	if(strcmp(argv[1],"-h")==0)
		strcpy(host_name,argv[2]);
	else if(strcmp(argv[3],"-h")==0)
		strcpy(host_name,argv[4]);
	else if(strcmp(argv[5],"-h")==0)
		strcpy(host_name,argv[6]);
	else
	{
		printf("Host name wasn't given\n");
		exit(EXIT_FAILURE);
	}
	if(strcmp(argv[1],"-i")==0)
		fp_commands=fopen(argv[2],"r");
	else if(strcmp(argv[3],"-i")==0)
		fp_commands=fopen(argv[4],"r");
	else if(strcmp(argv[5],"-i")==0)
		fp_commands=fopen(argv[6],"r");
	else
	{
		printf("Command file wasn't given\n");
		exit(EXIT_FAILURE);
	}
	
	int sock;
	unsigned int serverlen;
	struct sockaddr_in server;
	struct sockaddr *serverptr;
	struct hostent *rem;

	if((sock=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		perror("Socket creation");
		exit(EXIT_FAILURE);
	}

	if((rem=gethostbyname(host_name))==NULL)
	{
		perror("Get host by name");
		exit(EXIT_FAILURE);
	}

	server.sin_family=AF_INET;
	bcopy((char*) rem->h_addr,(char*) &server.sin_addr,rem->h_length);
	server.sin_port=htons(port_number);
	serverptr=(struct sockaddr*) &server;
	serverlen=sizeof(server);

	if(connect(sock,serverptr,serverlen)<0)
	{
		perror("Connect");
		exit(EXIT_FAILURE);
	}

	char *token;
	const char s[3]=" \n";						//To use in strtok
	char line[1024],fullmsg[1024],newmsg[300];
	int word=0;							//To count number of words in a line
	read_data(sock,newmsg);						//Get initial message from server
	printf("%s",newmsg);
	read_data(sock,newmsg);						//Get message that it is our turn
	printf("%s",newmsg);
	
	if(fp_commands==NULL)						//Check if file exists
	{
		printf("There is no such file\n");
		file_exists=0;
	}
	if(file_exists==1)
	{
		while(fgets(line,1024,fp_commands)!=NULL)		//Read every line
		{
			strcpy(fullmsg,line);
			token=strtok(line,s);
			if(strcmp(token,"sleep")==0)
			{
				int secs;
				while(token!=NULL)
				{
					word++;
					if(word==2)
						sscanf(token,"%d",&secs);
					token=strtok(NULL,s);
				}
				usleep(secs*1000);			//Sleep from msec
			}
			else if(strcmp(token,"exit")==0)
			{
				write_data(sock,"FINISHED");		//Send message to close socket
				exit(EXIT_SUCCESS);
			}
			else
			{
				write_data(sock,fullmsg);		//Send command
				read_data(sock,newmsg);
				printf("%s",newmsg);			//Wait for answer from client
			}
			word=0;
		}
		fclose(fp_commands);	
	}
	while(fgets(line,1024,stdin)!=NULL)
	{
		strcpy(fullmsg,line);
		token=strtok(line,s);
		if(strcmp(token,"sleep")==0)
		{
			int secs;
			while(token!=NULL)
			{
				word++;
				if(word==2)
					sscanf(token,"%d",&secs);
				token=strtok(NULL,s);
			}
			usleep(secs*1000);				//Sleep for msec
		}
		else if(strcmp(token,"exit")==0)
		{
			write_data(sock,"FINISHED");			//Send message to close socket
			exit(EXIT_SUCCESS);
		}
		else
		{
			write_data(sock,fullmsg);			//Send command
			read_data(sock,newmsg);				//Wait for answer from client
			printf("%s",newmsg);
		}
		word=0;
	}
}
