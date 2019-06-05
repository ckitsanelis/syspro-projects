#ifndef LIST_H
#define LIST_H

#define MAXSIZE 1000

typedef struct list_head* ptr_list;
typedef struct list_node* ptr_node;

typedef struct channels_head* ptr_channel_list;
typedef struct channels_node* ptr_channel;

typedef struct list_head
{
	int size;
	ptr_node head_node;
} list_head;

typedef struct list_node
{
	char name[200];
	char message[MAXSIZE];
	char *filedat;
	ptr_node next_node;
} list_node;

typedef struct channels_head
{
	int size;
	ptr_channel head_channel;
} channels_head;

typedef struct channels_node
{
	unsigned long id;
	char name[100];
	ptr_list message_list;
	ptr_list file_list;
	ptr_channel next_channel;
} channels_node;


ptr_list create_list();
void destroy_list(ptr_list mylist);
int empty_list(ptr_list mylist);
void insert_message(ptr_list mylist,char str[MAXSIZE]);
ptr_node insert_file(ptr_list mylist,char strname[220],unsigned long size);
void delete_messages(ptr_list mylist);
void delete_files(ptr_list mylist);


ptr_channel_list create_channels();
void destroy_channels(ptr_channel_list mylist);
int empty_channels(ptr_channel_list mylist);
ptr_channel search_channel(ptr_channel_list mylist,unsigned long num);
int insert_channel(ptr_channel_list mylist,unsigned long num,char chname[100]);

#endif
