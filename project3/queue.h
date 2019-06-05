#ifndef QUEUE_H_
#define QUEUE_H_

typedef struct Queue_node Qnode;
typedef struct Queue_list Qlist;

struct Queue_node
{
	int fds;
	struct Queue_node *next;
};

struct Queue_list
{
	int maxsize;
	int curr_size;
	struct Queue_node *head;
};

Qlist* create_queue(int size);
int empty_queue(Qlist *queue);
int add_to_queue(Qlist *queue,int newfd);
void remove_from_queue(Qlist *queue);
void delete_queue(Qlist *queue);
void print_queue(Qlist *queue);

typedef struct MultiQueue_node MQnode;
typedef struct MultiQueue_list MQlist;

struct MultiQueue_node
{
	char name[100];
	struct MultiQueue_node *next;
};

struct MultiQueue_list
{
	int curr_size;
	struct MultiQueue_node *head;
};


MQlist* create_multiqueue();
int empty_multiqueue(MQlist *queue);
int add_to_multiqueue(MQlist *queue,char new_name[100]);
void delete_multiqueue(MQlist *queue);

#endif
