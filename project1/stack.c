#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stack.h"

stack_node *create_stack(unsigned long num)
{
	stack_node *stack_head;
	stack_head=malloc(sizeof(stack_node));
	stack_head->key=num;
	stack_head->next=NULL;
	stack_head->success=0;
	return stack_head;
}

void push(unsigned long num,stack_node *head)
{
	stack_node *temp_node,*new_node;
	temp_node=head;
	while(temp_node->next!=NULL)		//Find the last node in the stack
		temp_node=temp_node->next;
	new_node=malloc(sizeof(stack_node));
	new_node->key=num;
	new_node->next=NULL;
	temp_node->next=new_node;		//Place the new one at the end
}

void pop(stack_node *head)
{
	stack_node *temp_node,*rem_node;
	temp_node=head;
	rem_node=head;
	while(rem_node->next!=NULL)		//Find the last node in the stack
		rem_node=rem_node->next;
	while(temp_node->next!=rem_node)	//Find the node before the last one
		temp_node=temp_node->next;
	temp_node->next=NULL;
	free(rem_node);
}

void print_stack(stack_node *head)		//Prints paths
{
	stack_node *temp_node;
	temp_node=head;
	printf("(");
	while(temp_node!=NULL)
	{
		if(temp_node->next!=NULL)
			printf("%li,",temp_node->key);
		else
			printf("%li)\n",temp_node->key);
		temp_node=temp_node->next;
	}
}

void trace_print(stack_node *head,double ammount)	//Prints paths with the overall ammount
{
	stack_node *temp_node;
	temp_node=head;
	printf("(");
	while(temp_node!=NULL)
	{
		printf("%li,",temp_node->key);
		temp_node=temp_node->next;
	}
	printf("%.0f)\n",ammount);
}

void delete_stack(stack_node *head)
{
	stack_node *temp_node,*del_node;
	temp_node=head;
	while(temp_node!=NULL)
	{
		del_node=temp_node;
		temp_node=temp_node->next;
		free(del_node);
	}
}
