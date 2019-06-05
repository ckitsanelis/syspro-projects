#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hash.h"

//Creates a hash table
hash_table *create_hash(int size)
{
	int i;
	hash_table *new_hashtable;
	if(size<1)
	{
		printf("failure: Can't create graph with negative or zero size\n");
		exit(EXIT_FAILURE);
	}
	new_hashtable=malloc(sizeof(hash_table));		//Allocate the necessary memory
	if(new_hashtable==NULL)
		return NULL;
	new_hashtable->table=malloc(size*sizeof(hash_entry));
	if(new_hashtable->table==NULL)
		return NULL;
	for(i=0;i<size;i++)
		new_hashtable->table[i]=NULL;
	new_hashtable->size=size;
	return new_hashtable;
}

//Configures the hash value of a key
int hash_func(hash_table *hashtable,unsigned long num)
{
	int hash_value;
	hash_value=num%hashtable->size;
	return hash_value;
}

//Returns the entry in the hash table with the specific key
hash_entry *search_table(hash_table *hashtable,unsigned long num)
{
	hash_entry *entry;
	int hash_value;
	hash_value=hash_func(hashtable,num);		//Search only the bucket with this key's hash value
	for(entry=hashtable->table[hash_value];entry!=NULL;entry=entry->next_entry)
	{
		if(num==entry->node_pointer->key)		//Returns pointer to the entry in the table
			return entry;
	}
	return NULL;		//If there is no such node in the table
}

//Inserts a new entry to the hash table
int insert_to_table(hash_table *hashtable,unsigned long num)
{
	hash_entry *entry,*new_entry;
	int hash_value;
	entry=search_table(hashtable,num);	//Checking if there is already a node with the key
	if(entry!=NULL)
	{
		printf("There is already an entry in the hash table with the same key\n");
		return 0;
	}
	else			//There isn't already a node, so we create a new one
	{
		hash_value=hash_func(hashtable,num);	//Which bucket we will insert the node to
		new_entry=malloc(sizeof(hash_entry));
		new_entry->node_pointer=malloc(sizeof(node));
		new_entry->node_pointer->key=num;
		new_entry->node_pointer->visited=0;
		new_entry->node_pointer->edges_leaving=NULL;
		new_entry->node_pointer->edges_coming=NULL;
		new_entry->next_entry=hashtable->table[hash_value];	//Place it at the beginning of the bucket list
		hashtable->table[hash_value]=new_entry;
		return 1;
	}
}

void delete_table(hash_table *hashtable)
{
	int buck;
	hash_entry *entry,*temp_entry;
	edge *edge,*temp_edge;
	if(hashtable==NULL)
		return;
	for(buck=0;buck<hashtable->size;buck++)		//For every bucket
	{
		entry=hashtable->table[buck];
		while(entry!=NULL)			//For every entry in a bucket
		{
			edge=entry->node_pointer->edges_leaving;
			while(edge!=NULL)		//Free the list of edges leaving
			{
				temp_edge=edge;
				edge=edge->next_edge;
				free(temp_edge);
			}
			edge=entry->node_pointer->edges_coming;
			while(edge!=NULL)		//Free the list of edges coming
			{
				temp_edge=edge;
				edge=edge->next_edge;
				free(temp_edge);
			}
			temp_entry=entry;
			entry=entry->next_entry;
			free(temp_entry->node_pointer);	//Free the pointer to the node
			free(temp_entry);		//Free the entry
		}
	}
	free(hashtable->table);
	free(hashtable);				//Free the hashtable
	return;
}
