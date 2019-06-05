#ifndef HASH_H_
#define HASH_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct Node node;
typedef struct Edge edge;
typedef struct Hash_entry hash_entry;
typedef struct Hash_table hash_table;
typedef struct Graph graph;

struct Node
{
	unsigned long key;
	int visited;
	struct Edge *edges_leaving;		//Pointer to the first edge leaving
	struct Edge *edges_coming;		//Pointer to the first edge coming in
};

struct Edge
{
	double weight;
	unsigned long node_head;		//Key of node that the edge is leaving from
	unsigned long node_tail;		//Key of node that the edge is going to
	struct Edge *next_edge;			//Pointer to the next edge in the list
};

struct Graph
{
	struct Hash_table *hashtable;
};

struct Hash_entry
{
	struct Node *node_pointer;		//Pointer to the node of the hash entry
	struct Hash_entry *next_entry;		//Pointer to next hash entry in a bucket
};

struct Hash_table
{
	int size;				//Size of array
	struct Hash_entry **table;		//Array of pointers that point to hash entries
};

hash_table *create_hash(int size);
int hash_func(hash_table *hashtable,unsigned long num);
hash_entry *search_table(hash_table *hashtable,unsigned long num);
int insert_to_table(hash_table *hashtable,unsigned long num);
void delete_table(hash_table *hashtable);

#endif
