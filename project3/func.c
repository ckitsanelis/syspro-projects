#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int read_data(int fd,char* buff)
{
	int i=0,length=0,bytes_read=0;
	if(read(fd,&length,sizeof(int))<=0)			//Get length of message
		exit(-3);
	while(i<length)						//Read number of length
	{
		if((bytes_read=read(fd,&buff[i],length-i))<0)		
			exit(-3);
		i=i+bytes_read;
	}
	return i;
}

int write_data(int fd,char* buff)
{
	int length=0;
	length=strlen(buff)+1;					//Length of message
	if(write(fd,&length,sizeof(int))<0)			//Send length first
		exit(-2);
	if(write(fd,buff,length)<0)				//Send message after
		exit(-2);
	return length;
}
