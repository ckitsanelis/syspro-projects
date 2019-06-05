#ifndef FUNC_H_
#define FUNC_H_

#include "hash.h"
#include "queue.h"

typedef struct thread_hash_data THdata;
typedef struct thread_conn_data TCdata;

struct thread_hash_data
{
	Qlist *queue;
	hash_table *htable;
};

struct thread_conn_data
{
	int socket;
	struct sockaddr *cl_ptr;
	socklen_t cl_len;
	Qlist *queue;
};

int read_data(int fd,char* buff);
int write_data(int fd,char* buff);

#endif
