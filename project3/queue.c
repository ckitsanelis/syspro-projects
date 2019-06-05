#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "queue.h"

Qlist* create_queue(int size)
{
	if(size<1)
	{
		printf("Can't create queue with negative or zero size\n");
		return NULL;
	}
	Qlist *new_queue;
	new_queue=malloc(sizeof(Qlist));		//Allocate necessary memory
	new_queue->head=NULL;				//And make it empty
	new_queue->maxsize=size;
	new_queue->curr_size=0;
	return new_queue;
}

int empty_queue(Qlist *queue)
{
	if(queue->head==NULL)				//Check if it empty
		return 1;
	else
		return 0;
}

int add_to_queue(Qlist *queue,int newfd)
{
	if(queue->maxsize==queue->curr_size)		//Queue is full, can't insert string
	{
		printf("CANT ADD:FULL QUEUE\n");
		return 0;
	}
	Qnode *new_node;
	new_node=malloc(sizeof(Qnode));			//Allocate memory for new node
	new_node->fds=newfd;				//Copy the integer to new node
	new_node->next=NULL;
	queue->curr_size=queue->curr_size+1;		//Increase current queue size
	if(queue->head==NULL)
		queue->head=new_node;
	else
	{
		Qnode *tmp_node;
		tmp_node=queue->head;
		while(tmp_node->next!=NULL)
			tmp_node=tmp_node->next;
		tmp_node->next=new_node;
	}
	return 1;
}

void remove_from_queue(Qlist *queue)
{
	if(queue->head==NULL)
	{
		printf("CANT REMOVE:EMPTY QUEUE\n");
		return;
	}
	Qnode *tmp_node;
	tmp_node=queue->head;
	queue->head=tmp_node->next;
	free(tmp_node);
	queue->curr_size=queue->curr_size-1;
}

void delete_queue(Qlist *queue)
{
	if(queue->head==NULL)
	{
		free(queue);
		return;
	}
	Qnode *tmp_node,*tmp_node2;
	tmp_node=queue->head;
	while(tmp_node!=NULL)
	{
		tmp_node2=tmp_node;
		tmp_node=tmp_node->next;
		free(tmp_node2);
	}
	free(queue);
}

void print_queue(Qlist *queue)
{
	Qnode *tmp_node;
	tmp_node=queue->head;printf("QUEUE\n");
	while(tmp_node!=NULL)
	{
		printf("%d\n",tmp_node->fds);
		tmp_node=tmp_node->next;
	}printf("END OF QUEUE\n");
}

//________________________________________________________________________________
//for the queue used to keep the names in multi functions

MQlist* create_multiqueue()
{
	MQlist *new_queue;
	new_queue=malloc(sizeof(MQlist));				//Allocate necessary memory
	new_queue->head=NULL;						//And make it empty
	new_queue->curr_size=0;
	return new_queue;
}

int empty_multiqueue(MQlist *queue)
{
	if(queue->head==NULL)						//Check if it empty
		return 1;
	else
		return 0;
}

int add_to_multiqueue(MQlist *queue,char new_name[100])
{
	MQnode *new_node;
	new_node=malloc(sizeof(MQnode));					//Allocate memory for new node
	strcpy(new_node->name,new_name);				//Copy the name to new node
	new_node->next=NULL;
	if(queue->head==NULL)
		queue->head=new_node;
	else
	{
		MQnode *tmp_node;
		tmp_node=queue->head;
		if(strcmp(tmp_node->name,new_name)==0)
			return 0;
		while(tmp_node->next!=NULL)
		{
			tmp_node=tmp_node->next;			//Check the whole list
			if(strcmp(tmp_node->name,new_name)==0)		//Name already exists in queue
				return 0;
		}
		tmp_node->next=new_node;				//Name doesn't exist, so we can add it
	}
	queue->curr_size=queue->curr_size+1;				//Increase current queue size
	return 1;
}

void delete_multiqueue(MQlist *queue)
{
	if(queue->head==NULL)
	{
		free(queue);
		return;
	}
	MQnode *tmp_node,*tmp_node2;
	tmp_node=queue->head;
	while(tmp_node!=NULL)
	{
		tmp_node2=tmp_node;
		tmp_node=tmp_node->next;
		free(tmp_node2);
	}
	free(queue);
}
