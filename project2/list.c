#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "list.h"

//Functions for the list of messages

ptr_list create_list()
{
	ptr_list new;
	new=malloc(sizeof(list_head));
	new->size=0;
	new->head_node=NULL;
	return new;
}

void destroy_list(ptr_list mylist)
{
	ptr_node temp,del;
	temp=mylist->head_node;
	while(temp!=NULL)
	{
		del=temp;
		temp=temp->next_node;
		free(del);
	}
}

int empty_list(ptr_list mylist)
{
	if(mylist->head_node==NULL)
		return 1;		//List is empty
	else
		return 0;		//List isn't empty
}

void insert_message(ptr_list mylist,char str[MAXSIZE])
{
	ptr_node new_node;
	new_node=malloc(sizeof(list_node));
	strcpy(new_node->message,str);
	if(mylist->head_node==NULL)
	{
		mylist->head_node=new_node;
		new_node->next_node=NULL;
		mylist->size++;
	}
	else
	{
		ptr_node temp;
		temp=mylist->head_node;
		while(temp->next_node!=NULL)
			temp=temp->next_node;
		temp->next_node=new_node;
		new_node->next_node=NULL;
		mylist->size++;
	}
}

ptr_node insert_file(ptr_list mylist,char strname[220],unsigned long size)
{
	ptr_node new_node;
	new_node=malloc(sizeof(list_node));
	strcpy(new_node->name,strname);
	new_node->filedat=malloc(size);
	if(mylist->head_node==NULL)
	{
		mylist->head_node=new_node;
		new_node->next_node=NULL;
		mylist->size++;
	}
	else
	{
		ptr_node temp;
		temp=mylist->head_node;
		while(temp->next_node!=NULL)
			temp=temp->next_node;
		temp->next_node=new_node;
		new_node->next_node=NULL;
		mylist->size++;
	}
	return new_node;
}

void delete_messages(ptr_list mylist)
{
	if(empty_list(mylist)==1)
		return;
	ptr_node temp,del;
	temp=mylist->head_node;
	while(temp!=NULL)
	{
		del=temp;
		temp=temp->next_node;
		free(del);
	}
	mylist->head_node=NULL;
	mylist->size=0;
}

void delete_files(ptr_list mylist)
{
	if(empty_list(mylist)==1)
		return;
	ptr_node temp,del;
	temp=mylist->head_node;
	while(temp!=NULL)
	{
		del=temp;
		temp=temp->next_node;
		free(del->filedat);
		free(del);
	}
	mylist->head_node=NULL;
	mylist->size=0;
}

//From here on the functions are for the list of channels

ptr_channel_list create_channels()
{
	ptr_channel_list new;
	new=malloc(sizeof(channels_head));
	new->size=0;
	new->head_channel=NULL;
	return new;
}

void destroy_channels(ptr_channel_list mylist)
{
	ptr_channel temp,del;
	temp=mylist->head_channel;
	while(temp!=NULL)
	{
		del=temp;
		temp=temp->next_channel;
		destroy_list(del->message_list);
		destroy_list(del->file_list);
		free(del);
	}
}

int empty_channels(ptr_channel_list mylist)
{
	if(mylist->head_channel==NULL)
		return 1;		//List is empty
	else
		return 0;		//List isn't empty
}

ptr_channel search_channel(ptr_channel_list mylist,unsigned long num)
{
	if(empty_channels(mylist)==1)
		return NULL;
	ptr_channel temp;
	temp=mylist->head_channel;
	while(temp!=NULL)
	{
		if(temp->id==num)
			return temp;
		temp=temp->next_channel;
	}
	return NULL;
}

int insert_channel(ptr_channel_list mylist,unsigned long num,char chname[100])
{
	ptr_channel new_channel;
	new_channel=search_channel(mylist,num);	//Checking if there is already a channel with the id
	if(new_channel!=NULL)
		return 0;
	new_channel=malloc(sizeof(channels_node));
	strcpy(new_channel->name,chname);
	new_channel->id=num;
	new_channel->message_list=create_list();
	new_channel->file_list=create_list();
	if(mylist->head_channel==NULL)
	{
		new_channel->next_channel=NULL;
		mylist->head_channel=new_channel;
		mylist->size++;
	}
	else
	{
		ptr_channel temp;
		temp=mylist->head_channel;
		while(temp->next_channel!=NULL)
			temp=temp->next_channel;
		new_channel->next_channel=NULL;
		temp->next_channel=new_channel;
		mylist->size++;
	}
	return 1;
}
