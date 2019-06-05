#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include "list.h"


int main (int argc, char** argv)
{
	signal(SIGINT,SIG_IGN);
	if(argc!=2)
	{
		printf("Need 2 arguments\n");
		exit(EXIT_FAILURE);
	}
	char serverpid[150];
	char num[10];
	FILE *fp;
	pid_t spid;
	sprintf(serverpid,"%s/pid.txt",argv[1]);
	fp=fopen(serverpid,"r");		//Try to open txt file
	if(fp==NULL)			//If the txt file doesn't exist
	{
		printf("There is no such active board!\n");	//Either the server has shutdown
		exit(EXIT_FAILURE);				//Or it never existed
	}
	fclose(fp);

	char *token;
	const char s[3]=" \n";		//To use in strtok
	char line[MAXSIZE];
	char msgsend[MAXSIZE];
	char msgreceive[MAXSIZE];
	int word=0;			//To count number of words in a line

	char pipe[150];
	int fd_bp1,fd_bp2;
	sprintf(pipe,"%s/_servertopost",argv[1]);
	if((fd_bp1=open(pipe,O_RDONLY | O_NONBLOCK))<0)
	{
		perror("Named pipe open error");
		exit(EXIT_FAILURE);
	}
	sprintf(pipe,"%s/_posttoserver",argv[1]);
	if((fd_bp2=open(pipe,O_WRONLY))<0)
	{
		perror("Named pipe open error");
		exit(EXIT_FAILURE);
	}

	printf("Maximum message size is %d\n",MAXSIZE);
	while(fgets(line,256,stdin)!=NULL)	//Read whatever the user inputs
	{
		fp=fopen(serverpid,"r");		//Check server still exists
		if(fp==NULL)
		{
			printf("Server has been shutdown\n");
			exit(EXIT_FAILURE);
		}
		else
			fclose(fp);

		if(strcmp(line,"\n")==0)
			continue;
		if(strlen(line)>MAXSIZE)
		{
			printf("Message is too big.Maximum size is %d\n",MAXSIZE);
			fflush(stdout);
			continue;
		}
		strcpy(msgsend,line);		//Copy to other string as strtok will break it into a series of strings
		word=0;
		token=strtok(line,s);
		if(strcmp(token,"list")==0)	//Making sure each command exists and has correct number of arguments, then writes to the pipe
		{
			while(token!=NULL)
			{
				word++;
				token=strtok(NULL,s);
			}
			if(word!=1)
			{
				printf("Incorrect command\n");
				continue;
			}
			if(write(fd_bp2,msgsend,MAXSIZE)<0)
			{
				perror("Error in writing");
				exit(EXIT_FAILURE);
			}
			while(1)
			{
				if(read(fd_bp1,msgreceive,MAXSIZE)>0)
				{
					if(strcmp(msgreceive,"break12345")!=0)
						printf("%s",msgreceive);
					else
						break;
				}
			}
		}
		else if(strcmp(token,"write")==0)
		{
			while(token!=NULL)
			{
				word++;
				token=strtok(NULL,s);
			}
			if(word<3)
			{
				printf("Write needs id and message\n");
				continue;
			}
			if(write(fd_bp2,msgsend,MAXSIZE)<0)
			{
				perror("Error in writing");
				exit(EXIT_FAILURE);
			}
			while(1)
			{
				if(read(fd_bp1,msgreceive,MAXSIZE)>0)
				{
					if(strcmp(msgreceive,"break12345")==0)
					{
						printf("Message sent successfully\n");
						break;
					}
					if(strcmp(msgreceive,"badbreak")==0)
					{
						printf("There is no such channel\n");
						break;
					}
				}
			}
		}
		else if(strcmp(token,"send")==0)
		{
			char fname[200];
			int fk;
			int filesize;
			size_t nbytes;
			unsigned long sizeread=0;
			char filepiece[MAXSIZE];
			struct stat st;
			while(token!=NULL)
			{
				word++;
				if(word==3)
					strcpy(fname,token);
				token=strtok(NULL,s);
			}
			if(word!=3)
			{
				printf("Send needs id and file\n");
				continue;
			}
			fk=open(fname, O_RDONLY);
			if(fk<0)
			{
				printf("Cannot open file\n");
				continue;
			}
			stat(fname,&st);
			filesize=st.st_size;
			strtok(msgsend,"\n");
			sprintf(msgsend,"%s %d\n",msgsend,filesize);
			if(write(fd_bp2,msgsend,MAXSIZE)<0)
			{
				perror("Error in writing");
				exit(EXIT_FAILURE);
			}
			while(1)
			{
				if(read(fd_bp1,msgreceive,MAXSIZE)>0)
				{
					if(strcmp(msgreceive,"continue")==0)
					{
						while(sizeread<=filesize)			//While we haven't sent all the packages
						{
							nbytes=read(fk,filepiece,MAXSIZE);	//Read file data
							if(nbytes==-1)
								continue;
							else if(nbytes==0)
								break;
							else if(nbytes>0)
							{
								sprintf(filepiece,"%.*s",(int)nbytes,filepiece);
								sizeread=sizeread+nbytes;
								if(write(fd_bp2,&filepiece,nbytes)<0)	//Pass it down the pipe
								{
									perror("Error in writing");
									exit(EXIT_FAILURE);
								}
								while(1)							//Before sending nect packet
								{
									if(read(fd_bp1,msgreceive,MAXSIZE)>0)
									{
										if(strcmp(msgreceive,"goon")==0)		//Wait for message from server
											break;
									}
								}
							}
						}
						close(fk);
					}
					else if(strcmp(msgreceive,"break12345")==0)
					{
						printf("File sent successfully\n");
						break;
					}
					else if(strcmp(msgreceive,"badbreak")==0)
					{
						printf("There is no such channel\n");
						break;
					}
				}
			}
		}
		else if(strcmp(token,"exit")==0)
		{
			while(token!=NULL)
			{
				word++;
				token=strtok(NULL,s);
			}
			if(word!=1)
			{
				printf("Incorrect command\n");
				continue;
			}
			exit(EXIT_SUCCESS);
		}
		else
			printf("There is no %s command\n",token);
	}
}
