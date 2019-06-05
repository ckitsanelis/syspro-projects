#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include "list.h"

sig_atomic_t volatile	CREADY;
sig_atomic_t volatile	SREADY;

void client_handler(int signum)
{
	CREADY=1;
}

void server_handler(int signum)
{
	SREADY=1;
}

void changename(char oldname[MAXSIZE],char nfile[MAXSIZE])		//Function to check if file name exists
{
	char tmpfile[MAXSIZE];			//Needed if we have to loop twice
	const char d[5]=".\0\n";		//To use in strtok
	char* token2;
	FILE *fl;
	strcpy(nfile,oldname);
	while(1)
	{
		fl=fopen(nfile,"r");
		if(fl==NULL)			//Will only return when we have found a filename that doesn'e exist in the folder
			return;
		else
		{
			fclose(fl);
			strcpy(tmpfile,nfile);
			token2=strtok(tmpfile,d);
			strcpy(nfile,token2);
			strcat(nfile,"1");			//Add a 1 before the dot of the filename
			token2=strtok(NULL,d);
			strcat(nfile,".");
			strcat(nfile,token2);
		}
	}
}


int main (int argc, char** argv)
{
	signal(SIGINT,SIG_IGN);				//To ignore ^C
	if(argc!=2)					//Need a path
	{
		printf("Need 2 arguments\n");
		exit(EXIT_FAILURE);
	}
	int readpid=0;					//Flag to know if we have read the server pid
	char num[10];
	char checkserver[150];
	FILE *fp,*fp2;
	pid_t pid,cpid;					//cpid used to kill server process
	struct stat dir;
	if(stat(argv[1],&dir)==-1)			//If the directory doesn't exist
	{
		mkdir(argv[1],0744);			//Create the directory
		pid=fork();				//Fork to create the server
		if(pid==-1)
		{
			perror("Fork error!");
			exit(EXIT_FAILURE);
		}
	}
	else						//Directory exists
	{
		sprintf(checkserver,"%s/pid.txt",argv[1]);
		fp=fopen(checkserver,"r");		//Try to open txt file
		if(fp!=NULL)				//If it exists
		{
			fgets(num,10,fp);
			sscanf(num,"%d",&cpid);		//Get the process id
			fclose(fp);
			readpid=1;
			pid=getpid();
		}
		else					//If it has been deleted
		{
			pid=fork();			//Fork to create the server
			if(pid==-1)
			{
				perror("Fork error!");
				exit(EXIT_FAILURE);
			}
		}
	}
	//Father process(Board client)
	if(pid!=0)
	{
		signal(SIGUSR1,&client_handler);
		char *token;
		const char s[3]=" \n";		//To use in strtok
		char line[MAXSIZE];
		char fullline[MAXSIZE];
		char msgreceive[MAXSIZE];
		char msgserver[50];
		int word=0;			//To count number of words in a line
		struct stat st;

		int fd_b1,fd_b2;
		char pipe[150];
		sprintf(pipe,"%s/_servertoboard",argv[1]);
		if(readpid==0)				//If pipes haven't previously been created
		{
			while(!CREADY)			//wait for server to create pipes
				sleep(1);
		}

		if((fd_b1=open(pipe,O_RDONLY | O_NONBLOCK))<0)
		{
			perror("Named pipe open error");
			exit(EXIT_FAILURE);
		}
		if(readpid==0)
			kill(0,SIGUSR1);
		sprintf(pipe,"%s/_boardtoserver",argv[1]);
		if((fd_b2=open(pipe,O_WRONLY))<0)
		{
			perror("Named pipe open error");
			exit(EXIT_FAILURE);
		}

		printf("Maximum message size is %d\n",MAXSIZE);
		while(fgets(line,256,stdin)!=NULL)		//Read whatever the user inputs
		{
			word=0;
			if(strcmp(line,"\n")==0)
				continue;
			if(strlen(line)>MAXSIZE)
			{
				printf("Message is too big.Maximum size is %d\n",MAXSIZE);
				fflush(stdout);
				continue;
			}
			strcpy(fullline,line);		//Copy to other string as strtok will break it into a series of strings
			token=strtok(line,s);
//Making sure each command exists and has correct number of arguments, then writes to the pipe
			if(strcmp(token,"createchannel")==0)
			{
				while(token!=NULL)
				{
					word++;
					token=strtok(NULL,s);
				}
				if(word!=3)
				{
					printf("Createchannel needs id and name\n");
					continue;
				}
				if(write(fd_b2,fullline,MAXSIZE)<0)
				{
					perror("Error in writing");
					exit(EXIT_FAILURE);
				}
				while(1)
				{
					if(read(fd_b1,msgreceive,MAXSIZE)>0)
					{
						if(strcmp(msgreceive,"channelcreated")==0)
						{
							printf("Channel created\n");
							break;
						}
						else if(strcmp(msgreceive,"channelexists")==0)
						{
							printf("There is already a channel with the same id\n");
							break;
						}
					}
				}
			}
			else if(strcmp(token,"getmessages")==0)
			{
				while(token!=NULL)
				{
					word++;
					token=strtok(NULL,s);
				}
				if(word!=2)
				{
					printf("Getmessages needs id\n");
					continue;
				}
				if(write(fd_b2,fullline,MAXSIZE)<0)
				{
					perror("Error in writing");
					exit(EXIT_FAILURE);
				}
				while(1)
				{
					if(read(fd_b1,msgreceive,MAXSIZE)>0)
					{
						if(strcmp(msgreceive,"badbreak")==0)
						{
							printf("There is no such channel\n");
							break;
						}
						else if(strcmp(msgreceive,"nostuff")==0)
						{
							printf("There aren't any messages or files in the channel\n");
							break;
						}
						else if(strcmp(msgreceive,"Files:")==0)		//When this is sent, only files will be sent from now on
						{
							printf("%s\n",msgreceive);
							strcpy(msgserver,"gotit");
							write(fd_b2,msgserver,10);
							int newfile;
							char newname[220],tempname[220];
							size_t cbytes;
							while(1)
							{
								if(read(fd_b1,msgreceive,MAXSIZE)>0)
								{
									if(strcmp(msgreceive,"endoffiles")==0)
										break;
									if(strcmp(msgreceive,"filestop")==0)
										continue;
									sprintf(tempname,"%s/%s",argv[1],msgreceive);
									changename(tempname,newname);			//Change it to an acceptable one
									newfile=open(newname,O_CREAT | O_WRONLY | O_APPEND,0666);	//Create the file
									strcpy(msgserver,"continue");			//Inform the user to send the file data
									if(write(fd_b2,msgserver,MAXSIZE)<0)
									{
										perror("Error in writing");
										exit(EXIT_FAILURE);
									}
									while(1)
									{
										cbytes=read(fd_b1,msgreceive,MAXSIZE);
										if(cbytes==-1)
											continue;
										if(cbytes>0)
										{
											if(strcmp(msgreceive,"filestop")==0)
											{
												close(newfile);
												break;
											}
											sprintf(msgreceive,"%.*s",(int)cbytes,msgreceive);
											write(newfile,msgreceive,cbytes);
											strcpy(msgserver,"goon");				//Inform user we are ready for next packet
											write(fd_b2,msgserver,10);
										}
									}
									printf("New file %s\n",newname);
								}
								else
									continue;
							}
							if(strcmp(msgreceive,"endoffiles")==0)
									break;
						}
						else if(strcmp(msgreceive,"endoffiles")==0)
							break;
						else				//This will only print messages
							printf("%s\n",msgreceive);
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
			else if(strcmp(token,"shutdown")==0)
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
				if(write(fd_b2,fullline,MAXSIZE)<0)
				{
					perror("Error in writing");
					exit(EXIT_FAILURE);
				}
				exit(EXIT_SUCCESS);
			}
			else
				printf("There is no %s command\n",token);
		}
	}


	//Child process(Board server)
	else
	{
		signal(SIGUSR1,&server_handler);
		int cbytes,bpbytes;
		int fd_s1,fd_s2,fd_s3,fd_s4;
		char msgbuf[MAXSIZE];
		char msgbuf2[MAXSIZE];
		char msgclient[MAXSIZE];
		char msgpost[MAXSIZE];
		char pipe2[150];

		//Creating the named pipes
		sprintf(pipe2,"%s/_boardtoserver",argv[1]);
		if(mkfifo(pipe2,0666)==-1)
		{
			if(errno!=EEXIST)
			{
				perror("mkfifo error!");
				exit(EXIT_FAILURE);
			}
		}
		sprintf(pipe2,"%s/_posttoserver",argv[1]);
		if(mkfifo(pipe2,0666)==-1)
		{
			if(errno!=EEXIST)
			{
				perror("mkfifo error!");
				exit(EXIT_FAILURE);
			}
		}
		sprintf(pipe2,"%s/_servertoboard",argv[1]);
		if(mkfifo(pipe2,0666)==-1)
		{
			if(errno!=EEXIST)
			{
				perror("mkfifo error!");
				exit(EXIT_FAILURE);
			}
		}
		sprintf(pipe2,"%s/_servertopost",argv[1]);
		if(mkfifo(pipe2,0666)==-1)
		{
			if(errno!=EEXIST)
			{
				perror("mkfifo error!");
				exit(EXIT_FAILURE);
			}
		}

		//Connecting the file descriptors with the named pipes
		sprintf(pipe2,"%s/_boardtoserver",argv[1]);
		if((fd_s1=open(pipe2,O_RDONLY | O_NONBLOCK))==-1)
		{
			perror("Named pipe open error");
			exit(EXIT_FAILURE);
		}

		sprintf(pipe2,"%s/_posttoserver",argv[1]);
		if((fd_s2=open(pipe2,O_RDONLY | O_NONBLOCK))<0)
		{
			perror("Named pipe open error");
			exit(EXIT_FAILURE);
		}

		kill(getppid(),SIGUSR1);
		while(!SREADY)			//wait for clint to read pipe
			sleep(1);
		sprintf(pipe2,"%s/_servertoboard",argv[1]);
		if((fd_s3=open(pipe2,O_WRONLY))<0)
		{
			perror("Named pipe open error");
			exit(EXIT_FAILURE);
		}
		sprintf(pipe2,"%s/_servertopost",argv[1]);

		//Saving server pid at txt file in path
		char path[150];
		sprintf(path,"%s/pid.txt",argv[1]);
		fp2=fopen(path,"w");
		fprintf(fp2,"%d\n",getpid());
		fclose(fp2);

		int connected=0;
		char line1[MAXSIZE],line2[MAXSIZE];
		char *token;
		const char s[3]=" \n";		//To use in strtok
		int word=0;			//To count number of words in a line
		ptr_channel_list mychannel_list;
		mychannel_list=create_channels();
		while(1)
		{
			cbytes=read(fd_s1,msgbuf,MAXSIZE);
			if(cbytes>0)				//Only if it has successfully read from client
			{
				strcpy(line1,msgbuf);
				word=0;
				token=strtok(line1,s);
				if(strcmp(token,"createchannel")==0)	//And compare the first word with the available commands
				{
					unsigned long id;
					char name[100];
					while(token!=NULL)
					{
						word++;
						if(word==2)
							sscanf(token,"%li",&id);
						else if(word==3)
							strcpy(name,token);
						token=strtok(NULL,s);
					}
					if((insert_channel(mychannel_list,id,name))==1)
						sprintf(msgclient,"channelcreated");
					else
						sprintf(msgclient,"channelexists");
					if(write(fd_s3,msgclient,MAXSIZE)<0)
					{
						perror("Error in writing");
						exit(EXIT_FAILURE);
					}
				}
				else if(strcmp(token,"getmessages")==0)
				{
					unsigned long id;
					while(token!=NULL)
					{
						word++;
						if(word==2)
							sscanf(token,"%li",&id);
						token=strtok(NULL,s);
					}
					ptr_channel temp_channel;		//Check if channel exists
					temp_channel=search_channel(mychannel_list,id);
					if(temp_channel==NULL)			//Channel doesn't exist
					{
						sprintf(msgclient,"badbreak");
						if(write(fd_s3,msgclient,MAXSIZE)<0)
						{
							perror("Error in writing");
							exit(EXIT_FAILURE);
						}
					}
					else if(temp_channel->message_list->head_node==NULL && temp_channel->file_list->head_node==NULL)			//Channel is empty
						{
						sprintf(msgclient,"nostuff");
						if(write(fd_s3,msgclient,MAXSIZE)<0)
						{
							perror("Error in writing");
							exit(EXIT_FAILURE);
						}
					}
					else
					{
						ptr_node temp_node;		//Go to start of the message list
						temp_node=temp_channel->message_list->head_node;
						if(temp_node!=NULL)
						{
							sprintf(msgclient,"Messages:");
							if(write(fd_s3,msgclient,MAXSIZE)<0)
							{
								perror("Error in writing");
								exit(EXIT_FAILURE);
							}
						}
						while(temp_node!=NULL)
						{				//And pass down the pipe each message
							strcpy(msgclient,temp_node->message);
							if(write(fd_s3,msgclient,MAXSIZE)<0)
							{
								perror("Error in writing");
								exit(EXIT_FAILURE);
							}
							temp_node=temp_node->next_node;
						}
						delete_messages(temp_channel->message_list);
						temp_node=temp_channel->file_list->head_node; 		//Go to the start of the file list
						if(temp_node!=NULL)					//This means at least 1 file exists
						{
							sprintf(msgclient,"Files:");
							if(write(fd_s3,msgclient,MAXSIZE)<0)
							{
								perror("Error in writing");
								exit(EXIT_FAILURE);
							}
							while(1)
							{
								if(read(fd_s1,msgbuf,MAXSIZE)>0)
								{
									if(strcmp(msgbuf,"gotit")==0)
										break;
								}
							}
						}
						while(temp_node!=NULL)					//Go through the whole file list
						{
							strcpy(msgclient,temp_node->name);		//Send the file name first
							if(write(fd_s3,msgclient,MAXSIZE)<0)
							{
								perror("Error in writing");
								exit(EXIT_FAILURE);
							}
							while(1)					//Then send the file in packets
							{
								if(read(fd_s1,msgbuf,MAXSIZE)>0)
								{
									if(strcmp(msgbuf,"continue")==0)
									{
										size_t abytes=0;
										int sizesent=0;
										int remaining=strlen(temp_node->filedat);
										while(1)
										{
											if(remaining>MAXSIZE)
												abytes=write(fd_s3,temp_node->filedat+sizesent,MAXSIZE);
											else
												abytes=write(fd_s3,temp_node->filedat+sizesent,remaining);
											if(abytes==-1)
												continue;
											if(abytes>0)
											{
												sizesent=sizesent+abytes;
												remaining=remaining-abytes;
												while(1)
												{
													if(read(fd_s1,msgbuf,MAXSIZE)>0)
													{
														if(strcmp(msgbuf,"goon")==0)		//Wait for message from client
															break;
													}
												}
											}
											if(remaining==0)
												break;
										}
										write(fd_s3,"filestop",15);
										break;
									}
								}
							}
							temp_node=temp_node->next_node;
						}
						strcpy(msgclient,"endoffiles");
						if(write(fd_s3,msgclient,MAXSIZE)<0)
						{
							perror("Error in writing");
							exit(EXIT_FAILURE);
						}
						delete_files(temp_channel->file_list);
					}
				}
				else if(strcmp(token,"shutdown")==0)
				{
					destroy_channels(mychannel_list);
					char rmfile[150];
					sprintf(rmfile,"%s/_boardtoserver",argv[1]);	//Delete all pipes
					unlink(rmfile);
					sprintf(rmfile,"%s/_servertoboard",argv[1]);
					unlink(rmfile);
					sprintf(rmfile,"%s/_posttoserver",argv[1]);
					unlink(rmfile);
					sprintf(rmfile,"%s/_servertopost",argv[1]);
					unlink(rmfile);
					sprintf(rmfile,"%s/pid.txt",argv[1]);
					unlink(rmfile);			//Don't unlink directory so it can be used in bash script
					exit(EXIT_SUCCESS);
				}
			}

			bpbytes=read(fd_s2,msgbuf2,MAXSIZE);
			if(bpbytes>0)
			{
				if(connected==0)			//Only after the first successful read from boardpost
				{
					if((fd_s4=open(pipe2,O_WRONLY))<0)
					{
						perror("Named pipe open error");
						exit(EXIT_FAILURE);
					}
					connected=1;
				}
				strcpy(line2,msgbuf2);
				word=0;
				token=strtok(line2,s);
				if(strcmp(token,"list")==0)	//And compare the first word with the available commands
				{
					ptr_channel temp_channel;
					temp_channel=mychannel_list->head_channel;
					if(temp_channel==NULL)					//List is empty, so no channels exist
					{
						strcpy(msgpost,"No available channels\n");
						if(write(fd_s4,msgpost,MAXSIZE)<0)		//Inform user
						{
							perror("Error in writing");
							exit(EXIT_FAILURE);
						}
					}
					else
					{
						sprintf(msgpost,"Available channels:\n");
						if(write(fd_s4,msgpost,MAXSIZE)<0)
						{
							perror("Error in writing");
							exit(EXIT_FAILURE);
						}
						while(temp_channel!=NULL)
						{
							sprintf(msgpost,"ID:%li Name:%s\n",temp_channel->id,temp_channel->name);
							if(write(fd_s4,msgpost,MAXSIZE)<0)
							{
								perror("Error in writing");
								exit(EXIT_FAILURE);
							}
							temp_channel=temp_channel->next_channel;
						}
					}
					sprintf(msgpost,"break12345");
					if(write(fd_s4,msgpost,MAXSIZE)<0)
					{
						perror("Error in writing");
						exit(EXIT_FAILURE);
					}
				}
				else if(strcmp(token,"write")==0)
				{
					unsigned long id;
					char str[MAXSIZE];
					while(token!=NULL)
					{
						word++;
						if(word==2)
							sscanf(token,"%li",&id);
						else if(word==3)
							strcpy(str,token);
						else if(word>3)				//Construct message in new string
						{
							strcat(str," ");
							strcat(str,token);
						}
						token=strtok(NULL,s);
					}
					ptr_channel temp_channel;		//Search for requested channel
					temp_channel=search_channel(mychannel_list,id);
					if(temp_channel==NULL)			//If it doesn't exist
					{
						sprintf(msgpost,"badbreak");
						if(write(fd_s4,msgpost,MAXSIZE)<0)		//Inform the user
						{
							perror("Error in writing");
							exit(EXIT_FAILURE);
						}
					}
					else					//If it does exist
					{
						insert_message(temp_channel->message_list,str); 	//Add message to specific channel message list
						sprintf(msgpost,"break12345");				//Inform the user of successful transfer
						if(write(fd_s4,msgpost,MAXSIZE)<0)
						{
							perror("Error in writing");
							exit(EXIT_FAILURE);
						}
					}
				}
				else if(strcmp(token,"send")==0)
				{
					unsigned long id;
					char file[200];
					char *filedata,*nfile;
					unsigned long filesize;
					unsigned long sizeread=0;
					size_t rbytes;
					while(token!=NULL)
					{
						word++;
						if(word==2)
							sscanf(token,"%li",&id);
						else if(word==3)
							strcpy(file,token);
						else if(word==4)
							sscanf(token,"%li",&filesize);
						token=strtok(NULL,s);
					}
					ptr_channel temp_channel;
					ptr_node temp_node;
					temp_channel=search_channel(mychannel_list,id);		//Search for requested channel
					if(temp_channel==NULL)		//If it doesn't exist
					{
						sprintf(msgpost,"badbreak");			//Inform the user
						if(write(fd_s4,msgpost,MAXSIZE)<0)
						{
							perror("Error in writing");
							exit(EXIT_FAILURE);
						}
					}
					else				//If it does exist
					{
						sprintf(msgpost,"continue");			//Inform the user to send the file data
						if(write(fd_s4,msgpost,MAXSIZE)<0)
						{
							perror("Error in writing");
							exit(EXIT_FAILURE);
						}
						nfile=basename(file);			//Keep only file name and not path
						temp_node=insert_file(temp_channel->file_list,nfile,filesize);
						while(1)			//Once the user has sent the file data
						{
							rbytes=read(fd_s2,msgbuf2,MAXSIZE);
							if(rbytes==-1)
								continue;
							else if(rbytes==0)
								break;
							else if(rbytes>0)
							{
								sprintf(msgbuf2,"%.*s",(int)rbytes,msgbuf2);
								memcpy(temp_node->filedat+sizeread,msgbuf2,MAXSIZE);
								sizeread=sizeread+rbytes;
								sprintf(msgpost,"goon");				//Inform user we are ready for next packet
								write(fd_s4,msgpost,10);
							}
							if(rbytes<MAXSIZE)
								break;
						}
						sprintf(msgpost,"break12345");			//Inform the user of successful transfer
						if(write(fd_s4,msgpost,MAXSIZE)<0)
						{
							perror("Error in writing");
							exit(EXIT_FAILURE);
						}
					}
				}
			}
		}
	}
}
