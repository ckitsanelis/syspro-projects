#ifndef STACK_H_
#define STACK_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct stack_node stack_node;

struct stack_node
{
	unsigned long key;
	int success;
	stack_node *next;
};

stack_node *create_stack(unsigned long num);
void push(unsigned long num,stack_node *head);
void pop(stack_node *head);
void print_stack(stack_node *head);
void trace_print(stack_node *head,double ammount);
void delete_stack(stack_node *head);

#endif
