#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "hash.h"
#include "func.h"
#include "queue.h"

void *manage_commands(void *data);
void *manage_connections(void* data);
pthread_mutex_t mutexes[HASHSIZE];
pthread_mutex_t queue_mutex;
pthread_cond_t cond_nonempty;
pthread_cond_t cond_nonfull;

int main (int argc, char* argv[])
{
	if(argc!=7)
	{
		printf("Incorrect number of arguments\n");
		exit(EXIT_FAILURE);
	}
	int port_number;
	int pool_size;
	int queue_size;

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
	if(strcmp(argv[1],"-s")==0)
		pool_size=atoi(argv[2]);
	else if(strcmp(argv[3],"-s")==0)
		pool_size=atoi(argv[4]);
	else if(strcmp(argv[5],"-s")==0)
		pool_size=atoi(argv[6]);
	else
	{
		printf("Thread pool size wasn't given\n");
		exit(EXIT_FAILURE);
	}
	if(strcmp(argv[1],"-q")==0)
		queue_size=atoi(argv[2]);
	else if(strcmp(argv[3],"-q")==0)
		queue_size=atoi(argv[4]);
	else if(strcmp(argv[5],"-q")==0)
		queue_size=atoi(argv[6]);
	else
	{
		printf("Queue size wasn't given\n");
		exit(EXIT_FAILURE);
	}

	int sock,newsock,*clientsock;
	socklen_t serverlen,clientlen;
	struct sockaddr_in server,client;
	struct sockaddr *serverptr,*clientptr;
	//Create socket
	if((sock=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		perror("Socket creation");
		exit(EXIT_FAILURE);
	}
	printf("Socket created successfully\n");
	//Prepare the sockaddr in structure
	server.sin_family=AF_INET;
	server.sin_addr.s_addr=INADDR_ANY;
	server.sin_port=htons(port_number);
	serverptr=(struct sockaddr*) &server;
	clientptr=(struct sockaddr*) &client;
	serverlen=sizeof(server);
	clientlen=sizeof(client);
	//Bind the socket
	if(bind(sock,serverptr,serverlen)<0)
	{
		perror("Socket bind");
		exit(EXIT_FAILURE);
	}
	printf("Binded successfully\n");
	//Listen
	if(listen(sock,5)<0)
	{
		perror("Listening");
		exit(EXIT_FAILURE);
	}
	printf("Listening...\n");
	hash_table* hashtable;										//Create the hash table
	hashtable=create_hash();
	Qlist* qu;											//Create the queue
	qu=create_queue(queue_size);
	int k;
	pthread_t master_thread;
	pthread_t worker_thread[pool_size];

	//Create the structs that will be passed to the threads
	TCdata connection_data;
	connection_data.socket=sock;
	connection_data.cl_ptr=clientptr;
	connection_data.cl_len=clientlen;
	connection_data.queue=qu;
	THdata commands_data;
	commands_data.queue=qu;
	commands_data.htable=hashtable;
	
	for(k=0;k<HASHSIZE;k++)
		pthread_mutex_init(&mutexes[k],0);							//Initialize the array of mutexes and cond variables
	pthread_mutex_init(&queue_mutex,0);
	pthread_cond_init(&cond_nonempty,0);
	pthread_cond_init(&cond_nonfull,0);
	
	//Create the threads
	if((pthread_create(&master_thread,NULL,manage_connections,(void*)&connection_data))<0)
	{
		perror("Master thread create");
		exit(EXIT_FAILURE);
	}
	for(k=0;k<pool_size;k++)
	{
		if((pthread_create(&worker_thread[k],NULL,manage_commands,(void*)&commands_data))<0)
		{
			perror("Worker thread create");
			exit(EXIT_FAILURE);
		}
	}
	//Wait until all threads finish and then destroy cond variables, mutexes,queue and hashtable
	pthread_join(master_thread,0);
	for(k=0;k<pool_size;k++)
		pthread_join(worker_thread[k],0);
	pthread_cond_destroy(&cond_nonempty);
	pthread_cond_destroy(&cond_nonfull);
	for(k=0;k<pool_size;k++)
		pthread_mutex_destroy(&mutexes[k]);
	pthread_mutex_destroy(&queue_mutex);
	delete_queue(qu);
	delete_table(hashtable);
}


void *manage_commands(void *data)
{
	char *token;
	const char s[3]=" \n";										//To use in strtok
	int word=0;											//To count number of words in a line
	THdata *hash_data;
	hash_data=(THdata*) data;
	Qlist* conn_queue;
	conn_queue=hash_data->queue;
	hash_table* hashtable;
	hashtable=hash_data->htable;
	unsigned int hash1,hash2;
	int socket;
	char buff[1024],fullbuff[1024];
	char message[300];
	char *saveptr;
	while(1)
	{
		pthread_mutex_lock(&queue_mutex);							//Lock the mutex
		while(conn_queue->curr_size<=0)								//Until a fds enters the queue
			pthread_cond_wait(&cond_nonempty,&queue_mutex);					//Suspend thread
		socket=conn_queue->head->fds;								//Get a fds from the queue
		remove_from_queue(conn_queue);								//And remove it from there
		pthread_cond_signal(&cond_nonfull);							//Signal that there is room in the fds queue
		pthread_mutex_unlock(&queue_mutex);							//Unlock the mutex
		write_data(socket,"You can now send your commands\n");					//Inform fds to start sending commands
		while(1)
		{
			read_data(socket,buff);								//Read every commands that was sent here
			strcpy(fullbuff,buff);
			token=strtok_r(buff,s,&saveptr);						//Compare the first word
			if(strcmp(token,"add_account")==0)						//In the case of add account
			{
				int balance=0,success=1;
				int delay=0;
				char name[100];
				while(token!=NULL)							//Get necessary arguments
				{
					word++;
					if(word==2)
						sscanf(token,"%d",&balance);
					else if(word==3)
						strcpy(name,token);
					else if(word==4)
						sscanf(token,"%d",&delay);
					token=strtok_r(NULL,s,&saveptr);
				}
				if(word!=3 && word!=4)							//Incorrect number of arguments
					success=0;
				if(success==1)								//Correct number of arguments and the name doesn't exist in the hashtable
				{
					hash1=hash_func(hashtable,name);
					pthread_mutex_lock(&mutexes[hash1]);				//Lock mutex of correct bucket in hashtable
					if(insert_to_table(hashtable,name,balance)==1)
					{
						usleep(delay*1000);
						sprintf(message,"Success. Account creation (%s:%d)\n",name,balance);
						write_data(socket,message);
					}
					else
						success=0;
					pthread_mutex_unlock(&mutexes[hash1]);
				}
				if(success==0)								//Failed to add account
				{
					usleep(delay*1000);
					sprintf(message,"Error. Account creation failed (%s:%d)\n",name,balance);
					write_data(socket,message);
				}
			}
			else if(strcmp(token,"add_transfer")==0)					//In the case of add transfer
			{
				int transfer_amm=0,success=1;
				int delay=0;
				char src_name[100],dst_name[100];
				while(token!=NULL)							//Get arguments
				{
					word++;
					if(word==2)
						sscanf(token,"%d",&transfer_amm);
					else if(word==3)
						strcpy(src_name,token);
					else if(word==4)
						strcpy(dst_name,token);
					else if(word==5)
						sscanf(token,"%d",&delay);
					token=strtok_r(NULL,s,&saveptr);
				}
				if(word!=4 && word!=5)							//Incorrect number of arguments
					success=0;
				if(success==1)
				{
					hash1=hash_func(hashtable,src_name);
					hash2=hash_func(hashtable,dst_name);				//Lock necessary mutex or mutexes
					if(hash1<hash2)
					{
						pthread_mutex_lock(&mutexes[hash1]);
						pthread_mutex_lock(&mutexes[hash2]);
					}
					else if(hash1==hash2)
						pthread_mutex_lock(&mutexes[hash1]);
					else
					{
						pthread_mutex_lock(&mutexes[hash2]);
						pthread_mutex_lock(&mutexes[hash1]);
					}
					if(add_transfer_to_list(hashtable,src_name,dst_name,transfer_amm)==1)			//Successful transaction
					{
						usleep(delay*1000);
						sprintf(message,"Transfer addition (%s:%s:%d)[:%d]\n",src_name,dst_name,transfer_amm,delay);
						write_data(socket,message);
					}
					else
						success=0;
					if(hash1<hash2)
					{
						pthread_mutex_unlock(&mutexes[hash1]);
						pthread_mutex_unlock(&mutexes[hash2]);
					}
					else if(hash1==hash2)
						pthread_mutex_unlock(&mutexes[hash1]);
					else
					{
						pthread_mutex_unlock(&mutexes[hash2]);
						pthread_mutex_unlock(&mutexes[hash1]);
					}
				}
				if(success==0)
				{
					usleep(delay*1000);
					sprintf(message,"Transfer addition failed (%s:%s:%d)[:%d]\n",src_name,dst_name,transfer_amm,delay);
					write_data(socket,message);
				}
			}
			else if(strcmp(token,"add_multi_transfer")==0)					//In case of multi transfer
			{
				hash_entry* temp_entry;
				MQnode* temp_node;
				MQlist* temp_queue;							//Create the queue
				temp_queue=create_multiqueue();
				int transfer_amm=0,success=1;
				int delay=0;
				char src_name[100],dst_name[100];
				while(token!=NULL)							//Get arguments
				{
					word++;
					if(word==2)
						sscanf(token,"%d",&transfer_amm);
					else if(word==3)
					{
						strcpy(src_name,token);
						temp_entry=search_table(hashtable,src_name);
						if(temp_entry==NULL)
							success=0;
					}
					else if(word>3)
						strcpy(dst_name,token);
					token=strtok_r(NULL,s,&saveptr);
					if(word>3 && token!=NULL)
					{
						if(search_table(hashtable,dst_name)==NULL)		//No such entry in hashtable
							success=0;
						if(add_to_multiqueue(temp_queue,dst_name)==0)		//Name already exists in multiqueue
							success=0;
					}
					if(word>3 && token==NULL)						//is is the last word in the string
					{
						if(strtod(dst_name,NULL)>0)					//If it is a number
							delay=strtod(dst_name,NULL);				//Use it as delay
						else
						{
							if(search_table(hashtable,dst_name)==NULL)		//No such entry in hashtable
								success=0;
							if(add_to_multiqueue(temp_queue,dst_name)==0)		//Name already exists in multiqueue
								success=0;
						}
					}
				}
				if(word<4)
					success=0;
				if(success==1)									//Source name has enough money to send to all
					if((temp_queue->curr_size*transfer_amm)>temp_entry->node_pointer->current_balance)
						success==0;
				if(success==1)
				{
					int lock_mtxs[HASHSIZE];
					int i;
					for(i=0;i<HASHSIZE;i++)
						lock_mtxs[i]=0;
					hash1=hash_func(hashtable,src_name);
					lock_mtxs[hash1]=1;
					temp_node=temp_queue->head;
					while(temp_node!=NULL)							//Run the whole multiqueue and check which mutexes need to be locked
					{
						hash2=hash_func(hashtable,temp_node->name);
						lock_mtxs[hash2]=1;
						temp_node=temp_node->next;
					}
					for(i=0;i<HASHSIZE;i++)
					{
						if(lock_mtxs[i]==1)
							pthread_mutex_lock(&mutexes[i]);			//Lock all necessary mutexes
					}
					usleep(delay*1000);
					temp_node=temp_queue->head;
					while(temp_node!=NULL)							//Run the whole multiqueue and transfer the money to each account
					{
						add_transfer_to_list(hashtable,src_name,temp_node->name,transfer_amm);
						temp_node=temp_node->next;
					}
					for(i=0;i<HASHSIZE;i++)
					{
						if(lock_mtxs[i]==1)
							pthread_mutex_unlock(&mutexes[i]);			//Lock all necessary mutexes
					}
				}
				delete_multiqueue(temp_queue);
				if(success==1)
				{
					sprintf(message,"Multi-Transfer addition (%s:%d)[:%d]\n",src_name,transfer_amm,delay);
					write_data(socket,message);
				}
				else
				{
					usleep(delay*1000);
					sprintf(message,"Multi-Transfer addition failed (%s:%d)[:%d]\n",src_name,transfer_amm,delay);
					write_data(socket,message);
				}
			}
			else if(strcmp(token,"print_balance")==0)						//In case of print balance
			{
				int success=1;
				char name[100];
				int delay=0;
				hash_entry* temp_entry;
				while(token!=NULL)								//Get arguments
				{
					word++;
					if(word==2)
						strcpy(name,token);
					else if(word==3)
						sscanf(token,"%d",&delay);
					token=strtok_r(NULL,s,&saveptr);
				}
				if(word!=2 && word!=3)
					success=0;
				else
					temp_entry=search_table(hashtable,name);				//Search name in hashtable
				if(success==1 && temp_entry!=NULL)						//It is in the hashtable
				{
					hash1=hash_func(hashtable,name);
					pthread_mutex_lock(&mutexes[hash1]);
					usleep(delay*1000);							//Lock the mutex and send the current balance
					sprintf(message,"Success. Balance (%s:%d)\n",name,temp_entry->node_pointer->current_balance);
					pthread_mutex_unlock(&mutexes[hash1]);
					write_data(socket,message);
				}
				else
				{
					usleep(delay*1000);
					sprintf(message,"Error. Balance (%s)\n",name);
					write_data(socket,message);
				}
			}
			else if(strcmp(token,"print_multi_balance")==0)						//In case of print multi balance
			{
				int success=1;
				char name[100];
				int delay=0;
				MQnode* temp_node;
				MQlist* temp_queue;								//Create the queue
				temp_queue=create_multiqueue();
				hash_entry* temp_entry;
				while(token!=NULL)
				{
					word++;
					if(word>1)
						strcpy(name,token);
					token=strtok_r(NULL,s,&saveptr);
					if(word>1 && token!=NULL)						//It isn't the last word in the string
					{
						if(search_table(hashtable,name)==NULL)				//No such entry in hashtable
							success=0;
						if(add_to_multiqueue(temp_queue,name)==0)			//Name already exists in multiqueue
							success=0;
					}
					if(word>1 && token==NULL)						//It is the last word
					{
						if(strtod(name,NULL)>0)
							delay=strtod(name,NULL);				//Check if it is number so it's a delay
						else
						{
							if(search_table(hashtable,name)==NULL)			//No such entry in hashtable
								success=0;
							if(add_to_multiqueue(temp_queue,name)==0)		//Name already exists in multiqueue
								success=0;
						}
					}
				}
				char temp_message[100];
				if(success==1)
				{
					int lock_mtxs[HASHSIZE];
					int i;
					for(i=0;i<HASHSIZE;i++)
						lock_mtxs[i]=0;
					temp_node=temp_queue->head;
					while(temp_node!=NULL)							//Run the whole multiqueue and check which mutexes need to be locked
					{
						hash2=hash_func(hashtable,temp_node->name);
						lock_mtxs[hash2]=1;
						temp_node=temp_node->next;
					}
					for(i=0;i<HASHSIZE;i++)
					{
						if(lock_mtxs[i]==1)
							pthread_mutex_lock(&mutexes[i]);			//Lock all necessary mutexes
					}
					sprintf(message,"Success. Multi-Balance (");
					usleep(delay*1000);
					temp_node=temp_queue->head;
					while(temp_node!=NULL)							//Run the whole multiqueue
					{
						temp_entry=search_table(hashtable,temp_node->name);		//Find name and print every balance in a string to send to the client
						sprintf(temp_message,"%s/%d",temp_entry->node_pointer->account_name,temp_entry->node_pointer->current_balance);
						strcat(message,temp_message);
						if(temp_node->next!=NULL)
							strcat(message,":");
						temp_node=temp_node->next;
					}
					for(i=0;i<HASHSIZE;i++)
					{
						if(lock_mtxs[i]==1)
							pthread_mutex_unlock(&mutexes[i]);			//Lock all necessary mutexes
					}
					strcat(message,")\n");
					write_data(socket,message);
				}
				else
				{
					word=0;
					usleep(delay*1000);
					sprintf(message,"Error. Multi-Balance (");
					token=strtok_r(fullbuff,s,&saveptr);
					while(token!=NULL)
					{
						word++;
						if(word==2)
						{
							sprintf(temp_message,"%s",token);
							strcat(message,temp_message);
						}
						else if(word>2)
						{
							sprintf(temp_message,":%s",token);
							strcat(message,temp_message);
						}
						token=strtok_r(NULL,s,&saveptr);
					}
					strcat(message,")\n");
					write_data(socket,message);
				}
				delete_multiqueue(temp_queue);
			}
			else if(strcmp(buff,"FINISHED")==0)
				break;
			else
			{
				sprintf(message,"Error. Unknown command\n");
				write_data(socket,message);
			}
			word=0;
		}
		close(socket);											//Got finished so we exit the while and close the socket
	}
}


void *manage_connections(void* data)
{
	TCdata *conn_data;
	conn_data=(TCdata*) data;
	int sock,newsock;
	socklen_t clientlen;
	struct sockaddr *clientptr;
	Qlist *conn_queue;
	sock=conn_data->socket;
	clientlen=conn_data->cl_len;
	clientptr=conn_data->cl_ptr;
	conn_queue=conn_data->queue;
	while(1)
	{
		newsock=accept(sock,clientptr,&clientlen);							//Continuously accept requests
		if(newsock<0)
			continue;
		pthread_mutex_lock(&queue_mutex);
		while(conn_queue->curr_size==conn_queue->maxsize)						//suspend thread until there is some space
			pthread_cond_wait(&cond_nonfull,&queue_mutex);
		add_to_queue(conn_queue,newsock);								//Add fds to queue
		write_data(newsock,"Hello new client.\nYou will be assigned a worker thread when one is available\n");			//Inform fds it was accepted
		pthread_cond_signal(&cond_nonempty);								//Signal that there at least one fds in queue
		pthread_mutex_unlock(&queue_mutex);
	}
}
